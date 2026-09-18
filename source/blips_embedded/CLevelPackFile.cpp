#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "Common.h"
#include "GameFuncs.h"
#include "CLevelPackFile.h"
#include "Levelpacks.h"
#include "Defines.h"

//every line of a level is at least two bytes (a character and the newline), so the
//line counter y (uint16_t) can not wrap while a pack stays below 131072 bytes
static_assert((sizeof(levelpack_bips) < 131072) && (sizeof(levelpack_bips_gold) < 131072) &&
	(sizeof(levelpack_bips_platinum) < 131072) && (sizeof(levelpack_bips_gold_2_players) < 131072), "a level pack is too big for the uint16_t line counter");


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

bool CLevelPackFile_loadFile(CLevelPackFile* LPackFile, const char* filename, uint8_t maxWidth, uint8_t maxHeight, int8_t level)
{
	bool Result = false;

	//remember the pack so a single level can be reloaded from it later on. The names
	//live in flash for the whole run, keeping the pointer is enough
	LPackFile->filename = filename;

	//the packs carry no terminator and the linker puts them back to back in flash,
	//so their length is the only thing that stops one pack running into the next
   	if(strcmp(filename, "bips_gold.bip") == 0)
		Result = CLevelPackFile_parseText(LPackFile, levelpack_bips_gold, sizeof(levelpack_bips_gold), maxWidth, maxHeight, level);
	else if(strcmp(filename, "bips_platinum.bip") == 0)
		Result = CLevelPackFile_parseText(LPackFile, levelpack_bips_platinum, sizeof(levelpack_bips_platinum), maxWidth, maxHeight, level);
	else if(strcmp(filename, "bips_gold_2_players.bip") == 0)
		Result = CLevelPackFile_parseText(LPackFile, levelpack_bips_gold_2_players, sizeof(levelpack_bips_gold_2_players), maxWidth, maxHeight, level);
	else
		Result = CLevelPackFile_parseText(LPackFile, levelpack_bips, sizeof(levelpack_bips), maxWidth, maxHeight, level);

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
bool CLevelPackFile_parseText(CLevelPackFile *LPackFile, const unsigned char* text, size_t textLen, uint8_t maxWidth, uint8_t maxHeight, int8_t level)
{
	char line[MAXLINELEN] = "";
	char levelField[MAXLEVELFIELDLEN] = "";
	char levelFieldValue[MAXLEVELFIELDDATALEN] = "";
	uint8_t linepos;
	const unsigned char* pchar = text;
	const unsigned char* pend = text + textLen;
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
	while(pchar < pend)
	{
		linepos = 0;
		while(pchar < pend)
		{
			c = (char)PLATFORM_READ_BYTE(pchar);
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
            pchar++;
		}

		//step over the newline, a '\0' in the middle of the data still ends the pack
		if(pchar < pend)
		{
			if(c == '\0')
				pchar = pend;
			else
				pchar++;
		}

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
