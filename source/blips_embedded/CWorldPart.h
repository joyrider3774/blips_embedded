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

//LastAnimPhase of a part that has never been painted, no real AnimPhase gets this high: the
//largest a part ever shows is AnimBase 4 plus AnimPhases 8 less one, which is 11. It is the
//widest value the field holds, see CWorldPart below
#define AnimPhaseNeverDrawn 31

typedef struct CWorldParts CWorldParts;
typedef struct CWorldPart CWolrdPart;
//kept as small as possible, a level needs up to MAXWORLDPARTS of these and the
//ESP8266 has to fit them next to the 32k framebuffer. X and Y are pixel positions,
//everything else is a tile coordinate, an id or a small counter. Grouped by size
//so there is no padding.
//there is only one parts list (the global WorldParts) so no parent pointer is kept
struct CWorldPart
{
	//Everything below the two pixel positions is a small counter, a tile coordinate or an id,
	//and each is given the bits its range needs instead of a byte of its own. The pool is
	//MAXWORLDPARTS of these and is by far the largest thing the game asks the heap for, so what
	//one part costs is multiplied by several hundred: this takes the part from 22 bytes to 14
	//and the pool on a CHGame from 7964 to 5068, which is the room its art cache comes out of.
	//They are read and written exactly as before, so nothing that uses them changes.
	//The range of each is in the comment beside it, and a value that outgrows it would be cut
	//silently, so a field is widened here before it is given a larger value anywhere

	//pixel positions (PlayFieldX * TileWidth, PlayFieldY * TileHeight), up to NrOfCols tiles
	int16_t X,Y;
	//playfield positions (0 .. NrOfCols-1 / NrOfRows-1, neighbour checks pass -1)
	int8_t PlayFieldX,PlayFieldY;
	//+- MoveSpeed while moving, and MoveSpeed is GameMoveSpeed (4) at most
	int8_t Xi : 4;
	int8_t Yi : 4;
	//IDxxx (0 .. IDExplosion = 14, 0 = free pool slot)
	uint8_t Type : 4;
	//Zxxx (0 .. ZExplosion = 15). Group went: it was set to 0 and never read
	uint8_t Z : 4;
	//0 or GameMoveSpeed (2 or 4, see IMAGESET)
	uint8_t MoveSpeed : 3;
	//always 0, and kept because the move delay is counted against it
	uint8_t MoveDelay : 2;
	//-1 .. MoveDelay, it is set to -1 and counted back up right away
	int8_t MoveDelayCounter : 2;
	//0, 4, 8 or 12: the first frame of the run down the sheet for the way the part is facing.
	//Three bits held 0 and 4 and turned 8 and 12 into them, so up and down ran the left and
	//right animations
	uint8_t AnimBase : 4;
	//AnimBase .. AnimBase + AnimPhases - 1, so 15 at most
	uint8_t AnimPhase : 4;
	//1, 4 or 8
	uint8_t AnimPhases : 4;
	//0 .. AnimPhases - 1, and counted up until it equals AnimPhases, so it holds that too
	uint8_t AnimCounter : 4;
	//PlayerAnimDelay (3) or ExplosionAnimDelay (1)
	uint8_t AnimDelay : 3;
	//0 .. AnimDelay - 1
	uint8_t AnimDelayCounter : 3;
	//AnimPhase as it was last painted, AnimPhaseNeverDrawn (31) = never
	uint8_t LastAnimPhase : 5;
	uint8_t PNeedToKill : 1;
	uint8_t BHide : 1;
	uint8_t IsMoving : 1;
	uint8_t IsDeath : 1;
};

//What the fields above are sized for. A device header that raises any of these has to widen
//the field with it, which is why they are checked here rather than left to be noticed
static_assert(IDExplosion <= 15, "Type does not fit its four bits");
static_assert(ZExplosion <= 15, "Z does not fit its four bits");
static_assert(GameMoveSpeed <= 7, "MoveSpeed does not fit its three bits");
static_assert(PlayerAnimDelay <= 7, "AnimDelay does not fit its three bits");
static_assert(ExplosionAnimDelay <= 7, "AnimDelay does not fit its three bits");
//AnimBase is a quarter of the sheet per direction and AnimPhase is AnimBase plus AnimCounter,
//so both are checked against the tallest sheet the game draws from
static_assert(12 + 4 - 1 <= 15, "AnimBase plus AnimCounter does not fit AnimPhase's four bits");
static_assert(NrOfCols * TileWidth <= 32767, "X does not fit int16_t");
static_assert(NrOfRows * TileHeight <= 32767, "Y does not fit int16_t");
static_assert(sizeof(struct CWorldPart) <= 14, "the part grew, the pool is MAXWORLDPARTS of it");

//asks for the pool of parts up front, see CWorldParts_Create
void CWorldPart_PoolCreate();
CWorldPart* CWorldPart_Create(const int8_t PlayFieldXin,const int8_t PlayFieldYin, const uint8_t TypeId);
void CWorldPart_Hide(CWorldPart* WorldPart);

void CWorldPart_Kill(CWorldPart* WorldPart);
bool CWorldPart_NeedToKill(CWorldPart* WorldPart);
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
//which frame of its sheet the part shows, see CWorldPart_SpriteData
uint8_t CWorldPart_SpriteFrame(CWorldPart* WorldPart);

void CWorldPart_Destroy(CWorldPart* WorldPart);


#endif