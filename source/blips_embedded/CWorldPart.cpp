#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <inttypes.h>
#include "Sound.h"
#include "Common.h"
#include "GameFuncs.h"
#include "CWorldPart.h"
#include "SPoint.h"
#include "string.h"


//parts come out of one fixed pool instead of malloc, that saves the 8 bytes of heap
//overhead every single part would otherwise cost and it can't fragment the heap.
//a free slot is marked by Type 0, no part ever has that id.
//the pool is only taken from the heap when the first part is created, so it costs
//nothing until a level is actually loaded, and it is kept from then on
static CWorldPart* WorldPartPool = NULL;
//Index below reaches 2 * MAXWORLDPARTS - 2 before it is wrapped
static_assert(MAXWORLDPARTS * 2 <= 65535, "pool indexes do not fit in uint16_t");
static uint16_t WorldPartPoolNext = 0;

static CWorldPart* CWorldPart_PoolAlloc()
{
	if (!WorldPartPool)
	{
		//calloc zeroes the slots, so every one of them starts out free (Type 0)
		WorldPartPool = (CWorldPart*)calloc(MAXWORLDPARTS, sizeof(CWorldPart));
		if (!WorldPartPool)
		{
			Platform_Log("CWorldPart_PoolAlloc: out of heap, %" PRIu32 " free\n", Platform_FreeHeap());
			return NULL;
		}
		WorldPartPoolNext = 0;
	}

	for (uint16_t Teller = 0; Teller < MAXWORLDPARTS; Teller++)
	{
		//start looking where the last one was taken, allocating a whole level
		//stays linear that way instead of rescanning the pool for every part
		uint16_t Index = WorldPartPoolNext + Teller;
		if (Index >= MAXWORLDPARTS)
			Index -= MAXWORLDPARTS;
		if (WorldPartPool[Index].Type == 0)
		{
			WorldPartPoolNext = Index + 1;
			if (WorldPartPoolNext >= MAXWORLDPARTS)
				WorldPartPoolNext = 0;
			return &WorldPartPool[Index];
		}
	}
	return NULL;
}

CWorldPart* CWorldPart_Create(const int8_t PlayFieldXin,const int8_t PlayFieldYin, const uint8_t TypeId)
{
	CWorldPart* Result = CWorldPart_PoolAlloc();
	if (Result)
	{
		Result->PlayFieldX = PlayFieldXin;
		Result->PlayFieldY = PlayFieldYin;
		Result->Xi = 0;
		Result->Yi = 0;
		Result->X = PlayFieldXin * TileWidth;
		Result->Y = PlayFieldYin * TileHeight;
		Result->Type = TypeId;
		Result->MoveDelay = 0;
		Result->MoveDelayCounter = 0;
		Result->IsMoving = false;
		Result->MoveSpeed = 0;
		Result->AnimPhase = 0;
		//different from every real AnimPhase, so a new part is always painted once
		Result->LastAnimPhase = AnimPhaseNeverDrawn;
		Result->Z = 0;
		Result->Group = 0;
		Result->PNeedToKill = false;
		Result->BHide = false;
		Result->AnimBase = 0;
		Result->IsDeath = false;
		switch (TypeId)
		{
			case IDPlayer:
				Result->AnimBase = 4;
				Result->AnimPhase = 4;
				Result->AnimPhases = 4;
				Result->AnimCounter = 0;
				Result->AnimDelay = PlayerAnimDelay;
				Result->MoveDelay = 0;
				Result->MoveSpeed = GameMoveSpeed;
				Result->AnimDelayCounter = 0;
				Result->Z = ZPlayer;
				break;
			case IDPlayer2:
				Result->AnimBase = 4;
				Result->AnimPhase = 4;
				Result->AnimPhases = 4;
				Result->AnimCounter = 0;
				Result->AnimDelay = PlayerAnimDelay;
				Result->MoveDelay = 0;
				Result->MoveSpeed = GameMoveSpeed;
				Result->AnimDelayCounter = 0;
				Result->Z = ZPlayer;
				break;
			case IDBox:
				Result->AnimPhase = 0;
				Result->MoveDelay = 0;
				Result->MoveSpeed = GameMoveSpeed;
				Result->AnimPhases = 1;
				Result->Z = ZBox;
				break;
			case IDBox1:
				Result->AnimPhase = 2;
				Result->MoveDelay = 0;
				Result->MoveSpeed = GameMoveSpeed;
				Result->AnimPhases = 1;
				Result->Z = ZBox;
				break;
			case IDBox2:
				Result->AnimPhase = 3;
				Result->AnimPhases = 1;
				Result->MoveDelay = 0;
				Result->MoveSpeed = GameMoveSpeed;
				Result->AnimPhases = 1;
				Result->Z = ZBox;
				break;
			case IDBoxWall:
				Result->AnimPhase = 1;
				Result->AnimPhases = 1;
				Result->MoveDelay = 0;
				Result->MoveSpeed = GameMoveSpeed;
				Result->AnimPhases = 1;
				Result->Z = ZBox;
				break;
			case IDBoxBomb:
				Result->AnimPhase = 4;
				Result->AnimPhases = 1;
				Result->MoveDelay = 0;
				Result->MoveSpeed = GameMoveSpeed;
				Result->AnimPhases = 1;
				Result->Z = ZBox;
				break;
			case IDDiamond:
				Result->Z = ZDiamond;
				break;
			case IDBomb:
				Result->Z = ZBomb;
				break;
			case IDFloor:
				Result->Z = ZFloor;
				break;
			case IDWall:
				Result->Z = ZWall;
				Result->AnimPhase = 0;
				break;
			case IDWallBreakable:
				Result->Z = ZWall;
				Result->AnimPhase = 1;
				break;
			case IDEmpty:
				break;
			case IDExplosion:
				playGameExplodeSound();
				Result->Z = ZExplosion;
				Result->AnimPhase = 0;
				Result->AnimPhases = 8;
				Result->AnimCounter = 1;
				Result->AnimDelay = ExplosionAnimDelay;
				Result->MoveDelay = 0;
				Result->MoveSpeed = GameMoveSpeed;
				Result->AnimDelayCounter = 0;
				break;
		}
	}
	return(Result);
}


