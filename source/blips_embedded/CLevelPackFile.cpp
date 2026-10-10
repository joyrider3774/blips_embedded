#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "Common.h"
#include "GameFuncs.h"
#include "CLevelPackFile.h"
//the levels read off the card, for a build with CARDLEVELS on
#include "cardimages.h"
#if !CARDLEVELS
#include "Levelpacks.h"
#endif
#include "Defines.h"

CLevelPackFile* CLevelPackFile_Create()
{
	CLevelPackFile* Result = (CLevelPackFile*)malloc(sizeof(CLevelPackFile));
	//nothing in the game works without a pack file, the caller has to report it.
	//Writing to Result here would fault on address 0 instead
	if (!Result)
	{
		Platform_Log("CLevelPackFile_Create: out of heap, %" PRIu32 " free\n", Platform_FreeHeap());
		return NULL;
	}
	Result->Loaded = false;
	Result->LevelCount = 0;
	Result->LoadedLevel = 0;
	memset(Result->author, 0, MAXAUTHORLEN);
	memset(Result->set, 0, MAXSETLEN);
	Result->filename = NULL;
	return Result;
}

void CLevelPackFile_Destroy(CLevelPackFile* LPackFile)
{
	if(LPackFile)
	{
		free(LPackFile);
		LPackFile = NULL;
	}
}

#if CARDLEVELS
//The packs are on the card and not in flash, so this is the card's own list of them: the names
//are the ones the game already knows a pack by, in the order the card's index holds them, see
//CARD_LEVEL_NAMES in cardindex.h. Everything above this reads exactly as it did
static const char* const cardPackNames[CARD_LEVEL_COUNT] = CARD_LEVEL_NAMES;

uint8_t CLevelPackFile_BuiltInCount(void)
{
	return (uint8_t)CARD_LEVEL_COUNT;
}

const char* CLevelPackFile_BuiltInName(uint8_t index)
{
	return (index < CARD_LEVEL_COUNT) ? cardPackNames[index] : NULL;
}
#else
uint8_t CLevelPackFile_BuiltInCount(void)
{
	return (uint8_t)(sizeof(builtInPacks) / sizeof(builtInPacks[0]));
}

const char* CLevelPackFile_BuiltInName(uint8_t index)
{
	return (index < CLevelPackFile_BuiltInCount()) ? builtInPacks[index].filename : NULL;
}
#endif

bool CLevelPackFile_loadFile(CLevelPackFile* LPackFile, const char* filename, uint8_t maxWidth, uint8_t maxHeight, int8_t level)
{
	bool Result = false;

	//remember the pack so a single level can be reloaded from it later on. The names
	//live in flash for the whole run, keeping the pointer is enough
	LPackFile->filename = filename;

#if CARDLEVELS
	//the pack lies on the card; a name none of them answered to falls back on the first, as below
	uint8_t which = 0;
	for (uint8_t i = 0; i < CARD_LEVEL_COUNT; i++)
	{
		if (strcmp(filename, cardPackNames[i]) == 0)
		{
			which = i;
			break;
		}
	}
	uint32_t at = 0, size = 0;
	if (!CardLevels_Pack(which, &at, &size))
		return false;
	Result = CLevelPackFile_parseCard(LPackFile, at, size, maxWidth, maxHeight, level);
#else
	//The packs carry no terminator and the linker puts them back to back in flash, so their
	//length is the only thing that stops one pack running into the next. The table comes from
	//tools/convert_levelpacks.py, so it holds exactly the packs that are in the build
	const LevelPackEntry* pack = &builtInPacks[0];
	for (uint8_t i = 0; i < CLevelPackFile_BuiltInCount(); i++)
	{
		if (strcmp(filename, builtInPacks[i].filename) == 0)
		{
			pack = &builtInPacks[i];
			break;
		}
	}
	//a name none of them answered to falls back on the first pack there is
	Result = CLevelPackFile_parseText(LPackFile, pack->text, pack->size, maxWidth, maxHeight, level);
#endif
	LPackFile->Loaded = true;
	return Result;
}

//reparses the pack that was loaded last so the requested level ends up in memory
bool CLevelPackFile_loadLevel(CLevelPackFile* LPackFile, int8_t level)
{
	if(!LPackFile->filename)
		return false;
	return CLevelPackFile_loadFile(LPackFile, LPackFile->filename, NrOfCols, NrOfRows, level);
}

