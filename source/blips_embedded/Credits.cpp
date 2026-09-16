#include <stdio.h>
#include <string.h>
#include "Credits.h"
#include "Common.h"
#include "GameFuncs.h"
#include "Sound.h"
#include "CLevelPackFile.h"


//char CreditsTekst[100], CreditsTekst2[100], CreditsTekst3[100];
//the credits never change, print the literal instead of copying it into a ram buffer
static const char* const CreditsTekst = "Creator:\njoyrider3774\njoyrider3774.itch.io\nOrig Creator:\nBryant Brownell\nbryant.brownell@\ngmail.com";

void CreditsInit()
{
	//this screen is about to be rebuilt, drop any cached draw signature
	ScreenForceRedraw();
	//snprintf(CreditsTekst, sizeof(CreditsTekst), "Creator:joyrider3774\njoyrider3774.itch.io");
	//snprintf(CreditsTekst2, sizeof(CreditsTekst2), "Bryant Brownell\nbryant.brownell@gmail.com");
	// if (InstalledLevelPacksCount > 0)
	// {
	// 	if (!LevelPackFile->Loaded)
	// 	{
	// 		snprintf(CreditsTekst3, sizeof(CreditsTekst3), "%s\ncreated by\n%s", LevelPackName, NormalCreateName);
	// 	}
	// 	else
	// 		snprintf(CreditsTekst3, sizeof(CreditsTekst3), "%s\ncreated by\n%s", LevelPackName, LevelPackFile->author);
	// }
	// else
	// 	snprintf(CreditsTekst3, sizeof(CreditsTekst3), "%s\ncreated by\nunknown author", LevelPackName);
}

void Credits()
{
	
	if (GameState == GSCreditsInit)
	{
		CreditsInit();
		GameState = GSCredits;
	}

	if (((currButtons & BUTTON_A) && !(prevButtons & BUTTON_A)) ||
		((currButtons & BUTTON_B) && !(prevButtons & BUTTON_B)))
	{
		playMenuBackSound();
		GameState = GSTitleScreenInit;
	}
	if (ScreenChanged(GameState * 100000))
	{
		pushImageRLE(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, IMGTitleScreen);
		GFX.fillRect(0, 30, 128, 69, ColorWhite);
		GFX.drawRect(0, 30, 128, 69, ColorBlack);
		GFX.drawRect(2, 32, 124, 65, ColorBlack);

		tftPrint(4, 34, CreditsTekst, ColorBlack, ColorBlack, 1);
	
		printTitleInfo();
	}
}
