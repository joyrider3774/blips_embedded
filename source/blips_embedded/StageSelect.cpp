#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "StageSelect.h"
#include "Common.h"
#include "GameFuncs.h"
#include "Sound.h"
#include "CWorldParts.h"
#include "CLevelPackFile.h"

void StageSelectInit()
{
	//this screen is about to be rebuilt, drop any cached draw signature
	ScreenForceRedraw();
	//coming back from a game or a menu, repaint the whole board
	CWorldParts_MarkAllDirty();
	if (SelectedLevel > 0)
	{
		CWorldParts_LoadFromLevelPackFile(WorldParts, LevelPackFile, SelectedLevel, true);
	}
	else
		CWorldParts_RemoveAll(WorldParts);
	
}

void StageSelect()
{
	char Tekst[180];
	if (GameState == GSStageSelectInit)
	{
		StageSelectInit();
		GameState = GSStageSelect;
	}
	int8_t id = -1;
	bool response = false;
	bool boardPainted = false;
	if(!AskingQuestion)
	{
		if (SelectedLevel == 0)
			pushImageRLE(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, IMGBackground);
		if (SelectedLevel > 0)
		{
			boardPainted = CWorldParts_DrawBoard(WorldParts);
			if ((InstalledLevels > 0) && boardPainted)
			{
				GFX.fillRect(-1, 115, 128, 130, ColorWhite);
				GFX.drawRect(-1, 115, 128, 130, ColorBlack);
				if (WorldParts->isLevelPackFileLevel)
				{
					if ((strlen(LevelPackFile->Meta.title) > 0) || (strlen(LevelPackFile->Meta.author) > 0))
					{
						if (strlen(LevelPackFile->Meta.author) > 0)
							snprintf(Tekst, sizeof(Tekst), "%s", LevelPackFile->Meta.title);
						else
							snprintf(Tekst, sizeof(Tekst), "%s", LevelPackFile->Meta.title);
						tftPrint(2, 118, Tekst, ColorBlack, ColorBlack, 1);
					}
				}
			}
		}
		//the top bar sits over the board, so it is repainted whenever the board was
		//ScreenChanged has to come first, || would otherwise short circuit it away on
		//every frame the board painted and this screen's signature would never be
		//recorded, leaving the screen we return to thinking nothing had changed
		if (ScreenChanged(GameState * 100000 + SelectedLevel * 10 + (SelectedLevel <= UnlockedLevels ? 1 : 0)) || boardPainted)
		{
			GFX.fillRect(-1, -1, 130, 13, ColorWhite);
			GFX.drawRect(-1, -1, 130, 13, ColorBlack);

			if(SelectedLevel <= UnlockedLevels)
				snprintf(Tekst, sizeof(Tekst),"Lvl:%d/%d A:Play",SelectedLevel,InstalledLevels);
			else
				snprintf(Tekst, sizeof(Tekst),"Lvl:%d/%d Locked!",SelectedLevel,InstalledLevels);
			tftPrint(2, 2, Tekst, ColorBlack, ColorBlack, 1);
		}

		if ((currButtons & BUTTON_B) && !(prevButtons & BUTTON_B))
		{
			GameState= GSTitleScreenInit;
			playMenuBackSound();
			CWorldParts_RemoveAll(WorldParts);			
		}

		if ((currButtons & BUTTON_A) && !(prevButtons & BUTTON_A))
		{
			playMenuSelectSound();
			if (SelectedLevel <= UnlockedLevels)
				GameState = GSGameInit;
			else
			{
				snprintf(Tekst, sizeof(Tekst),"This Level Hasn't\nbeen unlocked yet!\nDo you want to play\nthe last unlocked\nlevel %d/%d\n(A) Play (B) Cancel",UnlockedLevels,InstalledLevels);
				AskQuestion(IDLevelNotUnlocked, Tekst);
			}
		}

		if (currButtons & BUTTON_LEFT)
		{
			if (!(prevButtons & BUTTON_LEFT))
				frameLeftStart = framecount;
			if ((framecount - frameLeftStart) % LevelSelectUpdateTicks == 0)
			{
				if (SelectedLevel != 1)
				{
					playMenuSound();

					SelectedLevel--;
					if (SelectedLevel < 1)
						SelectedLevel = 1;
					CWorldParts_LoadFromLevelPackFile(WorldParts, LevelPackFile, SelectedLevel, true);
				}
			}
		}

		if (currButtons & BUTTON_RIGHT)
		{
			if (!(prevButtons & BUTTON_RIGHT))
				frameRightStart = framecount;
			if ((framecount - frameRightStart) % LevelSelectUpdateTicks == 0)
			{
				if (SelectedLevel != InstalledLevels)
				{
					SelectedLevel++;
					playMenuSound();
					if (SelectedLevel > InstalledLevels)
						SelectedLevel = InstalledLevels;
					CWorldParts_LoadFromLevelPackFile(WorldParts, LevelPackFile, SelectedLevel, true);
				}
			}
		}
	}
	else
	{
		if(AskQuestionUpdate(&id, &response, false))
		{
			if( id == IDLevelNotUnlocked)
			{
				if(response)
				{			
					SelectedLevel = UnlockedLevels;
					CWorldParts_LoadFromLevelPackFile(WorldParts, LevelPackFile, SelectedLevel, true);
					GameState = GSGameInit;
				}
			}

		}
	}
}