void CWorldPart_Hide(CWorldPart* WorldPart) 
{
	WorldPart->BHide = true;
}



void CWorldPart_Kill(CWorldPart* WorldPart)
{
	WorldPart->PNeedToKill = true;
}

bool CWorldPart_NeedToKill(CWorldPart* WorldPart) 
{
	return WorldPart->PNeedToKill;
}


uint8_t CWorldPart_GetGroup(CWorldPart* WorldPart) 
{
	return WorldPart->Group;
}

uint8_t CWorldPart_GetType(CWorldPart* WorldPart) 
{
	return WorldPart->Type;
}

int16_t CWorldPart_GetX(CWorldPart* WorldPart) 
{
	return WorldPart->X;
}

int16_t CWorldPart_GetY(CWorldPart* WorldPart) 
{
	return WorldPart->Y;
}

int8_t CWorldPart_GetPlayFieldX(CWorldPart* WorldPart) 
{
	return WorldPart->PlayFieldX;
}

int8_t CWorldPart_GetPlayFieldY(CWorldPart* WorldPart) 
{
	return WorldPart->PlayFieldY;
}

uint8_t CWorldPart_GetZ(CWorldPart* WorldPart) 
{
	return WorldPart->Z;
}

uint8_t CWorldPart_GetAnimPhase(CWorldPart* WorldPart) 
{
	return WorldPart->AnimPhase;
}

void CWorldPart_SetAnimPhase(CWorldPart* WorldPart, uint8_t AnimPhaseIn) 
{ 
	WorldPart->AnimPhase = AnimPhaseIn;
}

void CWorldPart_Event_ArrivedOnNewSpot(CWorldPart* WorldPart)
{
	uint16_t Teller=0;
    if(WorldPart->Type == IDPlayer || WorldPart->Type == IDPlayer2)
	{
		if (WorldParts)
		{
			for (Teller=0;Teller<WorldParts->ItemCount;Teller++)
			{
				if (WorldParts->Items[Teller]->PNeedToKill || WorldParts->Items[Teller]->BHide)
					continue;

				if((WorldParts->Items[Teller]->PlayFieldX == WorldPart->PlayFieldX) && (WorldParts->Items[Teller]->PlayFieldY == WorldPart->PlayFieldY))
				{
					if(WorldParts->Items[Teller]->Type == IDBomb)
					{
						//kill needs to come first, Add sorts the lists skewing indexing
						CWorldPart_Kill(WorldParts->Items[Teller]);
						WorldPart->IsDeath = true;
						CWorldPart_Hide(WorldPart);
						CWorldParts_Add(WorldParts, CWorldPart_Create(WorldPart->PlayFieldX, WorldPart->PlayFieldY, IDExplosion));
						break;
					}

					if(WorldParts->Items[Teller]->Type == IDDiamond)
					{
						CWorldPart_Kill(WorldParts->Items[Teller]);
						playGameCollectSound();
					}

				}
			}
		}
	}

	if (WorldPart->Type == IDBox || WorldPart->Type == IDBox1 || WorldPart->Type == IDBox2 || WorldPart->Type == IDBoxBomb || WorldPart->Type == IDBoxWall)
	{
		if (WorldPart->Type == IDBox)
			WorldPart->AnimPhase = 0;
		else if (WorldPart->Type == IDBox1)
			WorldPart->AnimPhase = 2;
		else if (WorldPart->Type == IDBox2)
			WorldPart->AnimPhase = 3;
		else if (WorldPart->Type == IDBoxWall)
			WorldPart->AnimPhase = 1;
		else if (WorldPart->Type == IDBoxBomb)
			WorldPart->AnimPhase = 4;

		if (WorldParts)
		{
			for (Teller = 0; Teller < WorldParts->ItemCount; Teller++)
			{
				if (WorldParts->Items[Teller]->PNeedToKill || WorldParts->Items[Teller]->BHide)
					continue;

				if ((WorldParts->Items[Teller]->PlayFieldX == WorldPart->PlayFieldX) && (WorldParts->Items[Teller]->PlayFieldY == WorldPart->PlayFieldY) &&
					(WorldParts->Items[Teller]->Type == IDBomb))
				{
					//kill needs to come first, Add sorts the lists skewing indexing
					CWorldPart_Kill(WorldParts->Items[Teller]);
					CWorldPart_Kill(WorldPart);
					CWorldParts_Add(WorldParts, CWorldPart_Create(WorldPart->PlayFieldX, WorldPart->PlayFieldY, IDExplosion));					
					break;
				}

				if ((WorldParts->Items[Teller]->PlayFieldX == WorldPart->PlayFieldX) && (WorldParts->Items[Teller]->PlayFieldY == WorldPart->PlayFieldY) &&
					(WorldParts->Items[Teller]->Type == IDBoxWall) && (WorldPart->Type == IDBoxWall) &&
					(WorldParts->Items[Teller] != WorldPart))
				{
					//kill needs to come first, Add sorts the lists skewing indexing
					CWorldPart_Kill(WorldParts->Items[Teller]);
					CWorldPart_Kill(WorldPart);
					CWorldParts_Add(WorldParts, CWorldPart_Create(WorldPart->PlayFieldX, WorldPart->PlayFieldY, IDExplosion));
					CWorldParts_Add(WorldParts, CWorldPart_Create(WorldPart->PlayFieldX, WorldPart->PlayFieldY, IDWall));
					break;
				}

				if ((WorldParts->Items[Teller]->PlayFieldX == WorldPart->PlayFieldX) && (WorldParts->Items[Teller]->PlayFieldY == WorldPart->PlayFieldY) &&
					(WorldParts->Items[Teller]->Type != IDWall) && (WorldParts->Items[Teller]->Type != IDDiamond) && 
					(WorldParts->Items[Teller]->Type != IDFloor) &&
					(WorldPart->Type == IDBoxBomb) && (WorldPart != WorldParts->Items[Teller]))
				{
					//kill needs to come first, Add sorts the lists skewing indexing
					if ((WorldParts->Items[Teller]->Type == IDPlayer) || (WorldParts->Items[Teller]->Type == IDPlayer2))
					{
						WorldParts->Items[Teller]->IsDeath = true;
						CWorldPart_Hide(WorldParts->Items[Teller]);
					}
					else
						CWorldPart_Kill(WorldParts->Items[Teller]);
					CWorldPart_Kill(WorldPart);
					CWorldParts_Add(WorldParts, CWorldPart_Create(WorldPart->PlayFieldX, WorldPart->PlayFieldY, IDExplosion));										
					break;
				}

				if ((WorldParts->Items[Teller]->PlayFieldX == WorldPart->PlayFieldX) && (WorldParts->Items[Teller]->PlayFieldY == WorldPart->PlayFieldY) &&
					(WorldParts->Items[Teller]->Type == IDBoxBomb) && (WorldPart != WorldParts->Items[Teller]))
				{
					//kill needs to come first, Add sorts the lists skewing indexing
					CWorldPart_Kill(WorldParts->Items[Teller]);
					CWorldPart_Kill(WorldPart);
					CWorldParts_Add(WorldParts, CWorldPart_Create(WorldPart->PlayFieldX, WorldPart->PlayFieldY, IDExplosion));					
					break;
				}
			}
		}
	}	
}