//stores a metadata field that was read into the metadata of the level being parsed
static void CLevelPackFile_storeLevelField(LevelMeta* levelMeta, char* levelField, char* levelFieldValue)
{
	char* ptmp = levelFieldValue;
	while(*ptmp == ' ')
		ptmp++;
	if (strcmp(levelField, "title") == 0)
	{
		snprintf(levelMeta->title, sizeof(levelMeta->title), "%s", ptmp);
	}
	else
	{
		if (strcmp(levelField, "author") == 0)
		{
			snprintf(levelMeta->author, sizeof(levelMeta->author), "%s", ptmp);
		}
		else
		{
			if (strcmp(levelField, "comment") == 0)
			{
				snprintf(levelMeta->comments, sizeof(levelMeta->comments), "%s", ptmp);
			}
		}
	}
}

//counts the level that was just parsed and keeps it when it is the one asked for,
//returns false when parsing should stop because the level limit has been reached
static bool CLevelPackFile_endLevel(CLevelPackFile* LPackFile, LevelMeta* levelMeta, int8_t level, uint8_t maxWidth, uint8_t maxHeight)
{
	//a level that does not fit the playfield is skipped, the next one reuses its slot
	if((levelMeta->maxx+1 > maxWidth) || (levelMeta->maxy+1 > maxHeight))
		return true;

	LPackFile->LevelCount++;
	//this is the level we want to keep, its parts were already written into the
	//pack while parsing so only the metadata is left to copy
	if(LPackFile->LevelCount == level)
	{
		memcpy(&LPackFile->Meta, levelMeta, sizeof(LevelMeta));
		LPackFile->LoadedLevel = level;
	}
	return LPackFile->LevelCount < MAXLEVELS;
}

//level == LPLevelHeaderOnly : only read the set / author of the pack itself
//level == LPLevelCountOnly  : read the whole pack to count the levels in it
//level >= 1                 : read the whole pack but only keep that one level in memory
//A pack is stored run length encoded, see tools/convert_levelpacks.py: a control byte with its
//top bit set stands for (c & 0x7F) + 1 copies of the byte that follows it, and one without for
//the (c + 1) bytes that follow. Level text is mostly runs of wall and floor, which takes about
//45% off a pack.
//The parser reads a pack from start to end, a byte at a time and never twice, so it is decoded
//as it is read: nothing of it is held in ram, which is what a device with 20k of it needs
typedef struct PackReader PackReader;
struct PackReader
{
	const unsigned char* pos;   //the next control byte, or the next byte of a literal
	const unsigned char* end;
#if CARDLEVELS
	//The same walk over a pack that lies on the card instead of in flash. A card is read by
	//offset and not through a pointer, so the two ends of the pack are offsets and a chunk of
	//it at a time is held here: the parser asks for one byte at a time and a read a byte would
	//be a card command a byte
	uint32_t at, cardEnd;
	uint16_t have, used;
	uint8_t buf[CARD_PACK_CHUNK];
#endif
	uint8_t left;               //how many bytes the run or the literal still owes
	unsigned char repeated;     //the byte a run repeats
	bool inRun;
};

static void PackReaderInit(PackReader* reader, const unsigned char* text, size_t textLen)
{
	reader->pos = text;
	reader->end = text + textLen;
#if CARDLEVELS
	reader->at = 0;
	reader->cardEnd = 0;
	reader->have = 0;
	reader->used = 0;
#endif
	reader->left = 0;
	reader->repeated = 0;
	reader->inRun = false;
}

#if CARDLEVELS
//the same, for a pack that lies on the card: where it starts and how long it is
static void PackReaderInitCard(PackReader* reader, uint32_t at, uint32_t length)
{
	PackReaderInit(reader, NULL, 0);
	reader->at = at;
	reader->cardEnd = at + length;
}
#endif

//The next raw byte of the pack, before the run length encoding is undone, and false once the
//pack has none left. This is the only place that knows where a pack is kept
static bool PackRaw(PackReader* reader, unsigned char* out)
{
#if CARDLEVELS
	if (reader->used >= reader->have)
	{
		if (reader->at >= reader->cardEnd)
			return false;
		uint32_t want = reader->cardEnd - reader->at;
		if (want > CARD_PACK_CHUNK)
			want = CARD_PACK_CHUNK;
		if (!Platform_CardRead(reader->at, reader->buf, want))
			return false;
		reader->at += want;
		reader->have = (uint16_t)want;
		reader->used = 0;
	}
	*out = reader->buf[reader->used++];
	return true;
#else
	if (reader->pos >= reader->end)
		return false;
	//PLATFORM_READ_BYTE is pgm_read_byte on some of the devices, which may look at what it
	//is handed more than once, so the pointer is never stepped on inside it
	*out = (unsigned char)PLATFORM_READ_BYTE(reader->pos);
	reader->pos++;
	return true;
#endif
}

