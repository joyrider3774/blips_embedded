#ifndef DEFINES_H
#define DEFINES_H

//the device comes first: the display library, SCREENBUFFER and IMAGESET are device settings, see
//PlatformESPboy.h / PlatformSDL.h
#include "PlatformDevice.h"

//1 = the art is read from a card while the game runs and none of it is in flash, see
//cardimages.h. It needs a device that can read one (PLATFORM_HAS_CARD in Platform.h) and the card
//file tools/mkcard.py writes. Every skin is then on the card in full RGB565 and the game can be
//asked for any of them, which is what flash could never hold: the whole reason only one reduced
//skin is built in is the 50944 bytes a device has for everything
#ifndef CARDIMAGES
#define CARDIMAGES 0
#endif

//How much RAM a card build keeps its art in. A picture small enough to be worth it is read once
//and kept here, so drawing it again is a copy; a full screen one is read a row or a strip at a
//time and never kept. A screen whose pictures do not all fit still draws correctly, it just reads
//them again, which CardImages_Reads() counts. See the arena in cardimages.cpp
#ifndef CARDARENA
#define CARDARENA 5120
#endif

//1 = the full screen background is drawn as one plain colour (ColorWhite, which LoadGraphics
//sets per skin) instead of as a picture. Every row of this game's background is already one
//colour - a gradient down the screen in the default skin, plain white in the black & white one -
//so what is lost is the gradient and nothing else.
//It is for a card build: the background is 32768 bytes, it is not small enough to keep in the
//arena, and a scrolling board redraws every strip of the screen, so it was 32 KB read off the
//card every frame and more than everything else put together. A device sets this itself, see
//PlatformCHGame.h; a skin whose background is a real picture must leave it off
#ifndef FLATBACKGROUND
#define FLATBACKGROUND 0
#endif

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
//1 = the colours of an image are spread over the ones the buffer can hold, so that a shade it
//has no colour for is a pattern of the two it does instead of the nearer of them. 0 = every
//colour becomes the nearest one there is, which shows as bands across anything that shades.
//An 8 bpp buffer is RGB332 and drops 2 bits of red, 3 of green and 3 of blue, and a 1 bpp buffer
//keeps only black and white, so both have something to spread. A 16 bpp buffer holds every colour
//of the image as it is and is left alone. A build can set this itself, see DitherSpread in
//Platform.h
#ifndef DITHERING
#define DITHERING 0
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
//parser stops accepting them, see MAXITEMCOUNT, so a pool of a full grid takes any level
//that could ever be written.
//The spare slots cover the explosions that get added while the parts they
//destroy are still waiting to be removed.
//The device header may ask for fewer, which a device with little ram has to: the pool is
//the largest thing the game asks the heap for. One that does may size itself from
//LEVELPACKMAXPARTS, the busiest level of the packs it actually ships, which is checked
//below where that is known
#ifndef MAXWORLDPARTS
#define MAXWORLDPARTS ((NrOfCols * NrOfRows) + 48)
#endif
//only the parts that actually moved this frame get redrawn on top, that is the
//active player plus whatever it is pushing
#define MAXMOVEABLEWORLDPARTS 8

//>>> written by tools/convert_levelpacks.py from assets/levelpacks, do not edit by hand
//LEVELPACKS: the level packs that are built in, an LP_ bit each (the size is what the
//pack takes in flash). All of them unless the device header or the build picks fewer; a
//pack that is left out takes no flash and is not offered in the game
#define LP_bips_1              (1ul <<  0)    //bips_1.bip                5937 bytes
#define LP_bips_2              (1ul <<  1)    //bips_2.bip                6882 bytes
#define LP_bips_gold           (1ul <<  2)    //bips_gold.bip             4588 bytes
#define LP_bips_gold_2_players (1ul <<  3)    //bips_gold_2_players.bip   4608 bytes
#define LP_bips_platinum_1     (1ul <<  4)    //bips_platinum_1.bip       5921 bytes
#define LP_bips_platinum_2     (1ul <<  5)    //bips_platinum_2.bip       6400 bytes
#define LP_ALL ((1ul << 6) - 1)
#ifndef LEVELPACKS
#define LEVELPACKS LP_ALL
#endif
#if (LEVELPACKS & LP_ALL) == 0
#error "LEVELPACKS has to leave at least one level pack in"
#endif
//how many packs there are in all, which is what the saved unlocks are sized by
#define MaxLevelPacks 6
//how many of them this build takes, which is how many the game lists
#define LEVELPACKCOUNT (((LEVELPACKS & LP_bips_1) != 0) + ((LEVELPACKS & LP_bips_2) != 0) + ((LEVELPACKS & LP_bips_gold) != 0) + ((LEVELPACKS & LP_bips_gold_2_players) != 0) + ((LEVELPACKS & LP_bips_platinum_1) != 0) + ((LEVELPACKS & LP_bips_platinum_2) != 0))

