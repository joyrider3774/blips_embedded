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
#if SCREENBUFFER == 0
//the same for the strip the board is composed in, defined with the drawing below
static void BandBufCreate();
static void BandBufDestroy();
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
#if SCREENBUFFER == 0
		BandBufCreate();
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

#if SCREENBUFFER == 0
//A part whose sprite holds no transparent pixel covers its whole tile: it is painted
//without looking at the transparent key and the background under it is left undecoded.
//Whether that holds is a property of the skin, LoadGraphics works it out per image
static inline bool PartOpaque(uint8_t Type)
{
	switch (Type)
	{
		case IDBox:
		case IDBox1:
		case IDBox2:
		case IDBoxWall:
		case IDBoxBomb:        return IMGBoxOpaque;
		case IDWall:
		case IDWallBreakable:  return IMGWallOpaque;
		case IDFloor:          return IMGFloorOpaque;
		default:               return false;
	}
}
#endif

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
	//playfield tiles an opaque sprite covers completely: the floor above, plus the walls and
	//the boxes that sit still on their tile. The background under them is never seen, so it
	//is not decoded (see BandFindCovered)
	uint8_t coverHere[TILEBITS];
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
	{
#if SCREENBUFFER == 0
		//floorHere keeps what it held, but nothing here says where the sprites are now,
		//so the background is decoded everywhere this frame
		memset(Flood->coverHere, 0, sizeof(Flood->coverHere));
#endif
		return true;
	}
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

#if SCREENBUFFER == 0
	//a floor tile hides the background, as long as this skin's floor covers its tile
	if (IMGFloorOpaque)
		memcpy(Flood->coverHere, Flood->floorHere, sizeof(Flood->coverHere));
	else
		memset(Flood->coverHere, 0, sizeof(Flood->coverHere));
	for (uint16_t Teller = 0; Teller < WorldParts->ItemCount; Teller++)
	{
		CWorldPart* Part = WorldParts->Items[Teller];
		//a part only hides a whole tile while it sits on one: a moving one is between two
		if (!PartOpaque(Part->Type) || Part->BHide ||
			(Part->X != Part->PlayFieldX * TileWidth) || (Part->Y != Part->PlayFieldY * TileHeight))
			continue;
		if ((Part->PlayFieldX >= 0) && (Part->PlayFieldX < NrOfCols) &&
			(Part->PlayFieldY >= 0) && (Part->PlayFieldY < NrOfRows))
			BitSet(Flood->coverHere, TILEBIT(Part->PlayFieldX, Part->PlayFieldY));
	}
#endif
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
//A whole cell row at a time. Half rows cost twice the work for every sprite: a part is
//TileHeight tall, so it lands in both halves and its pixels are walked for each of them.
//A full row is one window, one push and every part handled once
#define BANDHEIGHT (TileHeight)
//allocated with the parts list rather than reserved as a global, so nothing that runs
//instead of the game pays for it. NULL when it could not be allocated, and the board is
//then not painted
static uint16_t* bandBuf = NULL;                    //the strip being composed
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

//count pixels of an image into the strip. Runs here are a few pixels at a time, a sprite
//row is 8 of them: where flash is plain memory that is a short copy of 16 bit values, and
//calling memcpy for it costs more than the copy itself
static inline void BandCopy(uint16_t* dst, const uint8_t* src, int16_t count)
{
#if PLATFORM_DIRECT_FLASH
	//Four bytes at a time where both sides sit on an address that allows it: a 32 bit machine
	//then moves two pixels per load and per store. A 16 bit read needs an even address (a core
	//like the Cortex-M0+ faults on an odd one) and the pixels of an encoded row sit wherever the
	//control bytes leave them, so the other two ways are there for the rows that are not placed
	//as well
	if ((((uintptr_t)src | (uintptr_t)dst) & 3) == 0)
	{
		const uint32_t* s = (const uint32_t*)src;
		uint32_t* d = (uint32_t*)dst;
		const int16_t pairs = count >> 1;
		for (int16_t i = 0; i < pairs; i++)
			d[i] = s[i];
		//an odd last pixel of the run
		if (count & 1)
			dst[count - 1] = ((const uint16_t*)src)[count - 1];
	}
	else if (((uintptr_t)src & 1) == 0)
	{
		const uint16_t* s = (const uint16_t*)src;
		for (int16_t i = 0; i < count; i++)
			dst[i] = s[i];
	}
	else
	{
		for (int16_t i = 0; i < count; i++, src += 2)
			dst[i] = (uint16_t)(src[0] | (src[1] << 8));
	}
#else
	PLATFORM_READ_BYTES((uint8_t*)dst, src, count * sizeof(uint16_t));
#endif
}