//gives the next byte of the pack, false once there are none left
static bool PackReaderNext(PackReader* reader, char* out)
{
	if(reader->left == 0)
	{
		unsigned char control;
		if(!PackRaw(reader, &control))
			return false;
		if(control & 0x80)
		{
			reader->inRun = true;
			reader->left = (uint8_t)((control & 0x7F) + 1);
			//a run that says what it repeats but does not carry it is a truncated pack
			if(!PackRaw(reader, &reader->repeated))
				return false;
		}
		else
		{
			reader->inRun = false;
			reader->left = (uint8_t)(control + 1);
		}
	}

	reader->left--;
	if(reader->inRun)
	{
		*out = (char)reader->repeated;
		return true;
	}
	unsigned char literal;
	if(!PackRaw(reader, &literal))
		return false;
	*out = (char)literal;
	return true;
}

//Everything below reads the pack through the reader above and nothing else, so a pack on the
//card and a pack in flash are parsed by the same code: only where the reader was opened differs
static bool ParseWith(CLevelPackFile *LPackFile, PackReader* readerIn, uint8_t maxWidth, uint8_t maxHeight, int8_t level);

#if CARDLEVELS
bool CLevelPackFile_parseCard(CLevelPackFile *LPackFile, uint32_t at, uint32_t length, uint8_t maxWidth, uint8_t maxHeight, int8_t level)
{
	PackReader reader;
	PackReaderInitCard(&reader, at, length);
	return ParseWith(LPackFile, &reader, maxWidth, maxHeight, level);
}
#endif

bool CLevelPackFile_parseText(CLevelPackFile *LPackFile, const unsigned char* text, size_t textLen, uint8_t maxWidth, uint8_t maxHeight, int8_t level)
{
	PackReader reader;
	PackReaderInit(&reader, text, textLen);
	return ParseWith(LPackFile, &reader, maxWidth, maxHeight, level);
}

