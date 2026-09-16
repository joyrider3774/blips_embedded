#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "CWorldParts.h"
#include "CWorldPart.h"
#include "GameFuncs.h"

#if FLOODFILLFLOOR
//the floodfill buffers come and go with the parts list, defined with the floodfill below
static void FloodCreate();
static void FloodDestroy();
#endif

CWorldParts* CWorldParts_Create()
{
	CWorldParts* Result = (CWorldParts*)malloc(sizeof(CWorldParts));
	if (Result)
	{
		Result->ItemCount = 0;
		Result->MoveAbleItemCount = 0;
		Result->DisableSorting = false;
		Result->Player = NULL;
		Result->Player1 = NULL;
		Result->Player2 = NULL;
		Result->ActivePlayer = IDPlayer;
		Result->ActivePlayerFlicker = 0;
		Result->isLevelPackFileLevel = false;
		Result->ViewPort = CViewPort_Create(0, 0, NrOfColsVisible, NrOfRowsVisible, 0, 0, NrOfCols - 1, NrOfRows - 1);
#if FLOODFILLFLOOR
		FloodCreate();
#endif
	}
	return Result;
}

void CWorldParts_SwitchPlayers(CWorldParts* WorldParts)
{
	if (!WorldParts->Player2)
		return;

	if (!WorldParts->Player1 && WorldParts->Player2)
		return;

	if (WorldParts->ActivePlayer == IDPlayer)
	{
		WorldParts->ActivePlayer = IDPlayer2;
		WorldParts->Player = WorldParts->Player2;
	}
	else
	{
		WorldParts->ActivePlayer = IDPlayer;
		WorldParts->Player = WorldParts->Player1;
	}
	WorldParts->ActivePlayerFlicker = 15;
	CWorldParts_CenterVPOnPlayer(WorldParts);
}

void CWorldParts_CenterVPOnPlayer(CWorldParts* WorldParts)
{
	//The window is NrOfColsVisible by NrOfRowsVisible tiles. Asking for
	//centre - half .. centre + half spans one tile too many, because SetViewPort
	//adds another one to the max, and MaxScreenX / MaxScreenY then sit a whole tile
	//past the right / bottom edge of the screen. CViewPort_Move compares those
	//against the level limit, so scrolling stopped a tile early and the last column
	//and row of a level could never be brought into view
	//a player position minus half the visible tiles, that count grows with the resolution
	int16_t VPX, VPY;
	if (WorldParts->Player)
	{
		VPX = WorldParts->Player->PlayFieldX - ((NrOfColsVisible) >> 1);
		VPY = WorldParts->Player->PlayFieldY - ((NrOfRowsVisible) >> 1);
	}
	else
	{
		VPX = (NrOfCols >> 1) - ((NrOfColsVisible) >> 1);
		VPY = (NrOfRows >> 1) - ((NrOfRowsVisible) >> 1);
	}
	CViewPort_SetViewPort(WorldParts->ViewPort, VPX, VPY, VPX + NrOfColsVisible - 1, VPY + NrOfRowsVisible - 1);
	//the whole view just jumped to a new place
	CWorldParts_MarkAllDirty();
}

void CWorldParts_LimitVPLevel(CWorldParts* WorldParts)
{
	//tile positions, but MaxX becomes MinX + NrOfColsVisible - 1 below, which grows with the resolution
    int16_t MinX = NrOfCols, MinY = NrOfRows, MaxX = -1, MaxY = -1;
	uint16_t Teller = 0;
	for (Teller = 0;Teller<WorldParts->ItemCount;Teller++)
	{
		if (WorldParts->Items[Teller]->PlayFieldX < MinX)
			MinX = WorldParts->Items[Teller]->PlayFieldX;
		if (WorldParts->Items[Teller]->PlayFieldY < MinY)
			MinY = WorldParts->Items[Teller]->PlayFieldY;
		if (WorldParts->Items[Teller]->PlayFieldX > MaxX)
			MaxX = WorldParts->Items[Teller]->PlayFieldX;
		if (WorldParts->Items[Teller]->PlayFieldY > MaxY)
            MaxY = WorldParts->Items[Teller]->PlayFieldY;
    }

	// To Center smaller levels
	if (MaxX - MinX < NrOfColsVisible)
	{
		int16_t w = MaxX - MinX;
		MinX -= (NrOfColsVisible - w)/2;
		if (MinX < 0)
			MinX = 0;
		MaxX = MinX + NrOfColsVisible-1;
	}
	if (MaxY - MinY < NrOfRowsVisible)
	{
		int16_t h = MaxY - MinY;
		MinY -= (NrOfRowsVisible - h) / 2;
		if (MinY < 0)
			MinY = 0;
		MaxY = MinY + NrOfRowsVisible-1;
	}

    CViewPort_SetVPLimit(WorldParts->ViewPort, MinX,MinY,MaxX,MaxY);
    CWorldParts_CenterVPOnPlayer(WorldParts);
}