void CWorldPart_Event_BeforeDraw(CWorldPart* WorldPart) 
{
	switch(WorldPart->Type)
	{
		case IDPlayer2:
		case IDPlayer:
		{
			if (WorldPart->IsMoving)
			{
				WorldPart->AnimPhase = WorldPart->AnimBase + WorldPart->AnimCounter;
				WorldPart->AnimDelayCounter++;
				if (WorldPart->AnimDelayCounter == WorldPart->AnimDelay)
				{
					WorldPart->AnimDelayCounter = 0;
					WorldPart->AnimCounter++;
					if (WorldPart->AnimCounter == WorldPart->AnimPhases)
						WorldPart->AnimCounter = 0;
				}
			}
			break;
		}

		case IDExplosion:
		{
			WorldPart->AnimDelayCounter++;
			if (WorldPart->AnimDelayCounter == WorldPart->AnimDelay)
			{
				WorldPart->AnimDelayCounter = 0;
				WorldPart->AnimPhase++;
				if (WorldPart->AnimPhase == WorldPart->AnimPhases-1)
					CWorldPart_Kill(WorldPart);
			}
			break;
		}

		default:
			break;
	}
}

void CWorldPart_Event_LeaveCurrentSpot(CWorldPart*) 
{

}

void CWorldPart_Event_Moving(CWorldPart* WorldPart, int16_t ScreenPosX,int16_t ScreenPosY,int8_t, int8_t) 
{
	if(WorldPart->Type == IDPlayer || WorldPart->Type == IDPlayer2)
	{
		if((ScreenPosX > (WorldParts->ViewPort->MaxScreenX) - HALFWINDOWWIDTH) && (WorldPart->Xi > 0))
			CViewPort_Move(WorldParts->ViewPort, WorldPart->Xi,WorldPart->Yi);
		if((ScreenPosX < (WorldParts->ViewPort->MaxScreenX) - HALFWINDOWWIDTH) && (WorldPart->Xi < 0))
			CViewPort_Move(WorldParts->ViewPort, WorldPart->Xi,WorldPart->Yi);
		if((ScreenPosY > (WorldParts->ViewPort->MaxScreenY) - HALFWINDOWHEIGHT) && (WorldPart->Yi > 0))
			CViewPort_Move(WorldParts->ViewPort, WorldPart->Xi,WorldPart->Yi);
		if((ScreenPosY < (WorldParts->ViewPort->MaxScreenY) - HALFWINDOWHEIGHT) && (WorldPart->Yi < 0))
			CViewPort_Move(WorldParts->ViewPort, WorldPart->Xi,WorldPart->Yi);
	}
}

void CWorldPart_SetPosition(CWorldPart* WorldPart, const int8_t PlayFieldXin,const int8_t PlayFieldYin)
{
	if ((PlayFieldXin >= 0) && (PlayFieldXin < NrOfCols) && (PlayFieldYin >= 0) && (PlayFieldYin < NrOfRows))
	{
		WorldPart->PlayFieldX=PlayFieldXin;
		WorldPart->PlayFieldY=PlayFieldYin;
		WorldPart->X=PlayFieldXin*TileWidth;
		WorldPart->Y=PlayFieldYin*TileHeight;
		//Event_ArrivedOnNewSpot();
	}
}

