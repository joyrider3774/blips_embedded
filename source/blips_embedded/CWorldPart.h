#ifndef CWORLDPART_H
#define CWORLDPART_H

#include <stdint.h>
#include "Common.h"
#include "CWorldParts.h"
#include "SPoint.h"

//playfield positions are stored as int8_t (signed, neighbour checks go to -1)
static_assert((NrOfCols <= 127) && (NrOfRows <= 127), "playfield size does not fit in int8_t");
//pixel positions are stored as int16_t
static_assert((NrOfCols * TileWidth <= 32767) && (NrOfRows * TileHeight <= 32767), "pixel positions do not fit in int16_t");
//a speed is negated into the int8_t Xi / Yi, the viewport is moved by the same int8_t steps
static_assert((GameMoveSpeed <= 127) && (ViewportMove <= 127) && (PlayerAnimDelay <= 255) && (ExplosionAnimDelay <= 255), "move speed, viewport step or anim delay does not fit its type");

//LastAnimPhase of a part that has never been painted, no real AnimPhase gets this high
#define AnimPhaseNeverDrawn 0xFF

typedef struct CWorldParts CWorldParts;
typedef struct CWorldPart CWolrdPart;
//kept as small as possible, a level needs up to MAXWORLDPARTS of these and the
//ESP8266 has to fit them next to the 32k framebuffer. X and Y are pixel positions,
//everything else is a tile coordinate, an id or a small counter. Grouped by size
//so there is no padding.
//there is only one parts list (the global WorldParts) so no parent pointer is kept
struct CWorldPart
{
	//pixel positions (PlayFieldX * TileWidth, PlayFieldY * TileHeight)
	int16_t X,Y;
	//playfield positions (0 .. NrOfCols-1 / NrOfRows-1, neighbour checks pass -1)
	int8_t PlayFieldX,PlayFieldY;
	//+- MoveSpeed while moving
	int8_t Xi,Yi;
	//-1 .. MoveDelay, it is set to -1 and counted back up right away
	int8_t MoveDelayCounter;
	//IDxxx (0 .. IDExplosion, 0 = free pool slot)
	uint8_t Type;
	//0 or GameMoveSpeed, MoveDelay is always 0
	uint8_t MoveSpeed,MoveDelay;
	//Zxxx (0 .. ZExplosion), Group is always 0
	uint8_t Z,Group;
	//anim phases max AnimBase 12 + AnimPhases 4 - 1 (15), explosions go up to 7
	uint8_t AnimBase,AnimCounter,AnimDelay,AnimDelayCounter,AnimPhase,AnimPhases;
	uint8_t LastAnimPhase;          //AnimPhase as it was last painted, AnimPhaseNeverDrawn = never
	bool PNeedToKill;
	bool BHide;
	bool IsMoving;
	bool IsDeath;
};

CWorldPart* CWorldPart_Create(const int8_t PlayFieldXin,const int8_t PlayFieldYin, const uint8_t TypeId);
void CWorldPart_Hide(CWorldPart* WorldPart);

void CWorldPart_Kill(CWorldPart* WorldPart);
bool CWorldPart_NeedToKill(CWorldPart* WorldPart);
uint8_t CWorldPart_GetGroup(CWorldPart* WorldPart);
uint8_t CWorldPart_GetType(CWorldPart* WorldPart);
int16_t CWorldPart_GetX(CWorldPart* WorldPart);
int16_t CWorldPart_GetY(CWorldPart* WorldPart);
int8_t CWorldPart_GetPlayFieldX(CWorldPart* WorldPart);
int8_t CWorldPart_GetPlayFieldY(CWorldPart* WorldPart);
uint8_t CWorldPart_GetZ(CWorldPart* WorldPart);
uint8_t CWorldPart_GetAnimPhase(CWorldPart* WorldPart);
void CWorldPart_SetAnimPhase(CWorldPart* WorldPart, uint8_t AnimPhaseIn);
void CWorldPart_SetPosition(CWorldPart* WorldPart, const int8_t PlayFieldXin,const int8_t PlayFieldYin);


void CWorldPart_Event_ArrivedOnNewSpot(CWorldPart* WorldPart);
void CWorldPart_Event_BeforeDraw(CWorldPart* WorldPart);
void CWorldPart_Event_LeaveCurrentSpot(CWorldPart* WorldPart);
void CWorldPart_Event_Moving(CWorldPart* WorldPart, int16_t ScreenPosX,int16_t ScreenPosY,int8_t ScreenXi, int8_t ScreenYi);
void CWorldPart_MoveTo(CWorldPart* WorldPart, const int8_t PlayFieldXin,const int8_t PlayFieldYin,bool BackWards);
bool CWorldPart_CanMoveTo(CWorldPart* WorldPart, const int8_t PlayFieldXin,const int8_t PlayFieldYin);
void CWorldPart_Move(CWorldPart* WorldPart);
void CWorldPart_Draw(CWorldPart* WorldPart);
const uint8_t* CWorldPart_SpriteData(CWorldPart* WorldPart);

void CWorldPart_Destroy(CWorldPart* WorldPart);


#endif