void CWorldParts_RemoveAll(CWorldParts* WorldParts)
{
	uint16_t Teller;
	for (Teller=0;Teller<WorldParts->ItemCount;Teller++)
	{
		CWorldPart_Destroy(WorldParts->Items[Teller]);
		WorldParts->Items[Teller] = NULL;
	}
	WorldParts->ItemCount=0;
	WorldParts->MoveAbleItemCount = 0;
	WorldParts->ActivePlayer = -1;
	WorldParts->Player1 = NULL;
	WorldParts->Player2 = NULL;
	WorldParts->Player = NULL;
}
void CWorldParts_Remove(CWorldParts* WorldParts, int8_t PlayFieldXin,int8_t PlayFieldYin)
{
	uint16_t Teller1,Teller2;

	for (Teller1=0;Teller1<WorldParts->ItemCount;Teller1++)
	{
		if ((WorldParts->Items[Teller1]->PlayFieldX == PlayFieldXin) && (WorldParts->Items[Teller1]->PlayFieldY == PlayFieldYin))
		{
			bool checkPlayer = false;
			if(WorldParts->Items[Teller1]->Type == IDPlayer)
			{
				checkPlayer = true;
				WorldParts->Player1 = NULL;
			}
			if(WorldParts->Items[Teller1]->Type == IDPlayer2)
			{
				checkPlayer = true;
				WorldParts->Player2 = NULL;
			}
			if(checkPlayer)
			{
				if((WorldParts->Player1 && WorldParts->Player2) ||
					(WorldParts->Player1 && !WorldParts->Player2))
				{
					WorldParts->Player = WorldParts->Player1;
					WorldParts->ActivePlayer = IDPlayer;
				}

				if (!WorldParts->Player1 && WorldParts->Player2)
				{
					WorldParts->Player = WorldParts->Player2;
					WorldParts->ActivePlayer = IDPlayer2;
				}

				if (!WorldParts->Player1 && !WorldParts->Player2)
				{
					WorldParts->ActivePlayer = -1;
					WorldParts->Player = NULL;
				}
			}
			//the square this part occupied has to be painted again
			CWorldParts_MarkDirty(WorldParts->Items[Teller1]->X - WorldParts->ViewPort->MinScreenX,
			                      WorldParts->Items[Teller1]->Y - WorldParts->ViewPort->MinScreenY,
			                      TileWidth, TileHeight);
			CWorldPart_Destroy(WorldParts->Items[Teller1]);
			for (Teller2=Teller1;Teller2<WorldParts->ItemCount-1;Teller2++)
				WorldParts->Items[Teller2] = WorldParts->Items[Teller2+1];
			WorldParts->ItemCount--;
			if(Teller1 > 0)
				Teller1--;
		}
	}
}

void CWorldParts_Remove_Type(CWorldParts* WorldParts, int8_t PlayFieldXin,int8_t PlayFieldYin,uint8_t Type)
{
	uint16_t Teller1,Teller2;

	for (Teller1=0;Teller1<WorldParts->ItemCount;Teller1++)
	{
		if ((WorldParts->Items[Teller1]->PlayFieldX == PlayFieldXin) && (WorldParts->Items[Teller1]->PlayFieldY == PlayFieldYin) && (WorldParts->Items[Teller1]->Type == Type))
		{
			bool checkPlayer = false;
			if(WorldParts->Items[Teller1]->Type == IDPlayer)
			{
				checkPlayer = true;
				WorldParts->Player1 = NULL;
			}
			if(WorldParts->Items[Teller1]->Type == IDPlayer2)
			{
				checkPlayer = true;
				WorldParts->Player2 = NULL;
			}
			if(checkPlayer)
			{
				if((WorldParts->Player1 && WorldParts->Player2) ||
					(WorldParts->Player1 && !WorldParts->Player2))
				{
					WorldParts->Player = WorldParts->Player1;
					WorldParts->ActivePlayer = IDPlayer;
				}

				if (!WorldParts->Player1 && WorldParts->Player2)
				{
					WorldParts->Player = WorldParts->Player2;
					WorldParts->ActivePlayer = IDPlayer2;
				}

				if (!WorldParts->Player1 && !WorldParts->Player2)
				{
					WorldParts->ActivePlayer = -1;
					WorldParts->Player = NULL;
				}
			}

	
			//the square this part occupied has to be painted again
			CWorldParts_MarkDirty(WorldParts->Items[Teller1]->X - WorldParts->ViewPort->MinScreenX,
			                      WorldParts->Items[Teller1]->Y - WorldParts->ViewPort->MinScreenY,
			                      TileWidth, TileHeight);
			CWorldPart_Destroy(WorldParts->Items[Teller1]);
			for (Teller2=Teller1;Teller2<WorldParts->ItemCount-1;Teller2++)
				WorldParts->Items[Teller2] = WorldParts->Items[Teller2+1];
			WorldParts->ItemCount--;
			if(Teller1 > 0)
				Teller1--;
		}
	}
}