void CWorldPart_MoveTo(CWorldPart* WorldPart, const int8_t PlayFieldXin,const int8_t PlayFieldYin,bool BackWards)
{
	uint16_t Teller;
 	if(WorldPart->Type == IDPlayer)
	{
		if(!WorldPart->IsMoving && !WorldPart->BHide && !WorldPart->PNeedToKill)
		{
			if(CWorldPart_CanMoveTo(WorldPart, PlayFieldXin,PlayFieldYin) || BackWards)
			{
				WorldPart->PlayFieldX = PlayFieldXin;
				WorldPart->PlayFieldY = PlayFieldYin;
				if(WorldPart->X < WorldPart->PlayFieldX*TileWidth)
				{
					WorldPart->Xi = WorldPart->MoveSpeed;
					if (WorldParts)
					{
						for(Teller=0;Teller<WorldParts->ItemCount;Teller++)
						{
							if (WorldParts->Items[Teller]->PNeedToKill || WorldParts->Items[Teller]->BHide)
								continue;

							if((WorldParts->Items[Teller]->Type == IDBox || WorldParts->Items[Teller]->Type == IDBox1 || 
								WorldParts->Items[Teller]->Type == IDBoxWall || WorldParts->Items[Teller]->Type == IDBoxBomb) &&
								((WorldParts->Items[Teller]->PlayFieldX == WorldPart->PlayFieldX) && (WorldParts->Items[Teller]->PlayFieldY == WorldPart->PlayFieldY)))
							{
								CWorldPart_MoveTo(WorldParts->Items[Teller], WorldPart->PlayFieldX+1,WorldPart->PlayFieldY,false);
								break;
							}
						}
					}
					WorldPart->AnimBase = 4;

				}
				if(WorldPart->X > WorldPart->PlayFieldX*TileWidth)
				{
					WorldPart->Xi = -WorldPart->MoveSpeed;
					if (WorldParts)
					{
						for(Teller=0;Teller<WorldParts->ItemCount;Teller++)
						{
							if (WorldParts->Items[Teller]->PNeedToKill || WorldParts->Items[Teller]->BHide)
								continue;

							if((WorldParts->Items[Teller]->Type == IDBox || WorldParts->Items[Teller]->Type == IDBox1 ||
								WorldParts->Items[Teller]->Type == IDBoxWall || WorldParts->Items[Teller]->Type == IDBoxBomb) &&
							   ((WorldPart->PlayFieldX == WorldParts->Items[Teller]->PlayFieldX )  && 
							    (WorldParts->Items[Teller]->PlayFieldY == WorldPart->PlayFieldY)))
							{
								CWorldPart_MoveTo(WorldParts->Items[Teller], WorldPart->PlayFieldX-1,WorldPart->PlayFieldY,false);
								break;
							}
						}
					}
					WorldPart->AnimBase = 0;
				}
				if(WorldPart->Y > WorldPart->PlayFieldY*TileHeight)
				{
					WorldPart->Yi = -WorldPart->MoveSpeed;
					if (WorldParts)
					{
						for(Teller=0;Teller<WorldParts->ItemCount;Teller++)
						{
							if (WorldParts->Items[Teller]->PNeedToKill || WorldParts->Items[Teller]->BHide)
								continue;

							if((WorldParts->Items[Teller]->Type == IDBox || WorldParts->Items[Teller]->Type == IDBox1 ||
								WorldParts->Items[Teller]->Type == IDBoxWall || WorldParts->Items[Teller]->Type == IDBoxBomb) &&
							   ((WorldPart->PlayFieldY == WorldParts->Items[Teller]->PlayFieldY)  && 
							    (WorldParts->Items[Teller]->PlayFieldX == WorldPart->PlayFieldX)))
							{
								CWorldPart_MoveTo(WorldParts->Items[Teller], WorldPart->PlayFieldX,WorldPart->PlayFieldY-1,false);
								break;
							}
						}
					}
					WorldPart->AnimBase = 8;
				}
				if(WorldPart->Y < WorldPart->PlayFieldY*TileHeight)
				{
					WorldPart->Yi = WorldPart->MoveSpeed;
					if(WorldParts)
					{
						for(Teller=0;Teller<WorldParts->ItemCount;Teller++)
						{
							if (WorldParts->Items[Teller]->PNeedToKill || WorldParts->Items[Teller]->BHide)
								continue;

							if((WorldParts->Items[Teller]->Type == IDBox || WorldParts->Items[Teller]->Type == IDBox1 ||
								WorldParts->Items[Teller]->Type == IDBoxWall || WorldParts->Items[Teller]->Type == IDBoxBomb) &&
							   ((WorldParts->Items[Teller]->PlayFieldY == WorldPart->PlayFieldY)  && 
							    (WorldParts->Items[Teller]->PlayFieldX == WorldPart->PlayFieldX )))
							{
								CWorldPart_MoveTo(WorldParts->Items[Teller], WorldPart->PlayFieldX,WorldPart->PlayFieldY+1,false);
								break;
							}
						}
					}
					WorldPart->AnimBase = 12;
				}
				playGameMoveSound();
				WorldPart->IsMoving = true;
			}
			else
			{
				if (PlayFieldXin > WorldPart->PlayFieldX)
				{
					WorldPart->AnimBase= 4;
				}
				if (PlayFieldXin < WorldPart->PlayFieldX)
				{
					WorldPart->AnimBase = 0;
				}
				if (PlayFieldYin > WorldPart->PlayFieldY)
				{
					WorldPart->AnimBase = 12;
				}
				if (PlayFieldYin < WorldPart->PlayFieldY)
				{
					WorldPart->AnimBase = 8;
				}
				WorldPart->AnimPhase = WorldPart->AnimBase + WorldPart->AnimCounter;
				WorldPart->AnimDelayCounter++;
				if (WorldPart->AnimDelayCounter == WorldPart->AnimDelay)
				{
					WorldPart->AnimDelayCounter = 0;
					WorldPart->AnimCounter++;
					if (WorldPart->AnimCounter == WorldPart->AnimPhases)
						WorldPart->AnimCounter = 0;
				}
			}

		}
	}
	else if (WorldPart->Type == IDPlayer2)
	{
		if (!WorldPart->IsMoving && !WorldPart->PNeedToKill && !WorldPart->BHide)
		{
			if (CWorldPart_CanMoveTo(WorldPart, PlayFieldXin, PlayFieldYin) || BackWards)
			{
				WorldPart->PlayFieldX = PlayFieldXin;
				WorldPart->PlayFieldY = PlayFieldYin;
				if (WorldPart->X < WorldPart->PlayFieldX * TileWidth)
				{
					WorldPart->Xi = WorldPart->MoveSpeed;
					if (WorldParts)
					{
						for (Teller = 0; Teller < WorldParts->ItemCount; Teller++)
						{
							if (WorldParts->Items[Teller]->PNeedToKill || WorldParts->Items[Teller]->BHide)
								continue;

							if ((WorldParts->Items[Teller]->Type == IDBox || WorldParts->Items[Teller]->Type == IDBox2 || 
								WorldParts->Items[Teller]->Type == IDBoxWall || WorldParts->Items[Teller]->Type == IDBoxBomb) &&
								((WorldParts->Items[Teller]->PlayFieldX == WorldPart->PlayFieldX) && (WorldParts->Items[Teller]->PlayFieldY == WorldPart->PlayFieldY)))
							{
								CWorldPart_MoveTo(WorldParts->Items[Teller], WorldPart->PlayFieldX + 1, WorldPart->PlayFieldY, false);
								break;
							}
						}
					}
					WorldPart->AnimBase = 4;

				}
				if (WorldPart->X > WorldPart->PlayFieldX * TileWidth)
				{
					WorldPart->Xi = -WorldPart->MoveSpeed;
					if (WorldParts)
					{
						for (Teller = 0; Teller < WorldParts->ItemCount; Teller++)
						{
							if (WorldParts->Items[Teller]->PNeedToKill || WorldParts->Items[Teller]->BHide)
								continue;

							if ((WorldParts->Items[Teller]->Type == IDBox || WorldParts->Items[Teller]->Type == IDBox2 ||
								WorldParts->Items[Teller]->Type == IDBoxWall || WorldParts->Items[Teller]->Type == IDBoxBomb) &&
								((WorldPart->PlayFieldX == WorldParts->Items[Teller]->PlayFieldX) &&
									(WorldParts->Items[Teller]->PlayFieldY == WorldPart->PlayFieldY)))
							{
								CWorldPart_MoveTo(WorldParts->Items[Teller], WorldPart->PlayFieldX - 1, WorldPart->PlayFieldY, false);
								break;
							}
						}
					}
					WorldPart->AnimBase = 0;
				}
				if (WorldPart->Y > WorldPart->PlayFieldY * TileHeight)
				{
					WorldPart->Yi = -WorldPart->MoveSpeed;
					if (WorldParts)
					{
						for (Teller = 0; Teller < WorldParts->ItemCount; Teller++)
						{
							if (WorldParts->Items[Teller]->PNeedToKill || WorldParts->Items[Teller]->BHide)
								continue;

							if ((WorldParts->Items[Teller]->Type == IDBox || WorldParts->Items[Teller]->Type == IDBox2 || 
								WorldParts->Items[Teller]->Type == IDBoxWall || WorldParts->Items[Teller]->Type == IDBoxBomb) &&
								((WorldPart->PlayFieldY == WorldParts->Items[Teller]->PlayFieldY) &&
									(WorldParts->Items[Teller]->PlayFieldX == WorldPart->PlayFieldX)))
							{
								CWorldPart_MoveTo(WorldParts->Items[Teller], WorldPart->PlayFieldX, WorldPart->PlayFieldY - 1, false);
								break;
							}
						}
					}
					WorldPart->AnimBase = 8;
				}
				if (WorldPart->Y < WorldPart->PlayFieldY * TileHeight)
				{
					WorldPart->Yi = WorldPart->MoveSpeed;
					if (WorldParts)
					{
						for (Teller = 0; Teller < WorldParts->ItemCount; Teller++)
						{
							if (WorldParts->Items[Teller]->PNeedToKill || WorldParts->Items[Teller]->BHide)
								continue;

							if ((WorldParts->Items[Teller]->Type == IDBox || WorldParts->Items[Teller]->Type == IDBox2 ||
								WorldParts->Items[Teller]->Type == IDBoxWall || WorldParts->Items[Teller]->Type == IDBoxBomb) &&
								((WorldParts->Items[Teller]->PlayFieldY == WorldPart->PlayFieldY) &&
									(WorldParts->Items[Teller]->PlayFieldX == WorldPart->PlayFieldX)))
							{
								CWorldPart_MoveTo(WorldParts->Items[Teller], WorldPart->PlayFieldX, WorldPart->PlayFieldY + 1, false);
								break;
							}
						}
					}
					WorldPart->AnimBase = 12;
				}
				playGameMoveSound();
				WorldPart->IsMoving = true;
			}
			else
			{
				if (PlayFieldXin > WorldPart->PlayFieldX)
				{
					WorldPart->AnimBase = 4;
				}
				if (PlayFieldXin < WorldPart->PlayFieldX)
				{
					WorldPart->AnimBase = 0;
				}
				if (PlayFieldYin > WorldPart->PlayFieldY)
				{
					WorldPart->AnimBase = 12;
				}
				if (PlayFieldYin < WorldPart->PlayFieldY)
				{
					WorldPart->AnimBase = 8;
				}
				WorldPart->AnimPhase = WorldPart->AnimBase + WorldPart->AnimCounter;
				WorldPart->AnimDelayCounter++;
				if (WorldPart->AnimDelayCounter == WorldPart->AnimDelay)
				{
					WorldPart->AnimDelayCounter = 0;
					WorldPart->AnimCounter++;
					if (WorldPart->AnimCounter == WorldPart->AnimPhases)
						WorldPart->AnimCounter = 0;
				}
			}

		}
	}
	else
	{
		if(!WorldPart->IsMoving)
		{
			if((PlayFieldXin != WorldPart->PlayFieldX) || (PlayFieldYin != WorldPart->PlayFieldY))
				if(CWorldPart_CanMoveTo(WorldPart, PlayFieldXin,PlayFieldYin) || BackWards)
				{
					WorldPart->PlayFieldX = PlayFieldXin;
					WorldPart->PlayFieldY = PlayFieldYin;
					if(WorldPart->X < WorldPart->PlayFieldX*TileWidth)
						WorldPart->Xi = WorldPart->MoveSpeed;
					if(WorldPart->X > WorldPart->PlayFieldX*TileWidth)
						WorldPart->Xi = -WorldPart->MoveSpeed;
					if(WorldPart->Y > WorldPart->PlayFieldY*TileHeight)
						WorldPart->Yi = -WorldPart->MoveSpeed;
					if(WorldPart->Y < WorldPart->PlayFieldY*TileHeight)
						WorldPart->Yi = WorldPart->MoveSpeed;
					WorldPart->IsMoving = true;
					CWorldPart_Event_LeaveCurrentSpot(WorldPart);
				}
		}
	}
}