static bool ParseWith(CLevelPackFile *LPackFile, PackReader* readerIn, uint8_t maxWidth, uint8_t maxHeight, int8_t level)
{
	PackReader reader = *readerIn;
	char line[MAXLINELEN] = "";
	char levelField[MAXLEVELFIELDLEN] = "";
	char levelFieldValue[MAXLEVELFIELDDATALEN] = "";
	uint8_t linepos;
	//1 while the pack still has a byte to give
	bool more = true;
	char* pdoublepoint, *pset, *pauthor;
	uint16_t y = 0;
	bool inlevel = false;
	LPackFile->LevelCount = 0;
	LPackFile->LoadedLevel = 0;
	memset(LPackFile->author, 0, MAXAUTHORLEN);
	memset(LPackFile->set, 0, MAXSETLEN);
	//metadata of the level currently being parsed, it is only copied into the pack
	//once it turns out to be the level we want to keep in memory
	LevelMeta levelMetaData;
	LevelMeta* levelMeta = &levelMetaData;
	memset(levelMeta, 0, sizeof(LevelMeta));
	char c = '\0';
	while(more)
	{
		linepos = 0;
		//1 once this line has a byte in it, so that the end of the pack does not look like
		//one last empty line and end a level that already ended
		bool started = false;
		while((more = PackReaderNext(&reader, &c)))
		{
			started = true;
			if((c == '\n') || (c == '\0'))
				break;
			if((c != '\r') && (linepos < MAXLINELEN-1))
			{
				if((c >= 'A') && (c <= 'Z'))
				{
					line[linepos++] = c + 32;
				}
				else
					line[linepos++] = c;
            }
		}

		if(!started)
			break;
		//a '\0' in the middle of the data still ends the pack
		if(c == '\0')
			more = false;

		line[linepos] = '\0';

		if(LPackFile->LevelCount == 0)
		{
			if(!LPackFile->set[0])
			{
				pset = strstr(line, "set:");
				if(pset)
				{
					pset+= 4;
					while(*pset == ' ')
						pset++;
					snprintf(LPackFile->set, sizeof(LPackFile->set), "%s", pset);
				}
			}

			if(!LPackFile->author[0])
			{
				pauthor = strstr(line, "author:");
				if(pauthor)
				{
					pauthor+= 7;
					while(*pauthor == ' ')
						pauthor++;
					snprintf(LPackFile->author, sizeof(LPackFile->author), "%s", pauthor);
				}
			}
		}

		//found double point while in a level start a metadata field
		pdoublepoint = strstr(line, ":");
		if(inlevel && pdoublepoint)
		{
			if(levelField[0])
				CLevelPackFile_storeLevelField(levelMeta, levelField, levelFieldValue);
			memset(levelFieldValue, 0, MAXLEVELFIELDDATALEN);
			memset(levelField, 0, MAXLEVELFIELDLEN);
			//The name can be up to a whole line long, a name that does not fit is cut short
			//(it can not be one of the known fields then) instead of overflowing. Copied
			//rather than formatted with a precision, which not every printf understands
			size_t nameLen = (size_t)(pdoublepoint - &line[0]);
			if (nameLen > sizeof(levelField) - 1)
				nameLen = sizeof(levelField) - 1;
			memcpy(levelField, line, nameLen);
			levelField[nameLen] = '\0';
			snprintf(levelFieldValue, sizeof(levelFieldValue), "%s", pdoublepoint + 1);
			continue;
		}

		//we are in a level but found no empty line and no doublepoint then we are then in a multiline metadata field just append its value
		if(inlevel && linepos && !pdoublepoint && (levelField[0]))
		{
			//whatever no longer fits in the value is dropped
			size_t used = strlen(levelFieldValue);
			//the precision is never less than what fits, so the result is the same as
			//with a plain %s, it only tells the compiler the cut is on purpose
			snprintf(levelFieldValue + used, sizeof(levelFieldValue) - used, "%s%.*s", used ? "\n" : "", (uint8_t)(sizeof(levelFieldValue) - used - 1), line);
			continue;
		}

		//we are in a level and found a empty line then assume level end
		if(inlevel && !linepos)
		{
			if(levelField[0])
				CLevelPackFile_storeLevelField(levelMeta, levelField, levelFieldValue);
			//clear them for if condition above conerning level start
			memset(levelFieldValue, 0, MAXLEVELFIELDDATALEN);
			memset(levelField, 0, MAXLEVELFIELDLEN);
			inlevel = false;
			//don't exceed limits
			if(!CLevelPackFile_endLevel(LPackFile, levelMeta, level, maxWidth, maxHeight))
				break;
			continue;
		}

		//we are not in a level and found a wall and no doublepoint and we are not in a levelfield then assume levelstart
		if (!inlevel && !pdoublepoint && (!levelField[0]))
		{
			if (strchr(line, LPWall))
			{
				if (level == LPLevelHeaderOnly)
					return true;
				inlevel=true;
				y = 0;
				levelMeta->minx = NrOfCols;
				levelMeta->miny = NrOfRows;
				levelMeta->maxx = 0;
				levelMeta->maxy = 0;
				memset(levelMeta->author, 0, MAXAUTHORLEN);
				memset(levelMeta->title, 0, MAXTITLELEN);
				memset(levelMeta->comments, 0, MAXCOMMENTLEN);
				levelMeta->parts = 0;
			}
		}

		//we are in level and not in a level meta field
		if(inlevel && (!levelField[0]))
		{
			//the level being parsed becomes this number if it turns out to be valid,
			//only its parts are stored, all other levels are just counted and validated
			bool storeParts = (LPackFile->LevelCount + 1 == level);
			for(uint8_t x = 0; x < linepos; x++)
			{
				if (line[x] == LPFloor)
					continue;
				//DON'T EXCEED MAX ITEMCOUNT!
				if(levelMeta->parts+2 >= MAXITEMCOUNT)
				{
					levelMeta->maxx = 1000;
					break;
				}

				uint8_t partId = 0;
				switch(line[x])
				{
					case LPWall:
						if(x < levelMeta->minx)
							levelMeta->minx = x;
						if(x > levelMeta->maxx)
							levelMeta->maxx = x;
						if(y < levelMeta->miny)
							levelMeta->miny = y;
						if(y > levelMeta->maxy)
							levelMeta->maxy = y;
						partId = IDWall;
						break;
					case LPBox:
						partId = IDBox;
						break;
					case LPBomb:
						partId = IDBomb;
						break;
					case LPDiamond:
						partId = IDDiamond;
						break;
					case LPBox1:
						partId = IDBox1;
						break;
					case LPBox2:
						partId = IDBox2;
						break;
					case LPBoxBomb:
						partId = IDBoxBomb;
						break;
					case LPBoxWall:
						partId = IDBoxWall;
						break;
					case LPBreakableWall:
						partId = IDWallBreakable;
						break;
					case LPPlayer:
						partId = IDPlayer;
						break;
					case LPPlayer2:
						partId = IDPlayer2;
						break;
				}

				if(partId)
				{
					if(storeParts)
					{
						LevelPart* levelPart = &(LPackFile->Level[levelMeta->parts]);
						levelPart->x = x;
						levelPart->y = y;
						levelPart->id = partId;
					}
					levelMeta->parts++;
				}
			}
			y++;
		}
	}

	//a pack whose text does not end in an empty line leaves its last level without
	//a terminator, finish it here or it would be dropped
	if(inlevel)
	{
		if(levelField[0])
			CLevelPackFile_storeLevelField(levelMeta, levelField, levelFieldValue);
		CLevelPackFile_endLevel(LPackFile, levelMeta, level, maxWidth, maxHeight);
	}

	if(level >= 1)
		return LPackFile->LoadedLevel == level;
	return LPackFile->LevelCount > 0;
}