void CWorldParts_Add(CWorldParts* WorldParts, CWorldPart *WorldPart)
{
	//CWorldPart_Create returns NULL once the part pool is exhausted (or could not be
	//allocated), adding that
	//to the list would take the whole board down on the next draw
	if (!WorldPart)
		return;

	CWorldParts_MarkDirty(WorldPart->X - WorldParts->ViewPort->MinScreenX,
	                      WorldPart->Y - WorldParts->ViewPort->MinScreenY,
	                      TileWidth, TileHeight);

	if( WorldParts->ItemCount < MAXWORLDPARTS )
	{
		WorldParts->Items[WorldParts->ItemCount] = WorldPart;
		WorldParts->ItemCount++;
		CWorldParts_Sort(WorldParts);
	}

	if (WorldPart->Type == IDPlayer)
	{
		WorldParts->Player1 = WorldPart;
		WorldParts->Player = WorldParts->Player1;
		WorldParts->ActivePlayer = IDPlayer;
	}

	if (WorldPart->Type == IDPlayer2)
	{
		WorldParts->Player2 = WorldPart;
		WorldParts->Player = WorldParts->Player2;
		WorldParts->ActivePlayer = IDPlayer2;
	}
	
}


void CWorldParts_Sort(CWorldParts* WorldParts)
{
	uint16_t Teller1,Teller2;
	uint8_t Index;
	CWorldPart *Part;
	if (!WorldParts->DisableSorting)
	{
		for (Teller1 = 1; Teller1 <WorldParts->ItemCount;Teller1++)
		{
			Index = WorldParts->Items[Teller1]->Z;
			Part = WorldParts->Items[Teller1];
			Teller2 = Teller1;
			while ((Teller2 > 0) && (WorldParts->Items[Teller2-1]->Z > Index))
			{
				WorldParts->Items[Teller2] = WorldParts->Items[Teller2 - 1];
				Teller2--;
			}
			WorldParts->Items[Teller2] = Part;
		}
	}

}

bool CWorldParts_LoadFromLevelPackFile(CWorldParts* WorldParts, CLevelPackFile* LPFile, int8_t level, bool doCenterLevel)
{
	WorldParts->isLevelPackFileLevel = false;
	CWorldParts_RemoveAll(WorldParts);
	if ((level >= 1) && (level <= LPFile->LevelCount))
	{
		//only one level is kept in memory, reparse the pack if another one is wanted
		if (LPFile->LoadedLevel != level)
			if (!CLevelPackFile_loadLevel(LPFile, level))
				return false;

		WorldParts->isLevelPackFileLevel = true;
		WorldParts->DisableSorting = true;
		//a loaded level fits the playfield, so these stay within +-NrOfCols / 2 and +-NrOfRows / 2
		int8_t Xi = ((NrOfCols - 1) / 2) - (LPFile->Meta.maxx + LPFile->Meta.minx) / 2;
		int8_t Yi = ((NrOfRows - 1) / 2) - (LPFile->Meta.maxy + LPFile->Meta.miny) / 2;
		if (!doCenterLevel)
		{
			Xi = 0;
			Yi = 0;
		}

		for (uint16_t i = 0; i < LPFile->Meta.parts; i++)
		{
			uint8_t Type = LPFile->Level[i].id;
			int8_t X = LPFile->Level[i].x + Xi;
			int8_t Y = LPFile->Level[i].y + Yi;
			if (Type != IDFloor)
				CWorldParts_Add(WorldParts, CWorldPart_Create(X, Y, Type));
		}
		WorldParts->DisableSorting = false;
		if (WorldParts->Player1 && WorldParts->Player2)
		{
			WorldParts->Player = WorldParts->Player1;
			WorldParts->ActivePlayer = IDPlayer;
		}
		CWorldParts_Sort(WorldParts);
		CWorldParts_LimitVPLevel(WorldParts);
		return true;
	}
	return false;
}

bool CWorldParts_CenterLevel(CWorldParts* WorldParts)
{
	int8_t MinX = NrOfCols - 1;
	int8_t MinY = NrOfRows - 1;
	int8_t MaxX = 0;
	int8_t MaxY = 0;
	for (uint16_t Teller = 0; Teller < WorldParts->ItemCount; Teller++)
	{
		if (WorldParts->Items[Teller]->PlayFieldX < MinX)
			MinX = WorldParts->Items[Teller]->PlayFieldX;
		if (WorldParts->Items[Teller]->PlayFieldY < MinY)
			MinY = WorldParts->Items[Teller]->PlayFieldY;
		if (WorldParts->Items[Teller]->PlayFieldX > MaxX)
			MaxX = WorldParts->Items[Teller]->PlayFieldX;
		if (WorldParts->Items[Teller]->PlayFieldY > MaxY)
			MaxY = WorldParts->Items[Teller]->PlayFieldY;
	}
	int8_t Xi = ((NrOfCols - 1) / 2) - (MaxX + MinX) / 2;
	int8_t Yi = ((NrOfRows - 1) / 2) - (MaxY + MinY) / 2;
	for (uint16_t Teller = 0; Teller < WorldParts->ItemCount; Teller++)
	{
		CWorldPart_SetPosition(WorldParts->Items[Teller], WorldParts->Items[Teller]->PlayFieldX + Xi, WorldParts->Items[Teller]->PlayFieldY + Yi);
	}
	if (Xi != 0 || Yi != 0)
		return true;
	return false;
}

bool CWorldParts_ItemExists(CWorldParts* WorldParts, int8_t PlayFieldXin, int8_t PlayFieldYin, uint8_t Type)
{
	for (uint16_t Teller1 = 0; Teller1 < WorldParts->ItemCount; Teller1++)
	{
		if ((WorldParts->Items[Teller1]->PlayFieldX == PlayFieldXin) && (WorldParts->Items[Teller1]->PlayFieldY == PlayFieldYin) && (WorldParts->Items[Teller1]->Type == Type))
			return true;
	}
	return false;
}