bool CWorldPart_CanMoveTo(CWorldPart* WorldPart, const int8_t PlayFieldXin,const int8_t PlayFieldYin) 
{
	uint16_t Teller;
	bool Result = true;
	switch(WorldPart->Type)
	{
		case IDPlayer:
		{
			if ((PlayFieldXin >= 0) && (PlayFieldXin < NrOfCols) && (PlayFieldYin >= 0) && (PlayFieldYin < NrOfRows))
			{
				if (WorldParts)
				{
					for (Teller = 0; Teller < WorldParts->ItemCount; Teller++)
					{
						if (WorldParts->Items[Teller]->PNeedToKill || WorldParts->Items[Teller]->BHide)
							continue;

						if ((WorldParts->Items[Teller]->PlayFieldX == PlayFieldXin) && (WorldParts->Items[Teller]->PlayFieldY == PlayFieldYin))
						{
							if (WorldParts->Items[Teller]->Type == IDWall || WorldParts->Items[Teller]->Type == IDWallBreakable || 
								WorldParts->Items[Teller]->Type == IDPlayer || WorldParts->Items[Teller]->Type == IDPlayer2 || 
								WorldParts->Items[Teller]->Type == IDBox2)
							{
								Result = false;
								break;
							}
							if (WorldParts->Items[Teller]->Type == IDBox || WorldParts->Items[Teller]->Type == IDBox1 ||
								WorldParts->Items[Teller]->Type == IDBoxWall || WorldParts->Items[Teller]->Type == IDBoxBomb)
							{
								if (WorldPart->PlayFieldX > PlayFieldXin)
								{
									Result = CWorldPart_CanMoveTo(WorldParts->Items[Teller], PlayFieldXin - 1, PlayFieldYin);
								}
								if (WorldPart->PlayFieldX < PlayFieldXin)
								{
									Result = CWorldPart_CanMoveTo(WorldParts->Items[Teller], PlayFieldXin + 1, PlayFieldYin);
								}
								if (WorldPart->PlayFieldY > PlayFieldYin)
								{
									Result = CWorldPart_CanMoveTo(WorldParts->Items[Teller], PlayFieldXin, PlayFieldYin - 1);
								}
								if (WorldPart->PlayFieldY < PlayFieldYin)
								{
									Result = CWorldPart_CanMoveTo(WorldParts->Items[Teller], PlayFieldXin, PlayFieldYin + 1);
								}
								break;
							}
						}
					}
				}
				else
					Result = false;
			}
			break;
		}
		case IDPlayer2:
		{
			if ((PlayFieldXin >= 0) && (PlayFieldXin < NrOfCols) && (PlayFieldYin >= 0) && (PlayFieldYin < NrOfRows))
			{
				if (WorldParts)
				{
					for (Teller=0;Teller<WorldParts->ItemCount;Teller++)
					{
						if (WorldParts->Items[Teller]->PNeedToKill || WorldParts->Items[Teller]->BHide)
							continue;

						if((WorldParts->Items[Teller]->PlayFieldX == PlayFieldXin) && (WorldParts->Items[Teller]->PlayFieldY == PlayFieldYin))
						{
							if(WorldParts->Items[Teller]->Type == IDWall || WorldParts->Items[Teller]->Type == IDWallBreakable ||
								WorldParts->Items[Teller]->Type == IDPlayer || WorldParts->Items[Teller]->Type == IDPlayer2 || 
								WorldParts->Items[Teller]->Type == IDBox1)
							{
								Result = false;
								break;
							}
							if(WorldParts->Items[Teller]->Type == IDBox || WorldParts->Items[Teller]->Type == IDBox2 ||
								WorldParts->Items[Teller]->Type == IDBoxWall || WorldParts->Items[Teller]->Type == IDBoxBomb)
							{

								if (WorldPart->PlayFieldX > PlayFieldXin)
								{
									Result = CWorldPart_CanMoveTo(WorldParts->Items[Teller], PlayFieldXin-1,PlayFieldYin);
								}
								if (WorldPart->PlayFieldX < PlayFieldXin)
								{
									Result = CWorldPart_CanMoveTo(WorldParts->Items[Teller], PlayFieldXin+1,PlayFieldYin);
								}
								if (WorldPart->PlayFieldY > PlayFieldYin)
								{
									Result = CWorldPart_CanMoveTo(WorldParts->Items[Teller], PlayFieldXin,PlayFieldYin-1);
								}
								if (WorldPart->PlayFieldY < PlayFieldYin)
								{
									Result = CWorldPart_CanMoveTo(WorldParts->Items[Teller], PlayFieldXin,PlayFieldYin+1);
								}
								break;
							}
						}
					}
				}
				else
					Result = false;
			}
			break;
		}

		case IDBox:
		{
			if ((PlayFieldXin >= 0) && (PlayFieldXin < NrOfCols) && (PlayFieldYin >= 0) && (PlayFieldYin < NrOfRows))
			{
				if (WorldParts)
				{
					for (Teller = 0; Teller < WorldParts->ItemCount; Teller++)
					{
						if (WorldParts->Items[Teller]->PNeedToKill || WorldParts->Items[Teller]->BHide)
							continue;

						if ((WorldParts->Items[Teller]->Type == IDWall) || (WorldParts->Items[Teller]->Type == IDWallBreakable) ||
							(WorldParts->Items[Teller]->Type == IDBox) || (WorldParts->Items[Teller]->Type == IDDiamond) ||
							(WorldParts->Items[Teller]->Type == IDPlayer) || (WorldParts->Items[Teller]->Type == IDPlayer2) ||
							(WorldParts->Items[Teller]->Type == IDBox1) || (WorldParts->Items[Teller]->Type == IDBox2) ||
							(WorldParts->Items[Teller]->Type == IDBoxWall))
							if ((WorldParts->Items[Teller]->PlayFieldX == PlayFieldXin) && (WorldParts->Items[Teller]->PlayFieldY == PlayFieldYin))
							{
								Result = false;
								break;
							}
					}
				}
			}
			else
				Result = false;
			break;
		}

		case IDBoxBomb:
		{
			if ((PlayFieldXin >= 0) && (PlayFieldXin < NrOfCols) && (PlayFieldYin >= 0) && (PlayFieldYin < NrOfRows))
			{
				if (WorldParts)
				{
					for (Teller = 0; Teller < WorldParts->ItemCount; Teller++)
					{
						if (WorldParts->Items[Teller]->PNeedToKill || WorldParts->Items[Teller]->BHide)
							continue;

						if ((WorldParts->Items[Teller]->Type == IDWall) ||
							(WorldParts->Items[Teller]->Type == IDDiamond))
							if ((WorldParts->Items[Teller]->PlayFieldX == PlayFieldXin) && (WorldParts->Items[Teller]->PlayFieldY == PlayFieldYin))
							{
								Result = false;
								break;
							}
					}
				}
			}
			else
				Result = false;
			break;
		}

		case IDBoxWall:
		{
			if ((PlayFieldXin >= 0) && (PlayFieldXin < NrOfCols) && (PlayFieldYin >= 0) && (PlayFieldYin < NrOfRows))
			{
				if (WorldParts)
				{
					for (Teller = 0; Teller < WorldParts->ItemCount; Teller++)
					{
						if (WorldParts->Items[Teller]->PNeedToKill || WorldParts->Items[Teller]->BHide)
							continue;

						if ((WorldParts->Items[Teller]->Type == IDWall) || (WorldParts->Items[Teller]->Type == IDWallBreakable) ||
							(WorldParts->Items[Teller]->Type == IDBox) || (WorldParts->Items[Teller]->Type == IDDiamond) ||
							(WorldParts->Items[Teller]->Type == IDPlayer) || (WorldParts->Items[Teller]->Type == IDPlayer2) ||
							(WorldParts->Items[Teller]->Type == IDBox1) || (WorldParts->Items[Teller]->Type == IDBox2))
							if ((WorldParts->Items[Teller]->PlayFieldX == PlayFieldXin) && (WorldParts->Items[Teller]->PlayFieldY == PlayFieldYin))
							{
								Result = false;
								break;
							}
					}
				}
			}
			else
				Result = false;
			break;
		}
		
		case IDBox1:
		{
			if (WorldParts->ActivePlayer != IDPlayer)
				return false;
			if ((PlayFieldXin >= 0) && (PlayFieldXin < NrOfCols) && (PlayFieldYin >= 0) && (PlayFieldYin < NrOfRows))
			{
				if (WorldParts)
				{
					for (Teller = 0; Teller < WorldParts->ItemCount; Teller++)
					{
						if (WorldParts->Items[Teller]->PNeedToKill || WorldParts->Items[Teller]->BHide)
							continue;

						if ((WorldParts->Items[Teller]->Type == IDWall) || (WorldParts->Items[Teller]->Type == IDWallBreakable) ||
							(WorldParts->Items[Teller]->Type == IDBox) || (WorldParts->Items[Teller]->Type == IDDiamond) ||
							(WorldParts->Items[Teller]->Type == IDPlayer) || (WorldParts->Items[Teller]->Type == IDPlayer2) ||
							(WorldParts->Items[Teller]->Type == IDBox1) || (WorldParts->Items[Teller]->Type == IDBox2) ||
							(WorldParts->Items[Teller]->Type == IDBoxWall))
							if ((WorldParts->Items[Teller]->PlayFieldX == PlayFieldXin) && (WorldParts->Items[Teller]->PlayFieldY == PlayFieldYin))
							{
								Result = false;
								break;
							}
					}
				}
			}
			else
				Result = false;
			break;
		}

		case IDBox2:
		{
			if (WorldParts->ActivePlayer != IDPlayer2)
				return false;
			if ((PlayFieldXin >= 0) && (PlayFieldXin < NrOfCols) && (PlayFieldYin >= 0) && (PlayFieldYin < NrOfRows))
			{
				if (WorldParts)
				{
					for (Teller = 0; Teller < WorldParts->ItemCount; Teller++)
					{
						if (WorldParts->Items[Teller]->PNeedToKill || WorldParts->Items[Teller]->BHide)
							continue;

						if ((WorldParts->Items[Teller]->Type == IDWall) || (WorldParts->Items[Teller]->Type == IDWallBreakable) ||
							(WorldParts->Items[Teller]->Type == IDBox) || (WorldParts->Items[Teller]->Type == IDDiamond) ||
							(WorldParts->Items[Teller]->Type == IDPlayer) || (WorldParts->Items[Teller]->Type == IDPlayer2) ||
							(WorldParts->Items[Teller]->Type == IDBox1) || (WorldParts->Items[Teller]->Type == IDBox2) ||
							(WorldParts->Items[Teller]->Type == IDBoxWall))
							if ((WorldParts->Items[Teller]->PlayFieldX == PlayFieldXin) && (WorldParts->Items[Teller]->PlayFieldY == PlayFieldYin))
							{
								Result = false;
								break;
							}
					}
				}
			}
			else
				Result = false;
			break;
		}
		default:
			Result = false;
			break;
	}


	return Result;
}

