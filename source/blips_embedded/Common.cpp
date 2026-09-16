#include <stdint.h>
#include "SPoint.h"
#include "Defines.h"
#include "Common.h"
#include "CLevelPackFile.h"

uint16_t ColorWhite, ColorBlack;
GameStates GameState = GSTitleScreenInit;
uint8_t InstalledLevelPacksCount = 0, InstalledLevels = 0, UnlockedLevels = 1;
int8_t SelectedLevel = 0, SelectedLevelPack = 0, AskingQuestionID = -1;
bool AskingQuestion=false;
CWorldParts *WorldParts;
CLevelPackFile* LevelPackFile;
//the packs are built in, so their names stay in flash instead of being copied into ram
const char* const InstalledLevelPacks[MaxLevelPacks] = {"bips.bip", "bips_gold.bip", "bips_platinum.bip", "bips_gold_2_players.bip"};
const char* LevelPackName = InstalledLevelPacks[0];
uint32_t framecount = 0, frameUpStart = 0, frameDownStart = 0, frameLeftStart = 0, frameRightStart = 0;
uint8_t currButtons, prevButtons;
uint8_t Selection = 0;
bool debugMode = false;
const uint8_t *IMGBackground, *IMGBomb, *IMGBox, * IMGDiamond,* IMGExplosion,
			  *IMGFloor, *IMGInstructions, *IMGIntro1, *IMGIntro2,
			  *IMGPlayer,	*IMGPlayer2, *IMGTitleScreen, *IMGWall;
int8_t skin = 0;