//The busiest level each pack has. The game keeps a pool of world parts and it is the
//largest thing it asks the heap for, so a build wants no more slots than the packs it
//holds can fill, see MAXWORLDPARTS in the device header
#define LP_PARTS_bips_1               346
#define LP_PARTS_bips_2               342
#define LP_PARTS_bips_gold            397
#define LP_PARTS_bips_gold_2_players  336
#define LP_PARTS_bips_platinum_1      388
#define LP_PARTS_bips_platinum_2      396

//how many parts the busiest level of the packs this build holds has. A pack that is
//left out counts for nothing, so the count follows what LEVELPACKS says
//One comparison a pack. LP_PARTS_MAX is a function and not a macro on purpose: a macro
//naming its first argument twice doubles the text at every step, which with nineteen
//packs put the compiler out of memory. constexpr keeps it usable where a constant is
//wanted, such as the static_assert below and the size of the pool
static inline constexpr int LP_PARTS_MAX(int a, int b) { return (a > b) ? a : b; }
#define LP_PARTS_OF(p) (((LEVELPACKS & LP_##p) != 0) ? LP_PARTS_##p : 0)
#define LEVELPACKMAXPARTS LP_PARTS_MAX(LP_PARTS_MAX(LP_PARTS_MAX(LP_PARTS_MAX(LP_PARTS_MAX(LP_PARTS_MAX(0, LP_PARTS_OF(bips_1)), LP_PARTS_OF(bips_2)), LP_PARTS_OF(bips_gold)), LP_PARTS_OF(bips_gold_2_players)), LP_PARTS_OF(bips_platinum_1)), LP_PARTS_OF(bips_platinum_2))
//<<<

//The pool has to take the busiest level of the packs this build ships. A build that takes
//every level of every pack wants a full grid, which is what MAXWORLDPARTS is by default
static_assert(MAXWORLDPARTS >= LEVELPACKMAXPARTS, "a level of a pack in this build would not fit the pool");

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
//small device. A 1 bpp buffer has only two colours to show, so the
//black & white skin is the one it takes on its own. A build can still ask it for another one,
//whose shades then go through the brightness rule in SetBufferBit, and with DITHERING come out
//as a pattern of the two colours rather than as the nearer of them.
//Set by the device header or the build
//A card build names no skin either: every one of them is on the card in full RGB565 and the
//game is asked for one while it runs, see CardImages_UseSkin
#if !defined(FORCESKIN)
  #if (SCREENBUFFER == 1) && !CARDIMAGES
  #define FORCESKIN SKINBLACKWHITE
  #else
  #define FORCESKIN -1
  #endif
#endif
//1 when the images of skin n are part of the build
#define SKINBUILT(n) ((FORCESKIN < 0) || (FORCESKIN == (n)))

//1 when the black & white skin is in the build, whose pictures are packed one bit a pixel
//by tools/onebit.py and drawn by the routines in onebitimage.cpp rather than as RGB565. It
//shows two colours, and keeping each of them in sixteen bits costs both flash and the work
//of writing a colour per pixel. Every skin can be in the build here and picked in the
//options, so which kind a picture is cannot be known at build time: skinImagesOneBit says
//A card build has none of them: every skin is on the card in full RGB565, so there is no
//reduced form to read and nothing of the one bit paths is built
#define ONEBITIMAGES (!CARDIMAGES && SKINBUILT(SKINBLACKWHITE))

//1 when the black & white skin is the only one in the build. Every picture is then one bit a pixel
//and the paths that read RGB565 are dead: a build that is only ever going to draw one bit pictures
//need not carry the index the run length encoded background is read through, which is a row table
//the width of the screen
#define ONEBITONLY (ONEBITIMAGES && (FORCESKIN == SKINBLACKWHITE))
#endif