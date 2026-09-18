#include <string.h>
#include <stdio.h>
#include "Defines.h"
#include "TitleScreen.h"
#include "Common.h"
#include "GameFuncs.h"
#include "Sound.h"

void OptionsInit()
{
	//this screen is about to be rebuilt, drop any cached draw signature
	ScreenForceRedraw();
	Selection = 1;
}


void Options()
{
	char Tekst[100];

	if (GameState == GSOptionsInit)
	{
		OptionsInit();
		GameState = GSOptions;
	}

	if (ScreenChanged(GameState * 100000 + Selection * 1000 + skin * 100 + (isMusicOn() ? 10 : 0) + (isSoundOn() ? 1 : 0)))
	{
		pushImageRLE(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, IMGTitleScreen);
		printTitleInfo();

		GFX.fillRect(10, 38, 104, 52, ColorWhite);
		GFX.drawRect(10, 38, 104, 52, ColorBlack);
		GFX.drawRect(12, 40, 100, 48, ColorBlack);
		snprintf(Tekst, sizeof(Tekst),"Sound: %s\nSkin: %d\nMain Menu", isSoundOn() ? "On" : "Off", CurrentSkin()+1);
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

			if (Selection == 1)
			{
				playMenuSelectSound();
				setSoundOn(!isSoundOn());
				SaveSettings();
			}

			if (Selection == 2)
			{
#if FORCESKIN >= 0
				//only one skin is built in, FORCESKIN (a 1 bpp buffer forces the black & white one)
				playErrorSound();
#else
				playMenuSelectSound();
				skin--;
				if(skin < 0)
					skin = MAXSKINS -1;
				LoadGraphics();
				SaveSettings();
#endif
			}
		}
	}
		
	if (currButtons & BUTTON_RIGHT)
	{
		if (!(prevButtons & BUTTON_RIGHT))
			frameRightStart = framecount;
		if ((framecount - frameRightStart) % MenuUpdateTicks == 0)
		{

			if (Selection == 1)
			{
				playMenuSelectSound();
				setSoundOn(!isSoundOn());
				SaveSettings();
			}

			if (Selection == 2)
			{
#if FORCESKIN >= 0
				//only one skin is built in, FORCESKIN (a 1 bpp buffer forces the black & white one)
				playErrorSound();
#else
				playMenuSelectSound();
				skin++;
				if(skin == MAXSKINS)
					skin = 0;
				LoadGraphics();
				SaveSettings();
#endif
			}
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
			if (Selection < 3)
			{
				Selection++;
				playMenuSound();
			}
		}
	}
	
	if ((currButtons & BUTTON_B) && !(prevButtons & BUTTON_B))
	{
		playMenuBackSound();
		GameState = GSTitleScreenInit;
	}

	if ((currButtons & BUTTON_A) && !(prevButtons & BUTTON_A))
	{
		switch(Selection)
		{
			case 1:
				playMenuSelectSound();
				setSoundOn(!isSoundOn());
				SaveSettings();
				break;
			case 2:
#if FORCESKIN >= 0
				//only one skin is built in, FORCESKIN (a 1 bpp buffer forces the black & white one)
				playErrorSound();
#else
				playMenuSelectSound();
				skin++;
				if(skin >= MAXSKINS)
					skin = 0;
				LoadGraphics();
				SaveSettings();
#endif
				break;
			case 3:
				GameState=GSTitleScreenInit;
				playMenuSelectSound();
				break;
			default:
				break;
		}
	}
	
}