#if FLOODFILLFLOOR
//One bit per playfield tile instead of one byte, these are only ever flags.
#define TILEBITS ((NrOfRows * NrOfCols + 7) / 8)
#define TILEBIT(x, y) ((y) * NrOfCols + (x))
static_assert(NrOfRows * NrOfCols <= 65535, "tile indexes do not fit in uint16_t");
static inline bool BitGet(const uint8_t* bits, uint16_t i) { return (bits[i >> 3] >> (i & 7)) & 1; }
static inline void BitSet(uint8_t* bits, uint16_t i) { bits[i >> 3] |= (uint8_t)1 << (i & 7); }

//everything the floodfill works with, kept in one block so it is a single allocation
typedef struct FloodBuffers FloodBuffers;
struct FloodBuffers
{
	uint8_t visited[TILEBITS];
	//playfield tiles that hold a wall, filled once before the floodfills so they do not have
	//to look through every part of the level for every tile they visit
	uint8_t wallHere[TILEBITS];
	//playfield tiles the floodfill decided are floor, this is what gets painted
	uint8_t floorHere[TILEBITS];
#if SCREENBUFFER == 0
	uint8_t floorPrev[TILEBITS];
#endif
	//tiles still to handle, held as Y * NrOfCols + X. A tile is marked visited when it
	//is pushed, so it can enter this list only once and the list can never hold more
	//tiles than the playfield has
	uint16_t floodStack[NrOfRows * NrOfCols];
};
//taken from the heap by CWorldParts_Create and handed back by CWorldParts_Destroy.
//NULL outside of that, or when the allocation failed
static FloodBuffers* Flood = NULL;
static uint16_t floodStackCount = 0;

static void FloodCreate()
{
	if (Flood)
		return;
	//calloc zeroes floorPrev, as the old static array started out
	Flood = (FloodBuffers*)calloc(1, sizeof(FloodBuffers));
	if (!Flood)
		Platform_Log("FloodCreate: out of heap, %" PRIu32 " free\n", Platform_FreeHeap());
}

static void FloodDestroy()
{
	free(Flood);
	Flood = NULL;
}

//spreading passes one tile past the playfield on either side, so X and Y are signed
static void FloodPush(int8_t X, int8_t Y)
{
	if (X < 0 || X >= NrOfCols || Y < 0 || Y >= NrOfRows || BitGet(Flood->visited, TILEBIT(X, Y)))
		return;
	BitSet(Flood->visited, TILEBIT(X, Y));
	Flood->floodStack[floodStackCount++] = (uint16_t)(Y * NrOfCols + X);
}

// Floodfill, iterative. Recursing here used one call frame per open tile, which
// overflowed the 4k sketch stack on levels with a large open area
void FloodFill(CWorldParts* aWorldParts, int8_t X, int8_t Y)
{
	floodStackCount = 0;
	FloodPush(X, Y);

	while (floodStackCount > 0)
	{
		uint16_t Tile = Flood->floodStack[--floodStackCount];
		uint8_t TileX = Tile % NrOfCols;
		uint8_t TileY = Tile / NrOfCols;

		//outside the viewport, nothing to draw and nothing to spread to
		if ((TileX < aWorldParts->ViewPort->VPMinX) || (TileX > aWorldParts->ViewPort->VPMaxX) ||
			(TileY < aWorldParts->ViewPort->VPMinY) || (TileY > aWorldParts->ViewPort->VPMaxY))
		{
			continue;
		}

		//a wall blocks the fill
		if (BitGet(Flood->wallHere, TILEBIT(TileX, TileY)))
		{
			continue;
		}

		// Remember that this tile shows floor, the compositor paints it
		BitSet(Flood->floorHere, TILEBIT(TileX, TileY));

		// Spread to the neighbouring tiles
		FloodPush(TileX + 1, TileY);
		FloodPush(TileX - 1, TileY);
		FloodPush(TileX, TileY + 1);
		FloodPush(TileX, TileY - 1);
	}
}

