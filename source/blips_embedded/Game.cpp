#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "Common.h"
#include "GameFuncs.h"
#include "Sound.h"
#include "CWorldPart.h"
#include "CWorldParts.h"

bool ExplosionsFound = false;
bool FreeView = false;
//forces the free view bar to be drawn even when the board did not repaint
bool NeedRedraw = false;

bool StageDone()
{
	for (uint16_t Teller = 0; Teller < WorldParts->ItemCount; Teller++)
		if (WorldParts->Items[Teller]->Type == IDDiamond)
			return false;
	return true;
}

void RestartLevel()
{
	if (!AskingQuestion)
	{
		playMenuSelectSound();
		CWorldParts_DrawBoard(WorldParts);
		AskQuestion(IDRestartLevel, "You are about to\nrestart this level\nAre you sure you\nwant to restart?\n\n(A)Restart (B)Cancel");
	}
}

void GameInit()
{
	uint16_t Teller;
	//this screen is about to be rebuilt, drop any cached draw signature
	ScreenForceRedraw();
	//whatever the previous state left on screen has to go
	CWorldParts_MarkAllDirty();
	ExplosionsFound = false;
	FreeView = false;
	NeedRedraw = true;
	CWorldPart *Player=NULL;
	CWorldPart *Player2 = NULL;
	for (Teller=0;Teller<WorldParts->ItemCount;Teller++)
	{
		if (WorldParts->Items[Teller]->Type == IDPlayer)
		{
			Player = WorldParts->Items[Teller];
			break;
		}

		if (WorldParts->Items[Teller]->Type == IDPlayer2)
		{
			Player2 = WorldParts->Items[Teller];
		}
	}
	//should never happen
	if(!Player && !Player2)
	{
		Player = CWorldPart_Create(0,0, IDPlayer);
		CWorldParts_Add(WorldParts,Player);
		CWorldParts_LimitVPLevel(WorldParts);
	}
}

