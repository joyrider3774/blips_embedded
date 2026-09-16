#include <stdint.h>
#include "Common.h"
#include "Defines.h"
#include "GameFuncs.h"

uint8_t IntroScreenNr = 0;

void IntroInit()
{
	IntroScreenNr = 0;
}

void Intro()
{
	if (GameState == GSIntroInit)
	{
		IntroInit();
		GameState = GSIntro;
	}
	
	if((currButtons & BUTTON_A) || (currButtons & BUTTON_B))
	{
		GameState = GSTitleScreenInit;
	}

	if (framecount % 80 == 0)
	{
		IntroScreenNr++;
		if (IntroScreenNr > 2)
		{
			GameState = GSTitleScreenInit;
			return;
		}
	}

	if (ScreenChanged(GameState * 100000 + IntroScreenNr))
	switch(IntroScreenNr)
	{
		case 1 :
			pushImageRLE(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, IMGIntro1);
			break;
		case 2 :
			pushImageRLE(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, IMGIntro2);
			break;
   }
}