//false when there are no floodfill buffers, Flood->floorHere can not be read then
bool CWorldParts_DrawFloor(CWorldParts* WorldParts, CWorldPart* Player, CWorldPart* Player2)
{
	// This runs every frame, the buffers were allocated once by CWorldParts_Create
	if (!Flood)
		return false;
	if (!Player && !Player2)
		return true;
	memset(Flood->visited, 0, sizeof(Flood->visited));
	memset(Flood->floorHere, 0, sizeof(Flood->floorHere));
	//the same parts CWorldParts_ItemExists(..., IDWall) finds, in one pass over the level
	memset(Flood->wallHere, 0, sizeof(Flood->wallHere));
	for (uint16_t Teller = 0; Teller < WorldParts->ItemCount; Teller++)
	{
		CWorldPart* Part = WorldParts->Items[Teller];
		if ((Part->Type == IDWall) && (Part->PlayFieldX >= 0) && (Part->PlayFieldX < NrOfCols) &&
			(Part->PlayFieldY >= 0) && (Part->PlayFieldY < NrOfRows))
			BitSet(Flood->wallHere, TILEBIT(Part->PlayFieldX, Part->PlayFieldY));
	}
	if(Player)
		FloodFill(WorldParts, Player->PlayFieldX, Player->PlayFieldY);
	if(Player2)
		FloodFill(WorldParts, Player2->PlayFieldX, Player2->PlayFieldY);
	return true;
}
#endif
void CWorldParts_Move(CWorldParts* WorldParts)
{
	uint16_t Teller;

	WorldParts->MoveAbleItemCount = 0;
	for (Teller = 0; Teller < WorldParts->ItemCount; Teller++)
	{
		if (WorldParts->Items[Teller]->IsMoving)
		{
			//other items are not moveable and have no effect
			if (!WorldParts->Items[Teller]->PNeedToKill && !WorldParts->Items[Teller]->BHide)
			{
				CWorldParts_MarkDirty(WorldParts->Items[Teller]->X - WorldParts->ViewPort->MinScreenX,
				                      WorldParts->Items[Teller]->Y - WorldParts->ViewPort->MinScreenY,
				                      TileWidth, TileHeight);
				CWorldPart_Move(WorldParts->Items[Teller]);
				CWorldParts_MarkDirty(WorldParts->Items[Teller]->X - WorldParts->ViewPort->MinScreenX,
				                      WorldParts->Items[Teller]->Y - WorldParts->ViewPort->MinScreenY,
				                      TileWidth, TileHeight);
				if (!WorldParts->Items[Teller]->PNeedToKill && !WorldParts->Items[Teller]->BHide)
					if (WorldParts->MoveAbleItemCount < MAXMOVEABLEWORLDPARTS)
						WorldParts->MoveAbleItems[WorldParts->MoveAbleItemCount++] = WorldParts->Items[Teller];
			}
		}
	}
}

void CWorldParts_Draw(CWorldParts* WorldParts)
{
	for (uint16_t Teller=0;Teller<WorldParts->ItemCount;Teller++)
	{
	    if(WorldParts->Items[Teller]->PNeedToKill)
	    {
	         CWorldParts_Remove_Type(WorldParts, WorldParts->Items[Teller]->PlayFieldX,WorldParts->Items[Teller]->PlayFieldY,WorldParts->Items[Teller]->Type);
	         //go back one item to prevent skips & segaults, from 0 this wraps to 0xFFFF
	         //and the loop increment brings it back to 0
	         Teller--;
	     }
		else
		{
			if ((WorldParts->Items[Teller]->PlayFieldX >= WorldParts->ViewPort->VPMinX) && (WorldParts->Items[Teller]->PlayFieldX - 1 <= WorldParts->ViewPort->VPMaxX) &&
				(WorldParts->Items[Teller]->PlayFieldY >= WorldParts->ViewPort->VPMinY) && (WorldParts->Items[Teller]->PlayFieldY - 1 <= WorldParts->ViewPort->VPMaxY))
			{
				if (WorldParts->ActivePlayerFlicker > 0)
				{
					if (WorldParts->Items[Teller]->Type == WorldParts->ActivePlayer)
					{
						if (WorldParts->ActivePlayerFlicker % 3 == 1)
							CWorldPart_Draw(WorldParts->Items[Teller]);
					}
					else
						CWorldPart_Draw(WorldParts->Items[Teller]);

				}
				else
					CWorldPart_Draw(WorldParts->Items[Teller]);
			}			
		}
	}

	//Redraw moving items so they are always drawn on top
	for (uint8_t Teller = 0; Teller < WorldParts->MoveAbleItemCount; Teller++)
		CWorldPart_Draw(WorldParts->MoveAbleItems[Teller]);
		
	if (WorldParts->ActivePlayerFlicker > 0)
		WorldParts->ActivePlayerFlicker--;
}

// ===========================================================================
// Dirty band board rendering
//
// The screen is tracked as a grid of 8x8 cells, but repainting is done a row of
// cells at a time: the dirty cells of a row are composed together in one buffer
// and sent in a single transfer. Standing still costs nothing, walking repaints
// the two rows the player covers, and a scroll costs 16 transfers instead of the
// 256 that pushing single cells needed. Nothing part drawn ever reaches the
// display, so there is no flicker either way.
// ===========================================================================

#define CELLSX (WINDOW_WIDTH / TileWidth)
#define CELLSY (WINDOW_HEIGHT / TileHeight)
//one bit per cell of a row, the smallest type that has a bit for every cell the
//screen is wide (16 at 128 px wide with 8x8 tiles, 20 at 320 px with 16x16 tiles)
#if CELLSX <= 8
typedef uint8_t CellRow;
#elif CELLSX <= 16
typedef uint16_t CellRow;
#elif CELLSX <= 32
typedef uint32_t CellRow;
#else
typedef uint64_t CellRow;
#endif
static_assert(CELLSX <= 64, "cellDirty keeps one bit per cell of a row in at most a uint64_t");

static CellRow cellDirty[CELLSY];
#if SCREENBUFFER == 0
//Half of a cell row at a time. The display window is still opened once per row,
//the two halves just go through it back to back, so this costs a second
//pushPixels rather than a second window setup.
#define BANDHEIGHT (TileHeight / 2)
static uint16_t bandBuf[WINDOW_WIDTH * BANDHEIGHT]; //the strip being composed
static int16_t bandX0, bandY0, bandW;               //where that strip sits on screen
static int16_t lastMinScreenX = -30000, lastMinScreenY = -30000;

