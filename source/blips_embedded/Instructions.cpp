#include <string.h>
#include "Credits.h"
#include "Common.h"
#include "GameFuncs.h"
#include "Sound.h"

	
void InstructionsInit()
{

}

void Instructions()
{
	
	if (GameState == GSInstructionsInit)
	{
		InstructionsInit();
		GameState = GSInstructions;
	}

	if (((currButtons & BUTTON_A) && !(prevButtons & BUTTON_A)) ||
		((currButtons & BUTTON_B) && !(prevButtons & BUTTON_B)))
	{
		playMenuBackSound();
		GameState = GSTitleScreenInit;
	}
	if (ScreenChanged(GameState * 100000))
	{
		pushImageRLE(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, IMGInstructions);
	}
}