void Game()
{
	char Msg[200];
	if(GameState == GSGameInit)
	{
		GameInit();
		GameState = GSGame;
	}
	bool response = false;
	int8_t id = -1;
	if(!AskingQuestion)
	{
		if (!WorldParts->Player->IsMoving && StageDone())
		{
			playLevelDoneSound();
			if (SelectedLevel == UnlockedLevels)
			{
				if ( UnlockedLevels < InstalledLevels)
				{
					snprintf(Msg, sizeof(Msg),"Congratulations !\nYou Solved\nLevel %d/%d\nThe next level has\nnow been unlocked!\n(A) Continue",SelectedLevel,InstalledLevels);
					AskQuestion(IDSolvedLevelNextUnlocked, Msg);
				}
				else
				{
					snprintf(Msg, sizeof(Msg),"Congratulations !\nYou Solved\nLevel %d/%d\nlevelpack %s\nis now finished,\ntry out another one!\n(A) Continue",SelectedLevel,InstalledLevels,LevelPackName);
					AskQuestion(IDSolvedLastLevel, Msg);
						
				}
			}
			else
			{
				snprintf(Msg, sizeof(Msg),"Congratulations !\nYou Solved\nLevel %d/%d\n\n(A) Continue",SelectedLevel,InstalledLevels);
				AskQuestion(IDSolvedEarlierLevel, Msg);
			}
		}
		
		if (!FreeView && (currButtons & BUTTON_B) && !(prevButtons & BUTTON_B))
		{
			playMenuBackSound();
			AskQuestion(IDQuitPlaying, "Quit playing the\ncurrent level and\nreturn to the level\nselector?\n\n(A) Quit (B) Cancel");
		}

		//restart
		if ((currButtons & BUTTON_L) && (!(prevButtons & BUTTON_L)))
		{
			RestartLevel();
		}

		//freeview
		if (!FreeView && (currButtons & BUTTON_R) && (!(prevButtons & BUTTON_R)))
		{
			playMenuSelectSound();
			FreeView = true;
			NeedRedraw = true;
			prevButtons = currButtons;
		}

		if (FreeView)
		{
			if (((currButtons & BUTTON_B) && (!(prevButtons & BUTTON_B))) ||
				((currButtons & BUTTON_R) && (!(prevButtons & BUTTON_R))))
			{
				playMenuBackSound();
				FreeView = false;
				CWorldParts_CenterVPOnPlayer(WorldParts);
			}

			//the board notices the viewport scrolled and repaints everything by itself
			if (FreeView)
			{
				if (currButtons & BUTTON_LEFT)
					CViewPort_Move(WorldParts->ViewPort, -ViewportMove, 0);
				if (currButtons & BUTTON_RIGHT)
					CViewPort_Move(WorldParts->ViewPort, ViewportMove, 0);
				if (currButtons & BUTTON_UP)
					CViewPort_Move(WorldParts->ViewPort, 0, -ViewportMove);
				if (currButtons & BUTTON_DOWN)
					CViewPort_Move(WorldParts->ViewPort, 0, ViewportMove);
			}
		}
		else
		{
			if (!WorldParts->Player->IsMoving && (((WorldParts->Player1) && !WorldParts->Player1->IsDeath) || ((WorldParts->Player2) && !WorldParts->Player2->IsDeath)))
			{
				if (!(prevButtons & BUTTON_A) && (currButtons & BUTTON_A))
				{
					CWorldParts_SwitchPlayers(WorldParts);
				}

				if (currButtons & BUTTON_RIGHT)
				{
					CWorldPart_MoveTo(WorldParts->Player, WorldParts->Player->PlayFieldX + 1, WorldParts->Player->PlayFieldY, false);
				}
				else
				{
					if (currButtons & BUTTON_LEFT)
					{
						CWorldPart_MoveTo(WorldParts->Player, WorldParts->Player->PlayFieldX - 1, WorldParts->Player->PlayFieldY, false);
					}
					else
					{
						if (currButtons & BUTTON_UP)
						{
							CWorldPart_MoveTo(WorldParts->Player, WorldParts->Player->PlayFieldX, WorldParts->Player->PlayFieldY - 1, false);
						}
						else
						{
							if (currButtons & BUTTON_DOWN)
							{
								CWorldPart_MoveTo(WorldParts->Player, WorldParts->Player->PlayFieldX, WorldParts->Player->PlayFieldY + 1, false);
							}
						}
					}
				}
			}
		}

		if(!AskingQuestion)
		{
			CWorldParts_Move(WorldParts);
			bool boardPainted = CWorldParts_DrawBoard(WorldParts);
			//the top bar sits over the board, so it is repainted whenever the board was
			if (FreeView && (boardPainted || NeedRedraw))
			{
				GFX.fillRect(0, 0, WINDOW_WIDTH, 12, ColorWhite);
				GFX.drawRect(0, 11, WINDOW_WIDTH, 1, ColorBlack);
				tftPrint(2, 2, "dpad:Move B:exit", ColorBlack, ColorBlack, 1);
			}
			NeedRedraw = false;
			if (((WorldParts->Player1) && WorldParts->Player1->IsDeath) || ((WorldParts->Player2) && WorldParts->Player2->IsDeath))
			{
				ExplosionsFound = false;
				for (uint16_t teller = 0; teller < WorldParts->ItemCount; teller++)
				{
					if (WorldParts->Items[teller]->Type == IDExplosion)
					{
						ExplosionsFound = true;
						break;
					}
				}

				if (!ExplosionsFound)
				{
					AskQuestion(IDPlayerDied, "Too bad you died !\nDo you want to\ntry again?\n\n(A) Try Again\n(B) Level Selector");
				}
			}
		}
	}
	else
	{
		if(AskQuestionUpdate(&id, &response, false))
		{
			if (id == IDPlayerDied)
			{
				if (response)
				{
					CWorldParts_LoadFromLevelPackFile(WorldParts, LevelPackFile, SelectedLevel, true);
					FreeView = false;
				}
				else
				{
					GameState = GSStageSelectInit;
				}
			}

			if(id == IDSolvedLevelNextUnlocked)
			{
				UnlockedLevels++;
				SelectedLevel++;
				SaveUnlockData();
				GameState = GSStageSelectInit;
			}

			if(id == IDSolvedLastLevel)
			{
				GameState = GSTitleScreenInit;
			}

			if (id == IDSolvedEarlierLevel)
			{
				GameState = GSStageSelectInit;
			}

			if (id == IDRestartLevel)
			{
				if(response)
				{
					CWorldParts_LoadFromLevelPackFile(WorldParts, LevelPackFile, SelectedLevel, true);
					FreeView = false;
				}
				else
				{
					//the question was drawn over the free view bar
					NeedRedraw = true;
				}
			}

			if (id == IDQuitPlaying)
			{
				//keep playing needs no repaint here, closing the question marked the board dirty
				if (response)
				{
					GameState = GSStageSelectInit;
				}
			}
		}
	}
}