//The background is run length encoded (see pushImageRLE). To start decoding in the
//middle of it, this keeps for every screen row the control byte the row starts in
//and how many pixels of that control belong to the rows above it.
//an encoded background is at most 3 bytes per pixel (a one pixel run every time),
//so the offsets need 32 bits once the screen is bigger than about 147x147
#if WINDOW_WIDTH * WINDOW_HEIGHT * 3 < 65536
typedef uint16_t BgOffset;
#else
typedef uint32_t BgOffset;
#endif
static BgOffset bgRowOffset[WINDOW_HEIGHT];
static uint8_t bgRowUsed[WINDOW_HEIGHT];
static const uint8_t* bgIndexed = NULL;

//pixels are little endian RGB565, read per byte as the images are uint8_t arrays
static inline uint16_t ReadPixel(const uint8_t* p)
{
	return PLATFORM_READ_BYTE(p) | (PLATFORM_READ_BYTE(p + 1) << 8);
}
#endif

//marking is called from all over the world part code, with a buffer it is unused but harmless
void CWorldParts_MarkDirty(int16_t x, int16_t y, int16_t w, int16_t h)
{
	if ((w <= 0) || (h <= 0))
		return;
	int16_t x1 = x + w - 1;
	int16_t y1 = y + h - 1;
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (x1 > WINDOW_WIDTH - 1) x1 = WINDOW_WIDTH - 1;
	if (y1 > WINDOW_HEIGHT - 1) y1 = WINDOW_HEIGHT - 1;
	if ((x > x1) || (y > y1))
		return;
	for (int16_t cy = y / TileHeight; cy <= y1 / TileHeight; cy++)
		for (int16_t cx = x / TileWidth; cx <= x1 / TileWidth; cx++)
			cellDirty[cy] |= (CellRow)((CellRow)1 << cx);
}

void CWorldParts_MarkAllDirty()
{
	for (int16_t cy = 0; cy < CELLSY; cy++)
		//every bit up to CELLSX, shifting by the full width of the type is undefined
		cellDirty[cy] = (CellRow)((CellRow)~(CellRow)0 >> (sizeof(CellRow) * 8 - CELLSX));
}


#if SCREENBUFFER
//With a buffer the whole frame is drawn off screen and sent in one go, so there is
//nothing to track: the board is simply drawn again every frame, which is what the
//game did before any of the dirty cell work.
bool CWorldParts_DrawBoard(CWorldParts* WorldParts)
{
	pushImageRLE(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, IMGBackground);

#if FLOODFILLFLOOR
	//the floodfill only records which tiles are floor, stamp them here
	const int16_t msx = WorldParts->ViewPort->MinScreenX;
	const int16_t msy = WorldParts->ViewPort->MinScreenY;
	//without the floodfill buffers there is no floor to paint this frame
	if (CWorldParts_DrawFloor(WorldParts, WorldParts->Player1, WorldParts->Player2))
	{
		for (uint8_t ty = 0; ty < NrOfRows; ty++)
			for (uint8_t tx = 0; tx < NrOfCols; tx++)
				if (BitGet(Flood->floorHere, TILEBIT(tx, ty)))
					DrawImage(tx * TileWidth - msx, ty * TileHeight - msy, TileWidth, TileHeight, IMGFloor);
	}
#endif

	CWorldParts_Draw(WorldParts);

	//every pixel of the board was drawn, anything that sits on top has to follow
	return true;
}
#else
//the square a part occupies on screen
static void MarkPartDirty(CWorldParts* WorldParts, CWorldPart* Part)
{
	CWorldParts_MarkDirty(Part->X - WorldParts->ViewPort->MinScreenX,
	                      Part->Y - WorldParts->ViewPort->MinScreenY,
	                      TileWidth, TileHeight);
}

static void IndexBackground()
{
	const uint8_t* data = IMGBackground;
	uint32_t pixel = 0; //first pixel the current control covers
	int16_t row = 0;
	while (row < WINDOW_HEIGHT)
	{
		uint8_t control = PLATFORM_READ_BYTE(data);
		uint32_t count = (control & 0x7F) + 1;
		//every row that starts inside this control
		while ((row < WINDOW_HEIGHT) && ((uint32_t)row * WINDOW_WIDTH < pixel + count))
		{
			bgRowOffset[row] = (BgOffset)(data - IMGBackground);
			bgRowUsed[row] = (uint8_t)((uint32_t)row * WINDOW_WIDTH - pixel);
			row++;
		}
		pixel += count;
		data += (control & 0x80) ? 3 : 1 + count * 2;
	}
	bgIndexed = IMGBackground;
}

