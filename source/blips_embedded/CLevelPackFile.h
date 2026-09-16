#ifndef CLEVELPACKFILE_H
#define CLEVELPACKFILE_H

#include <stdint.h>
#include <stddef.h>

#include "Defines.h"

#define MAXLEVELS 27
#define MAXITEMCOUNT (NrOfCols*NrOfRows) + 2
#define MAXCOMMENTLEN 10
#define MAXSETLEN 50
#define MAXAUTHORLEN 25
#define MAXTITLELEN 35
#define MAXLEVELFIELDDATALEN 35
#define MAXLEVELFIELDLEN 10
#define MAXLINELEN 90

//values for the level parameter of the parse / load functions, any value >= 1
//means parse the whole pack but only keep that single level in memory
#define LPLevelHeaderOnly -1 //only read the pack's set / author, stop at the first level
#define LPLevelCountOnly 0   //read the whole pack to count its levels, keep no level

//level numbers are int8_t (-1 = LPLevelHeaderOnly), part counts uint16_t, a column
//is a position in a line so it fits a uint8_t, and so does the playfield size
static_assert(MAXLEVELS <= 127, "level numbers do not fit in int8_t");
static_assert(MAXITEMCOUNT <= 65535, "part counts do not fit in uint16_t");
static_assert((MAXLINELEN <= 256) && (NrOfCols <= 255) && (NrOfRows <= 255), "line positions do not fit in uint8_t");

#define LPFloor ' '
#define LPPlayer '@'
#define LPBox '$'
#define LPBomb '*'
#define LPWall '#'
#define LPDiamond 'd'
#define LPPlayer2 '&'
#define LPBox1 '1'
#define LPBox2 '2'
#define LPBoxBomb 'b'
#define LPBoxWall 'w'
#define LPBreakableWall 'x'



typedef struct LevelPart LevelPart;
//an id and a tile coordinate each fit in a byte, at MAXITEMCOUNT parts the
//difference between this and three ints is over 3k of heap
struct LevelPart
{
	uint8_t id;
	uint8_t x;     //0 .. MAXLINELEN - 2
	uint8_t y;     //line within the level, only kept for levels that fit the playfield
};

typedef struct LevelMeta LevelMeta;
struct LevelMeta
{
	uint16_t maxx;   //0 .. MAXLINELEN - 2, or 1000 when the level has too many parts
	uint16_t maxy;   //line count of the level, bounded by the size of the pack only
	uint16_t parts;  //0 .. MAXITEMCOUNT - 3
	uint8_t minx;    //0 .. NrOfCols
	uint8_t miny;    //0 .. NrOfRows
	char author[MAXAUTHORLEN];
	char comments[MAXCOMMENTLEN];
	char title[MAXTITLELEN];
};

typedef struct CLevelPackFile CLevelPackFile;
struct CLevelPackFile
{
	//only the level that is currently selected is kept in memory, the pack is
	//reparsed from scratch whenever another level is requested
	LevelPart Level[MAXITEMCOUNT];
	LevelMeta Meta;
	char author[MAXAUTHORLEN];
	char set[MAXSETLEN];
	char filename[MaxLevelPackNameLength];
	uint8_t LevelCount;  //0 .. MAXLEVELS
	uint8_t LoadedLevel; //1 based number of the level held in Level / Meta, 0 = none
	bool Loaded;
};

CLevelPackFile* CLevelPackFile_Create();
void CLevelPackFile_Destroy(CLevelPackFile* LevelPackFile);
bool CLevelPackFile_parseText(CLevelPackFile* LevelPackFile, const unsigned char* text, size_t textLen, uint8_t maxWidth, uint8_t maxHeight, int8_t level);
bool CLevelPackFile_loadFile(CLevelPackFile* LevelPackFile, char* filename, uint8_t maxWidth, uint8_t maxHeight, int8_t level);
bool CLevelPackFile_loadLevel(CLevelPackFile* LevelPackFile, int8_t level);

#endif
