#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include "Defines.h"
#include "CWorldParts.h"
#include "CLevelPackFile.h"
#include "Platform.h"

enum GameStates : uint8_t {GSTitleScreen,GSCredits,GSIntro,GSGame,GSStageSelect,GSOptions, GSInstructions,
    GSTitleScreenInit, GSCreditsInit, GSIntroInit, GSGameInit, GSStageSelectInit, GSOptionsInit, GSInstructionsInit
};

//extern LCDBitmap *IMGBackground, *IMGFloor, *IMGPlayer, * IMGPlayer2, *IMGBox, *IMGEmpty, *IMGTitleScreen, *IMGInstructions, *IMGWall,
//            *IMGIntro1, *IMGIntro2, *IMGIntro3, *IMGBomb, *IMGDiamond, *IMGExplosion;
extern uint16_t ColorWhite, ColorBlack;
extern GameStates GameState;
extern CLevelPackFile* LevelPackFile;
//sized to what they hold: pack counts up to MaxLevelPacks, levels up to MAXLEVELS, question
//ids up to IDQuitPlaying. SelectedLevelPack, SelectedLevel and AskingQuestionID go below 0
//(-1 = no question, or a pack / level just before being wrapped / clamped)
static_assert((MaxLevelPacks <= 127) && (MAXLEVELS <= 127) && (IDQuitPlaying <= 127), "globals do not fit their types");
extern uint8_t InstalledLevelPacksCount, InstalledLevels, UnlockedLevels;
extern int8_t SelectedLevel, SelectedLevelPack, AskingQuestionID;
extern bool AskingQuestion;
extern CWorldParts *WorldParts;
extern char InstalledLevelPacks[MaxLevelPacks][MaxLevelPackNameLength];
extern char LevelPackName[MaxLevelPackNameLength];
extern uint8_t currButtons, prevButtons;
extern uint8_t Selection;      //menu entry, 0 (none yet) .. 4
//frame counters run for as long as the game does, menus repeat on (framecount - start) % MenuUpdateTicks
extern uint32_t framecount, frameUpStart, frameDownStart, frameLeftStart, frameRightStart;
extern bool debugMode;
extern int8_t skin;
extern const uint8_t *IMGBackground, *IMGBomb, *IMGBox, * IMGDiamond,* IMGExplosion,
			    *IMGFloor, *IMGInstructions, *IMGIntro1, *IMGIntro2,
			    *IMGPlayer,	*IMGPlayer2, *IMGTitleScreen, *IMGWall;
#endif