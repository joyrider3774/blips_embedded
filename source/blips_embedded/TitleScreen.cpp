#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "Defines.h"
#include "TitleScreen.h"
#include "Common.h"
#include "GameFuncs.h"
#include "Sound.h"

static void SelectLevelPack(int8_t Delta)
{
	if (InstalledLevelPacksCount == 0)
		return;
	SelectedLevelPack += Delta;
	if (SelectedLevelPack < 0)
		SelectedLevelPack = InstalledLevelPacksCount - 1;
	if (SelectedLevelPack > InstalledLevelPacksCount - 1)
		SelectedLevelPack = 0;
	LevelPackName = InstalledLevelPacks[SelectedLevelPack];
	CLevelPackFile_loadFile(LevelPackFile, LevelPackName, NrOfCols, NrOfRows, LPLevelHeaderOnly);
	playMenuSound();
	SaveSettings();
}

void TitleScreenInit()
{
	//this screen is about to be rebuilt, drop any cached draw signature
	ScreenForceRedraw();
	Selection = 1;
	CLevelPackFile_loadFile(LevelPackFile, LevelPackName, NrOfCols, NrOfRows, LPLevelHeaderOnly);
}


void TitleScreen()
{
	char Tekst[160];

	if (GameState == GSTitleScreenInit)
	{
		TitleScreenInit();
		GameState = GSTitleScreen;
	}
	int8_t id = -1;
	bool response = false;
	if(!AskingQuestion)
	{
		if (ScreenChanged(GameState * 100000 + Selection * 1000 + SelectedLevelPack))
		{
			pushImageRLE(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, IMGTitleScreen);
			printTitleInfo();

			GFX.fillRect(10, 38, 104, 52, ColorWhite);
			GFX.drawRect(10, 38, 104, 52, ColorBlack);
			GFX.drawRect(12, 40, 100, 48, ColorBlack);
			snprintf(Tekst, sizeof(Tekst),"Play Pack\n<%s>\nOptions\nInstructions\nCredits",LevelPackName);
			tftPrint(28, 42, Tekst, ColorBlack, ColorBlack, 1);
			//The ">>" marker one line lower for every entry above the selected one. It is put
			//together here rather than with a "%.*s" precision in the format: not every device's
			//printf understands one, and on the PlayStation it printed "(null)" instead
			uint16_t at = 0;
			if (Selection > 1)
			{
				Tekst[at++] = ' ';
				Tekst[at++] = '\n';
			}
			for (uint8_t above = 2; above < Selection; above++)
				Tekst[at++] = '\n';
			Tekst[at++] = '>';
			Tekst[at++] = '>';
			Tekst[at] = '\0';
			tftPrint(14, 42, Tekst, ColorBlack, ColorBlack, 1);
		}
		
		if (currButtons & BUTTON_LEFT)
		{
			if (!(prevButtons & BUTTON_LEFT))
				frameLeftStart = framecount;
			if ((framecount - frameLeftStart) % MenuUpdateTicks == 0)
			{
				if (Selection == 2)
					SelectLevelPack(-1);
			}
		}
		
		if (currButtons & BUTTON_RIGHT)
		{
			if (!(prevButtons & BUTTON_RIGHT))
				frameRightStart = framecount;
			if ((framecount - frameRightStart) % MenuUpdateTicks == 0)
			{
				if (Selection == 2)
					SelectLevelPack(1);
			}
		}

		if ((currButtons & BUTTON_UP))
		{
			if (!(prevButtons & BUTTON_UP))
				frameUpStart = framecount;
			if (((framecount - frameUpStart) % MenuUpdateTicks == 0))
			{
				if (Selection > 1)
				{
					Selection--;
					playMenuSound();
				}
			}
		}

		if ((currButtons & BUTTON_DOWN))
		{
			if (!(prevButtons & BUTTON_DOWN))
				frameDownStart = framecount;
			if (((framecount - frameDownStart) % MenuUpdateTicks == 0))
			{
				if (Selection < 5)
				{
					Selection++;
					playMenuSound();
				}
			}
		}

		if ((currButtons & BUTTON_A) && !(prevButtons & BUTTON_A))
		{
			switch(Selection)
			{
				case 1:
					if (InstalledLevelPacksCount > 0)
					{
						CLevelPackFile_loadFile(LevelPackFile, LevelPackName, NrOfCols, NrOfRows, LPLevelCountOnly);
						FindLevels();
						if (InstalledLevels > 0)
						{
							LoadUnlockData();
							SelectedLevel = UnlockedLevels;
							GameState = GSStageSelectInit;
							playMenuSelectSound();
						}
						else
						{
							playMenuSelectSound();
							snprintf(Tekst, sizeof(Tekst), "There are no levels\nfound in levelpack\n%s\n\nPlease create a level\nfor this level pack\nfirst!", LevelPackName);
							AskQuestion(IDNoLevelsInPack, Tekst);
						}
					}
					break;
				case 2:
					SelectLevelPack(1);
					break;
				case 3:
					GameState = GSOptionsInit;
					playMenuSelectSound();
					break;
				case 4:
					GameState = GSInstructionsInit;
					playMenuSelectSound();
					break;
				case 5:
					GameState=GSCreditsInit;
					playMenuSelectSound();
					break;
			}
		}
		
	}
	AskQuestionUpdate(&id, &response, false);
}

