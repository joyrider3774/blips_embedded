#ifndef CWORLDPARTS_H
#define CWORLDPARTS_H

#include <stdint.h>
#include "Defines.h"
#include "CViewPort.h"
#include "CLevelPackFile.h"

typedef struct CWorldPart CWorldPart;
typedef struct CWorldParts CWorldParts;

//ItemCount goes up to MAXWORLDPARTS, MoveAbleItemCount up to MAXMOVEABLEWORLDPARTS
static_assert(MAXWORLDPARTS <= 65535, "ItemCount does not fit in uint16_t");
static_assert(MAXMOVEABLEWORLDPARTS <= 255, "MoveAbleItemCount does not fit in uint8_t");
static_assert((IDPlayer <= 127) && (IDPlayer2 <= 127), "ActivePlayer does not fit in int8_t");

struct CWorldParts
{
		CViewPort *ViewPort;
		CWorldPart *Items[MAXWORLDPARTS];
		CWorldPart* MoveAbleItems[MAXMOVEABLEWORLDPARTS];
		CWorldPart *Player,*Player1,*Player2;
		//0 .. MAXWORLDPARTS
		uint16_t ItemCount;
		//IDPlayer, IDPlayer2 or -1 = no player
		int8_t ActivePlayer;
		//0 .. MAXMOVEABLEWORLDPARTS
		uint8_t MoveAbleItemCount;
		//counts down from 15 after switching players
		uint8_t ActivePlayerFlicker;
		bool DisableSorting;
		bool isLevelPackFileLevel;
};

CWorldParts* CWorldParts_Create();
void CWorldParts_Sort(CWorldParts* WorldParts);
void CWorldParts_SwitchPlayers(CWorldParts* WorldParts);
void CWorldParts_Add(CWorldParts* WorldParts, CWorldPart *WorldPart);
bool CWorldParts_CenterLevel(CWorldParts* WorldParts);
void CWorldParts_CenterVPOnPlayer(CWorldParts* WorldParts);
void CWorldParts_LimitVPLevel(CWorldParts* WorldParts);
void CWorldParts_Move(CWorldParts* WorldParts);
void CWorldParts_Draw(CWorldParts* WorldParts);
//repaint only the screen cells whose contents changed
bool CWorldParts_DrawBoard(CWorldParts* WorldParts);
void CWorldParts_MarkDirty(int16_t x, int16_t y, int16_t w, int16_t h);
void CWorldParts_MarkAllDirty();
void CWorldParts_Remove(CWorldParts* WorldParts, int8_t PlayFieldXin,int8_t PlayFieldYin);
void CWorldParts_Remove_Type(CWorldParts* WorldParts, int8_t PlayFieldXin,int8_t PlayFieldYin,uint8_t Type);
void CWorldParts_RemoveAll(CWorldParts* WorldParts);
void CWorldParts_Save_vircon(CWorldParts* WorldParts, char* Filename);
void CWorldParts_Save(CWorldParts* WorldParts, char *Filename);
bool CWorldParts_LoadFromLevelPackFile(CWorldParts* WorldParts, CLevelPackFile* LPFile, int8_t level, bool doCenterLevel);
void CWorldParts_Load(CWorldParts* WorldParts, char *Filename);
#if FLOODFILLFLOOR
bool CWorldParts_DrawFloor(CWorldParts* WorldParts, CWorldPart* Player, CWorldPart* Player2);
#endif
void CWorldParts_Destroy(CWorldParts* WorldParts);
#endif