//the background is a full screen image so it lines up with the band
static void BandBackground()
{
	if (!IMGBackground)
	{
		for (uint16_t i = 0; i < bandW * BANDHEIGHT; i++)
			bandBuf[i] = ColorWhite;
		return;
	}
	//a new skin brings a new background
	if (bgIndexed != IMGBackground)
		IndexBackground();

	for (int16_t r = 0; r < BANDHEIGHT; r++)
	{
		const uint8_t* data = IMGBackground + bgRowOffset[bandY0 + r];
		//used / count / avail stay in 1..128, a control never covers more pixels than that.
		//skip and left are screen positions / widths, so they grow with WINDOW_WIDTH
		uint8_t used = bgRowUsed[bandY0 + r]; //pixels of this control that lie before the row
		uint16_t skip = bandX0;               //pixels of the row left of the strip
		uint16_t left = bandW;
		uint16_t* drow = &bandBuf[r * bandW];
		while (left > 0)
		{
			uint8_t control = PLATFORM_READ_BYTE(data);
			uint8_t count = (control & 0x7F) + 1;
			bool run = (control & 0x80) != 0;
			uint8_t avail = count - used;
			if (skip >= avail)
				skip -= avail;
			else
			{
				used += skip;
				avail -= skip;
				skip = 0;
				if (avail > left)
					avail = left;
				if (run)
				{
					uint16_t col = ReadPixel(data + 1);
					for (uint8_t i = 0; i < avail; i++)
						*drow++ = col;
				}
				else
				{
					const uint8_t* src = data + 1 + used * 2;
					for (uint8_t i = 0; i < avail; i++, src += 2)
						*drow++ = ReadPixel(src);
				}
				left -= avail;
			}
			used = 0;
			data += run ? 3 : 1 + count * 2;
		}
	}
}

//blit an 8x8 sprite that sits at screen position sx,sy, clipped to the band
static void BandSprite(int16_t sx, int16_t sy, const uint8_t* image, bool keyed)
{
	if (!image)
		return;
	//cheap reject before touching the pixels
	if ((sy + TileHeight <= bandY0) || (sy >= bandY0 + BANDHEIGHT) ||
		(sx + TileWidth <= bandX0) || (sx >= bandX0 + bandW))
		return;

	const uint16_t* src = (const uint16_t*)image;
	for (int16_t r = 0; r < TileHeight; r++)
	{
		int16_t dy = sy + r - bandY0;
		if ((dy < 0) || (dy >= BANDHEIGHT))
			continue;
		uint16_t* drow = &bandBuf[dy * bandW];
		const uint16_t* srow = &src[r * TileWidth];
		for (int16_t c = 0; c < TileWidth; c++)
		{
			int16_t dx = sx + c - bandX0;
			if ((dx < 0) || (dx >= bandW))
				continue;
			uint16_t col = PLATFORM_READ_WORD(&srow[c]);
			//magenta is the transparent key, 0xF81F in RGB565
			if (keyed && (col == 0xF81F))
				continue;
			drow[dx] = col;
		}
	}
}

//is this part visible, same test the old full screen draw used
static bool PartVisible(CWorldParts* WorldParts, CWorldPart* Part)
{
	if (Part->BHide)
		return false;
	if ((Part->PlayFieldX < WorldParts->ViewPort->VPMinX) || (Part->PlayFieldX - 1 > WorldParts->ViewPort->VPMaxX) ||
		(Part->PlayFieldY < WorldParts->ViewPort->VPMinY) || (Part->PlayFieldY - 1 > WorldParts->ViewPort->VPMaxY))
		return false;
	if (WorldParts->ActivePlayerFlicker > 0)
		if (Part->Type == WorldParts->ActivePlayer)
			if (WorldParts->ActivePlayerFlicker % 3 != 1)
				return false;
	return true;
}

