#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "Common.h"
#include "Intro.h"
#include "CWorldParts.h"
#include "GameFuncs.h"
#include "StageSelect.h"
#include "Credits.h"
#include "TitleScreen.h"
#include "Game.h"
#include "Sound.h"
#include "Options.h"
#include "Instructions.h"

//The program itself, Game_Setup and Game_Loop are called by the device's own source

const uint32_t timePerFrame =  1000000 / FPS;
static float frameRate = 0;
static uint32_t currentTime = 0, lastTime = 0, frameTime = 0;
static bool endFrame = true;
bool webAppStore = false;

static uint32_t getFreeRam() {
	return Platform_FreeHeap();
}

static uint32_t getFreeStack() {
	return Platform_FreeStack();
}

//lowest free heap seen since boot, sampled at the end of Game_Setup and of every frame.
//Something allocated and freed again within one frame does not show up here
static uint32_t lowestFreeRam = UINT32_MAX;

static void trackLowestFreeRam()
{
    uint32_t freeRam = getFreeRam();
    if (freeRam < lowestFreeRam)
        lowestFreeRam = freeRam;
}

static void printDebugCpuRamLoad()
{
    if(debugMode || FORCEDEBUG)
    {
        //the text is only put together a few times a second: every frame it would cost the
        //formatting and the heap and stack readings for figures nobody can read that fast.
        //It is still drawn every frame, the board may have been drawn over it
        static char debuginfo[80] = "";
        static uint32_t lastUpdate = 0;
        uint32_t now = Platform_Micros();
        if ((debuginfo[0] == '\0') || (now - lastUpdate >= 250000))
        {
            lastUpdate = now;
            //the whole frames per second and the fraction 0..99, without FPSLOCK the rate can pass 255
            uint16_t fps_int = (uint16_t)frameRate;
            uint8_t fps_frac = (uint8_t)((frameRate - fps_int) * 100);
            //S is the least sketch stack that has been free since boot, out of 4096 bytes
            //L: is the lowest free heap since boot, in the same column as R: on the line above
            snprintf(debuginfo, sizeof(debuginfo), "F:%3d.%2d R:%3" PRIu32 " \nS:%4" PRIu32 "   L:%3" PRIu32 " ", fps_int, fps_frac, getFreeRam(), getFreeStack(), lowestFreeRam);
            //Platform_Log("%s\n", debuginfo);
        }
        tftPrint(0, 0, debuginfo, SCREEN.color565(255,255,255), SCREEN.color565(0,0,0), 1);
    }
}


void Game_Setup(void)
{
    //webAppStore is set in Platform_Init
    Platform_Init("Blips v1.0");
    if(!webAppStore)
    {
        Platform_Log("Free Ram at boot game: %6" PRIu32 "\n", getFreeRam());
    	GameState = GSIntroInit;
    	Selection = 0;
    	InstalledLevelPacksCount = 0;
    	InstalledLevels = 0;
    	SelectedLevel = 0;
    	SelectedLevelPack = 0;
    	UnlockedLevels = 1;
    	AskingQuestionID = -1;
    	AskingQuestion = false;
    	framecount = 0;
    	frameUpStart = 0;
    	frameDownStart = 0;
    	frameLeftStart = 0;
    	frameRightStart = 0;
    	//normal game stuff
    	srand(Platform_RandomSeed());
    	WorldParts = CWorldParts_Create();
    	SearchForLevelPacks();
    	initSound();
    	initMusic();
    	LoadSettings();
    	LoadGraphics();
    	//with a 1 bpp buffer, the colours its set and clear bits are shown in. The skin is
    	//always black & white there
    	Platform_SetBufferColors(ColorWhite, ColorBlack);
    	LevelPackFile = CLevelPackFile_Create();
        trackLowestFreeRam();
        currentTime = Platform_Micros();
        lastTime = 0;
    }
    else
    {
        //webappstore stuff
    }
}

void Game_Loop(void)
{
    if(!webAppStore)
    {        
        currentTime = Platform_Micros();
        frameTime  = currentTime - lastTime;
    #if FPSLOCK
        if((frameTime < timePerFrame) || !endFrame)
           return;
    #else
        //no lock, a frame starts as soon as the last one is done
        if(!endFrame)
           return;
    #endif
        endFrame = false;
        //without the lock two frames can start within the same microsecond on a fast PC
        frameRate = 1000000.0 / (frameTime ? frameTime : 1);
        lastTime = currentTime;
        prevButtons = currButtons;
        currButtons = Platform_GetButtons();
    	processSound();

    	if((currButtons & BUTTON_UP) && (currButtons & BUTTON_DOWN) && !(prevButtons & BUTTON_DOWN))
    		debugMode = !debugMode;

    	switch (GameState)
    	{
    		case GSInstructionsInit:
    		case GSInstructions:
    			Instructions();
    		break;
    		case GSIntroInit:
    		case GSIntro:
    			Intro();
    			break;
    		case GSTitleScreenInit:
    		case GSTitleScreen:
    			TitleScreen();
    			break;
    		case GSCreditsInit:
    		case GSCredits:
    			Credits();
    			break;
    		case GSGameInit:
    		case GSGame:
    			Game();
    			break;
    		case GSStageSelectInit:
    		case GSStageSelect:
    			StageSelect();
    			break;
    		case GSOptionsInit:
    		case GSOptions:
    			Options();
    			break;
    	}
        trackLowestFreeRam();
        printDebugCpuRamLoad();
        Platform_PresentFrame();
    	framecount++;
        endFrame = true;
    }
    else
    {
        //webappstore stuff
        static uint32_t prev = 0;
        if(Platform_Micros() - prev > 1000000)
        {
            prev = Platform_Micros();
            Platform_Log("Free Ram webappstore: %6" PRIu32 "\n", getFreeRam());
        }
    }
}