void CWorldPart_Move(CWorldPart* WorldPart)
{
	if (WorldPart->IsMoving && !WorldPart->PNeedToKill && !WorldPart->BHide)
	{
		if (WorldPart->MoveDelayCounter == WorldPart->MoveDelay)
		{
			WorldPart->X += WorldPart->Xi;
			WorldPart->Y += WorldPart->Yi;
			CWorldPart_Event_Moving(WorldPart, WorldPart->X,WorldPart->Y,WorldPart->Xi,WorldPart->Yi);
			if ((WorldPart->X == WorldPart->PlayFieldX * TileWidth) && (WorldPart->Y == WorldPart->PlayFieldY * TileHeight))
			{
				WorldPart->IsMoving = false;
				WorldPart->Xi = 0;
				WorldPart->Yi = 0;
				CWorldPart_Event_ArrivedOnNewSpot(WorldPart);
			}
			WorldPart->MoveDelayCounter = -1;
		}
		WorldPart->MoveDelayCounter++;
	}
}

//the image for the frame this part is currently showing. Every type is drawn the
//same way, an 8x8 magenta keyed sprite taken from its sheet at AnimPhase
const uint8_t* CWorldPart_SpriteData(CWorldPart* WorldPart)
{
	const uint8_t* base;
	switch (WorldPart->Type)
	{
		case IDPlayer:         base = IMGPlayer;    break;
		case IDPlayer2:        base = IMGPlayer2;   break;
		case IDBox:
		case IDBox1:
		case IDBox2:
		case IDBoxWall:
		case IDBoxBomb:        base = IMGBox;         break;
		case IDDiamond:        base = IMGDiamond;     break;
		case IDBomb:           base = IMGBomb;        break;
		case IDFloor:          base = IMGFloor;       break;
		case IDWall:
		case IDWallBreakable:  base = IMGWall;        break;
		case IDExplosion:      base = IMGExplosion;   break;
		default: return NULL;
	}
	return base + WorldPart->AnimPhase * TileWidth * TileHeight * sizeof(uint16_t);
}

void CWorldPart_Draw(CWorldPart* WorldPart)
{
	if (!WorldPart->BHide)
	{
		CWorldPart_Event_BeforeDraw(WorldPart);
		DrawImageTransparent(WorldPart->X - WorldParts->ViewPort->MinScreenX, WorldPart->Y - WorldParts->ViewPort->MinScreenY,
		                     TileWidth, TileHeight, CWorldPart_SpriteData(WorldPart));
	}
}

void CWorldPart_Destroy(CWorldPart* WorldPart)
{
	if (WorldPart)
		//hand the pool slot back, Type 0 marks it free again
		WorldPart->Type = 0;
}