//the same, but leaving the pixels of the image that carry the transparent key
static inline void BandCopyKeyed(uint16_t* dst, const uint8_t* src, int16_t count)
{
#if PLATFORM_DIRECT_FLASH
	//two pixels at a time where the addresses allow it: a pair that holds no transparent pixel
	//is one load and one store, which is most of a sprite
	if ((((uintptr_t)src | (uintptr_t)dst) & 3) == 0)
	{
		const uint32_t* s = (const uint32_t*)src;
		uint32_t* d = (uint32_t*)dst;
		const int16_t pairs = count >> 1;
		for (int16_t i = 0; i < pairs; i++)
		{
			const uint32_t two = s[i];
			//magenta is the transparent key, 0xF81F in RGB565
			const uint16_t low = (uint16_t)two, high = (uint16_t)(two >> 16);
			if ((low != 0xF81F) && (high != 0xF81F))
				d[i] = two;
			else
			{
				if (low != 0xF81F)
					dst[i * 2] = low;
				if (high != 0xF81F)
					dst[i * 2 + 1] = high;
			}
		}
		if (count & 1)
		{
			const uint16_t last = ((const uint16_t*)src)[count - 1];
			if (last != 0xF81F)
				dst[count - 1] = last;
		}
		return;
	}
	if (((uintptr_t)src & 1) == 0)
	{
		const uint16_t* s = (const uint16_t*)src;
		for (int16_t i = 0; i < count; i++)
			if (s[i] != 0xF81F)
				dst[i] = s[i];
		return;
	}
	for (int16_t i = 0; i < count; i++, src += 2)
	{
		const uint16_t col = (uint16_t)(src[0] | (src[1] << 8));
		if (col != 0xF81F)
			dst[i] = col;
	}
#else
	//reading this a pixel at a time would be a flash read each, the row is copied first
	uint16_t row[TileWidth];
	PLATFORM_READ_BYTES((uint8_t*)row, src, count * sizeof(uint16_t));
	for (int16_t i = 0; i < count; i++)
		if (row[i] != 0xF81F)
			dst[i] = row[i];
#endif
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

//Screen columns of the strip that a floor tile will cover completely, per row of the
//strip. The floor is painted opaque over the background, so those pixels are left out of
//the background below. covX1 <= covX0 means nothing is covered
static int16_t covX0[BANDHEIGHT], covX1[BANDHEIGHT];

static void BandClearCovered()
{
	for (int16_t r = 0; r < BANDHEIGHT; r++)
		covX0[r] = covX1[r] = 0;
}

#if FLOODFILLFLOOR
//the longest unbroken run of covered tiles across the strip, worked out per row because a
//row of tiles only lines up with the strip while the viewport is not scrolled between two
//tile rows. Only a run is looked for: the floor of a level is one area, and this has to
//stay cheap next to the pixels it saves
static void BandFindCovered(int16_t msx, int16_t msy)
{
	BandClearCovered();
	if (!Flood)
		return;
	const int16_t tx0 = (bandX0 + msx) / TileWidth;
	const int16_t tx1 = (bandX0 + bandW - 1 + msx) / TileWidth;
	int16_t lastTy = -1;
	for (int16_t r = 0; r < BANDHEIGHT; r++)
	{
		//the tile row this screen row falls in, the tile above covers the rows before it
		const int16_t ty = (bandY0 + r + msy) / TileHeight;
		if (ty == lastTy)
		{
			//same tiles as the row above, so the same run
			covX0[r] = covX0[r - 1];
			covX1[r] = covX1[r - 1];
			continue;
		}
		lastTy = ty;
		if ((ty < 0) || (ty >= NrOfRows))
			continue;
		int16_t bestStart = 0, bestLen = 0, start = 0, len = 0;
		for (int16_t tx = tx0; tx <= tx1; tx++)
		{
			if ((tx >= 0) && (tx < NrOfCols) && BitGet(Flood->coverHere, TILEBIT(tx, ty)))
			{
				if (len == 0)
					start = tx;
				len++;
				if (len > bestLen)
				{
					bestLen = len;
					bestStart = start;
				}
			}
			else
				len = 0;
		}
		if (bestLen == 0)
			continue;
		//the tiles in screen pixels, clipped to the strip
		int16_t x0 = bestStart * TileWidth - msx;
		int16_t x1 = (bestStart + bestLen) * TileWidth - msx;
		if (x0 < bandX0)
			x0 = bandX0;
		if (x1 > bandX0 + bandW)
			x1 = bandX0 + bandW;
		if (x1 > x0)
		{
			covX0[r] = x0;
			covX1[r] = x1;
		}
	}
}
#endif

static void BandBufCreate()
{
	if (bandBuf)
		return;
	bandBuf = (uint16_t*)malloc(WINDOW_WIDTH * BANDHEIGHT * sizeof(uint16_t));
	if (!bandBuf)
		Platform_Log("BandBufCreate: out of heap, %" PRIu32 " free\n", Platform_FreeHeap());
}

static void BandBufDestroy()
{
	free(bandBuf);
	bandBuf = NULL;
	//the next board has to paint everything again
	bgIndexed = NULL;
	lastMinScreenX = -30000;
	lastMinScreenY = -30000;
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
		int16_t pos = bandX0;                 //screen column the next pixels go to
		uint16_t* drow = &bandBuf[r * bandW];
		//the floor paints over these, decoding them would be thrown away
		const int16_t hide0 = covX0[r], hide1 = covX1[r];
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
				//this control covers [pos, end), the floor hides [hide0, hide1) of the row:
				//what is left is the piece before the hidden run and the piece after it
				const int16_t end = pos + avail;
				int16_t spans[2][2];
				int16_t parts = 0;
				if (hide1 <= hide0)
				{
					//nothing hidden on this row
					spans[0][0] = pos;
					spans[0][1] = end;
					parts = 1;
				}
				else
				{
					if (pos < hide0)
					{
						spans[parts][0] = pos;
						spans[parts][1] = (end < hide0) ? end : hide0;
						parts++;
					}
					if (end > hide1)
					{
						spans[parts][0] = (pos > hide1) ? pos : hide1;
						spans[parts][1] = end;
						parts++;
					}
				}
				const uint16_t col = run ? ReadPixel(data + 1) : 0;
				for (int16_t part = 0; part < parts; part++)
				{
					const int16_t s = spans[part][0];
					const int16_t n = spans[part][1] - s;
					uint16_t* d = drow + (s - pos);
					if (run)
					{
						for (int16_t i = 0; i < n; i++)
							d[i] = col;
					}
					else
						BandCopy(d, data + 1 + (used + (s - pos)) * 2, n);
				}
				drow += avail;
				pos = end;
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

	//clipped once here instead of per pixel: the rows and the columns of the sprite that
	//land in the strip
	const int16_t r0 = (sy < bandY0) ? bandY0 - sy : 0;
	const int16_t r1 = (sy + TileHeight > bandY0 + BANDHEIGHT) ? bandY0 + BANDHEIGHT - sy : TileHeight;
	const int16_t c0 = (sx < bandX0) ? bandX0 - sx : 0;
	const int16_t c1 = (sx + TileWidth > bandX0 + bandW) ? bandX0 + bandW - sx : TileWidth;
	const int16_t cols = c1 - c0;
	//little endian RGB565 like the strip, so a visible row is copied as it is
	for (int16_t r = r0; r < r1; r++)
	{
		uint16_t* drow = &bandBuf[(sy + r - bandY0) * bandW + (sx + c0 - bandX0)];
		const uint8_t* src = image + (r * TileWidth + c0) * sizeof(uint16_t);
		if (keyed)
			BandCopyKeyed(drow, src, cols);
		else
			BandCopy(drow, src, cols);
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
	//the strip it is composed in could not be allocated
	if (!bandBuf)
		return painted;
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
		//one window for the row, the strip covers all of it
		SCREEN.setAddrWindow(bandX0, cy * TileHeight, bandW, TileHeight);

		bandY0 = cy * TileHeight;

		//where the opaque sprites will paint over the background, so it is not decoded there
#if FLOODFILLFLOOR
		if (hasFloor)
			BandFindCovered(msx, msy);
		else
			BandClearCovered();
#else
		BandClearCovered();
#endif

		BandBackground();

#if FLOODFILLFLOOR
		//floor, only the playfield tiles that reach into this strip
		int16_t wx0 = bandX0 + msx;
		int16_t wx1 = bandX0 + bandW - 1 + msx;
		int16_t wy = bandY0 + msy;
		for (int16_t ty = wy / TileHeight; ty <= (wy + BANDHEIGHT - 1) / TileHeight; ty++)
			for (int16_t tx = wx0 / TileWidth; tx <= wx1 / TileWidth; tx++)
				if (hasFloor && (tx >= 0) && (tx < NrOfCols) && (ty >= 0) && (ty < NrOfRows) && BitGet(Flood->floorHere, TILEBIT(tx, ty)))
					BandSprite(tx * TileWidth - msx, ty * TileHeight - msy, IMGFloor, !IMGFloorOpaque);
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
			//a box or a wall has no transparent pixel, its rows go in as one copy
			BandSprite(Part->X - msx, sy, CWorldPart_SpriteData(Part), !PartOpaque(Part->Type));
		}

		//moving parts go on top, as they did before
		for (Teller = 0; Teller < WorldParts->MoveAbleItemCount; Teller++)
		{
			CWorldPart* Part = WorldParts->MoveAbleItems[Teller];
			if (Part->BHide)
				continue;
			BandSprite(Part->X - msx, Part->Y - msy, CWorldPart_SpriteData(Part), !PartOpaque(Part->Type));
		}

#if LOVYANGFX
		//true: bandBuf holds plain RGB565, the library puts it in display order
		SCREEN.writePixels((const uint16_t*)bandBuf, bandW * BANDHEIGHT, true);
#else
		SCREEN.pushPixels(bandBuf, bandW * BANDHEIGHT);
#endif
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
#if SCREENBUFFER == 0
	BandBufDestroy();
#endif
}
