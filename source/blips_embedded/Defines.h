#ifndef DEFINES_H
#define DEFINES_H

//the device comes first: the display library, SCREENBUFFER and IMAGESET are device settings, see
//PlatformESPboy.h / PlatformSDL.h
#include "PlatformDevice.h"

#define WINDOW_WIDTH 128
#define WINDOW_HEIGHT 128
#define HALFWINDOWWIDTH 64
#define HALFWINDOWHEIGHT 64
#define FPS 30
//1 = every frame waits until 1/FPS of a second has passed, 0 = a frame starts as soon
//as the last one is done, to see how fast the game can go. Movement, animation, input
//repeat and music all count frames, so without the lock they run faster as well.
//A build can set it itself
#ifndef FPSLOCK
#define FPSLOCK 1
#endif
//1 = the debug header (frame rate, free heap and stack) is always shown, Up + Down does not
//hide it. 0 = it starts hidden and Up + Down shows and hides it. A build can set it itself
#ifndef FORCEDEBUG
#define FORCEDEBUG 0
#endif

//1 = the floor tiles the player can reach are found with a floodfill every frame and
//drawn under the level, 0 = no floor is drawn and the floodfill, its bitmaps and its tile
//stack are left out of the build. A build can set it itself
#ifndef FLOODFILLFLOOR
#define FLOODFILLFLOOR 1
#endif
//GameMoveSpeed, PlayerAnimDelay and ViewportMove depend on the tile size, see IMAGESET below
#define ExplosionAnimDelay 1			//frames per explosion anim phase, inc if fps increases

#define MAXSKINS 2

//image headers to build with: 1 = images (16x16 tiles), 2 = images2 (same images at half size, 8x8 tiles)
//set by the device header (PlatformESPboy.h / PlatformSDL.h) or by the build
#ifndef IMAGESET
#error "the device header has to define IMAGESET"
#endif

//move speed is in pixels per frame, so smaller tiles need a lower speed to keep the
//same pace: 16 / 4 and 8 / 2 both take 4 frames per tile. That is also why the anim
//delay (frames per anim phase) is the same for both, the walk cycle advances at the
//same rate per tile either way. The tile size has to be a multiple of the move speed,
//a part only stops moving once it lands exactly on its tile. ViewportMove is how many
//pixels a frame the view pans when looking around the level
#if IMAGESET == 2
#define IMAGES_DIR images2
#define TileWidth 8
#define TileHeight 8
#define GameMoveSpeed 2					//dec if fps increases
#define PlayerAnimDelay 3				//inc if fps increases
#define ViewportMove 2					//dec if fps increases
#else
#define IMAGES_DIR images
#define TileWidth 16
#define TileHeight 16
#define GameMoveSpeed 4					//dec if fps increases
#define PlayerAnimDelay 3				//inc if fps increases
#define ViewportMove 4					//dec if fps increases
#endif

//path of an image header in the selected folder: #include GAME_IMAGE(box_RGB565_LE.h)
#define GAME_IMAGE_STR(x) #x
#define GAME_IMAGE_PATH(dir, file) GAME_IMAGE_STR(dir/file)
#define GAME_IMAGE(file) GAME_IMAGE_PATH(IMAGES_DIR, file)

#define NrOfRows 16
#define NrOfCols 25
#define NrOfColsVisible (WINDOW_WIDTH / TileWidth)
#define NrOfRowsVisible ((WINDOW_HEIGHT / TileHeight))

#define IDEmpty 1
#define IDPlayer 2
#define IDBox 3
#define IDFloor 4
#define IDBomb 5
#define IDWall 6
#define IDDiamond 7
#define IDPlayer2 8
#define IDBox1 9
#define IDBox2 10
#define IDBoxBomb 11
#define IDBoxWall 12
#define IDWallBreakable 13
#define IDExplosion 14

#define ZPlayer 10
#define ZDiamond 3
#define ZWall 4
#define ZBox 6
#define ZBomb 3
#define ZFloor 1
#define ZBoxBomb 5
#define ZExplosion 15

//a level can hold at most one part per playfield cell, the spare slots are for the
//explosions that get added while parts are still being removed
//A level can hold one part per playfield tile: that is also where the level
//parser stops accepting them, see MAXITEMCOUNT. Do not size this from the parts
//the current packs happen to use, a level is allowed to fill the whole grid.
//The spare slots cover the explosions that get added while the parts they
//destroy are still waiting to be removed.
#define MAXWORLDPARTS ((NrOfCols * NrOfRows) + 48)
//only the parts that actually moved this frame get redrawn on top, that is the
//active player plus whatever it is pushing
#define MAXMOVEABLEWORLDPARTS 8

#define MaxLevelPacks 4

#define MaxLevelPackNameLength 25

#define IDSolvedLevelNextUnlocked 2
#define IDSolvedLastLevel 3
#define IDSolvedEarlierLevel 4
#define IDNoPlayer 5
#define IDNoDiamonds 6
#define IDNoLevelsInPack 7
#define IDCurrentLevelNotSaved 8
#define IDRestartLevel 9
#define IDDeleteAllParts 10
#define IDDeleteLevelPack 11
#define IDDeleteLevel 12
#define IDLevelNotUnlocked 13
#define IDPlayerDied 15
#define IDBipExported 16
#define IDBipRefuseExport 17
#define IDMaxLevelsLevelPack 18
#define IDQuitPlaying 19

#define MenuUpdateTicks 10
//frames between repeats while a direction is held in the level selector
#define LevelSelectUpdateTicks 5
//the only skin a 1 bpp buffer can show
#define SKINBLACKWHITE 1

//FORCESKIN: -1 = every skin is built in and can be picked in the options, n = only skin n
//(0 default, 1 black & white) is built in and always used, which saves the flash of the others on a
//small device. A 1 bpp buffer can only show the black & white skin, so it forces that one.
//Set by the device header or the build
#if SCREENBUFFER == 1
  #if defined(FORCESKIN) && (FORCESKIN >= 0) && (FORCESKIN != SKINBLACKWHITE)
  #error "a 1 bpp buffer can only show the black & white skin, FORCESKIN has to be -1 or 1"
  #endif
  #undef FORCESKIN
  #define FORCESKIN SKINBLACKWHITE
#elif !defined(FORCESKIN)
  #define FORCESKIN -1
#endif
//1 when the images of skin n are part of the build
#define SKINBUILT(n) ((FORCESKIN < 0) || (FORCESKIN == (n)))
#endif