bool CWorldParts_DrawBoard(CWorldParts* WorldParts)
{
	uint16_t Teller;
	bool painted = false;
	//read once, these are used for every part of every strip
	const int16_t msx = WorldParts->ViewPort->MinScreenX;
	const int16_t msy = WorldParts->ViewPort->MinScreenY;

	//parts that are about to disappear leave a hole behind them
	for (Teller = 0; Teller < WorldParts->ItemCount; Teller++)
		if (WorldParts->Items[Teller]->PNeedToKill)
		{
			MarkPartDirty(WorldParts, WorldParts->Items[Teller]);
			CWorldParts_Remove_Type(WorldParts, WorldParts->Items[Teller]->PlayFieldX,
			                        WorldParts->Items[Teller]->PlayFieldY, WorldParts->Items[Teller]->Type);
			//from 0 this wraps to 0xFFFF and the loop increment brings it back to 0
			Teller--;
		}

	//animation runs once per part per frame, exactly where the old draw did it
	for (Teller = 0; Teller < WorldParts->ItemCount; Teller++)
		if (PartVisible(WorldParts, WorldParts->Items[Teller]))
			CWorldPart_Event_BeforeDraw(WorldParts->Items[Teller]);
	for (Teller = 0; Teller < WorldParts->MoveAbleItemCount; Teller++)
		if (!WorldParts->MoveAbleItems[Teller]->BHide)
			CWorldPart_Event_BeforeDraw(WorldParts->MoveAbleItems[Teller]);

	//the viewport scrolled, so every pixel on screen moved
	if ((WorldParts->ViewPort->MinScreenX != lastMinScreenX) ||
		(WorldParts->ViewPort->MinScreenY != lastMinScreenY))
	{
		lastMinScreenX = WorldParts->ViewPort->MinScreenX;
		lastMinScreenY = WorldParts->ViewPort->MinScreenY;
		CWorldParts_MarkAllDirty();
	}

#if FLOODFILLFLOOR
	//work out where floor is, and repaint everything if that changed
	//without the floodfill buffers there is no floor to paint this frame
	const bool hasFloor = CWorldParts_DrawFloor(WorldParts, WorldParts->Player1, WorldParts->Player2);
	if (hasFloor && (memcmp(Flood->floorHere, Flood->floorPrev, sizeof(Flood->floorHere)) != 0))
	{
		memcpy(Flood->floorPrev, Flood->floorHere, sizeof(Flood->floorHere));
		CWorldParts_MarkAllDirty();
	}
#endif

	//anything moving or animating repaints its own square
	for (Teller = 0; Teller < WorldParts->ItemCount; Teller++)
	{
		CWorldPart* Part = WorldParts->Items[Teller];
		//Event_BeforeDraw above has already produced this frame's AnimPhase, so
		//comparing it with the one last painted catches every appearance change.
		//That includes a player held against a wall it can not walk into, which
		//turns and steps in place while IsMoving stays false
		if (Part->IsMoving || (Part->AnimPhase != Part->LastAnimPhase) ||
			((WorldParts->ActivePlayerFlicker > 0) && (Part->Type == WorldParts->ActivePlayer)))
			MarkPartDirty(WorldParts, Part);
		//anything marked above is repainted below, so this is what it will show
		Part->LastAnimPhase = Part->AnimPhase;
	}

	//compose and push one strip per row that has dirty cells
	for (int16_t cy = 0; cy < CELLSY; cy++)
	{
		if (!cellDirty[cy])
			continue;

		//the run from the first to the last dirty cell of the row goes out as one
		int16_t first = -1, last = -1;
		for (int16_t cx = 0; cx < CELLSX; cx++)
			if (cellDirty[cy] & ((CellRow)1 << cx))
			{
				if (first < 0)
					first = cx;
				last = cx;
			}
		cellDirty[cy] = 0;

		bandX0 = first * TileWidth;
		bandW = (last - first + 1) * TileWidth;

		//the chip select sits on the I/O expander, every write transaction costs I2C
		//traffic, so one transaction is kept open for all the rows of this frame
		if (!painted)
			SCREEN.startWrite();
		//one window for the whole row, the halves are streamed into it in order
		SCREEN.setAddrWindow(bandX0, cy * TileHeight, bandW, TileHeight);

		for (int16_t half = 0; half < TileHeight; half += BANDHEIGHT)
		{
			bandY0 = cy * TileHeight + half;

			BandBackground();

#if FLOODFILLFLOOR
			//floor, only the playfield tiles that reach into this strip
			int16_t wx0 = bandX0 + msx;
			int16_t wx1 = bandX0 + bandW - 1 + msx;
			int16_t wy = bandY0 + msy;
			for (int16_t ty = wy / TileHeight; ty <= (wy + BANDHEIGHT - 1) / TileHeight; ty++)
				for (int16_t tx = wx0 / TileWidth; tx <= wx1 / TileWidth; tx++)
					if (hasFloor && (tx >= 0) && (tx < NrOfCols) && (ty >= 0) && (ty < NrOfRows) && BitGet(Flood->floorHere, TILEBIT(tx, ty)))
						BandSprite(tx * TileWidth - msx, ty * TileHeight - msy, IMGFloor, false);
#endif

			//the parts, in the order the list is sorted so layering is kept
			for (Teller = 0; Teller < WorldParts->ItemCount; Teller++)
			{
				CWorldPart* Part = WorldParts->Items[Teller];
				int16_t sy = Part->Y - msy;
				if ((sy + TileHeight <= bandY0) || (sy >= bandY0 + BANDHEIGHT))
					continue;
				if (!PartVisible(WorldParts, Part))
					continue;
				BandSprite(Part->X - msx, sy, CWorldPart_SpriteData(Part), true);
			}

			//moving parts go on top, as they did before
			for (Teller = 0; Teller < WorldParts->MoveAbleItemCount; Teller++)
			{
				CWorldPart* Part = WorldParts->MoveAbleItems[Teller];
				if (Part->BHide)
					continue;
				BandSprite(Part->X - msx, Part->Y - msy, CWorldPart_SpriteData(Part), true);
			}

#if LOVYANGFX
			//true: bandBuf holds plain RGB565, the library puts it in display order
			SCREEN.writePixels((const uint16_t*)bandBuf, bandW * BANDHEIGHT, true);
#else
			SCREEN.pushPixels(bandBuf, bandW * BANDHEIGHT);
#endif
		}
		painted = true;
	}
	if (painted)
		SCREEN.endWrite();

	if (WorldParts->ActivePlayerFlicker > 0)
		WorldParts->ActivePlayerFlicker--;

	return painted;
}
#endif


void CWorldParts_Destroy(CWorldParts* WorldParts)
{
    CViewPort_Destroy(WorldParts->ViewPort);
	uint16_t Teller;
	for (Teller=0;Teller<WorldParts->ItemCount;Teller++)
	{
		CWorldPart_Destroy(WorldParts->Items[Teller]);
		WorldParts->Items[Teller] = NULL;
	}
#if FLOODFILLFLOOR
	FloodDestroy();
#endif
}
