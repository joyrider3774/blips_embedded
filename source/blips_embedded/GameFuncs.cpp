#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "Common.h"
#include "GameFuncs.h"
//the one bit pictures of the black & white skin, which the band renderer reads too
#include "onebitimage.h"
#include "Sound.h"
//the art read off a card, for a build with CARDIMAGES on
#include "cardimages.h"
//A card build has no art in flash at all: every skin is on the card in full RGB565 and the
//pictures come off it while the game runs, see cardimages.h. So none of the generated headers
//are part of it
#if !CARDIMAGES
//only the skins FORCESKIN leaves in are part of the build (a 1 bpp buffer forces the black & white one)
#if SKINBUILT(0)
#include GAME_IMAGE(default/background_RLE565.h)
#include GAME_IMAGE(default/bomb_RGB565_LE.h)
#include GAME_IMAGE(default/box_RGB565_LE.h)
#include GAME_IMAGE(default/diamond_RGB565_LE.h)
#include GAME_IMAGE(default/explosion_RGB565_LE.h)
#include GAME_IMAGE(default/floor_RGB565_LE.h)
#include GAME_IMAGE(default/instructions_RLE565.h)
#include GAME_IMAGE(default/intro1_RLE565.h)
#include GAME_IMAGE(default/intro2_RLE565.h)
#include GAME_IMAGE(default/player_RGB565_LE.h)
#include GAME_IMAGE(default/player2_RGB565_LE.h)
#include GAME_IMAGE(default/titlescreen_RLE565.h)
#include GAME_IMAGE(default/wall_RGB565_LE.h)
#endif

#if SKINBUILT(SKINBLACKWHITE)
#include GAME_IMAGE(black_white/background_RLE565.h)
#include GAME_IMAGE(black_white/bomb_RGB565_LE.h)
#include GAME_IMAGE(black_white/box_RGB565_LE.h)
#include GAME_IMAGE(black_white/diamond_RGB565_LE.h)
#include GAME_IMAGE(black_white/explosion_RGB565_LE.h)
#include GAME_IMAGE(black_white/floor_RGB565_LE.h)
#include GAME_IMAGE(black_white/instructions_RLE565.h)
#include GAME_IMAGE(black_white/intro1_RLE565.h)
#include GAME_IMAGE(black_white/intro2_RLE565.h)
#include GAME_IMAGE(black_white/player_RGB565_LE.h)
#include GAME_IMAGE(black_white/player2_RGB565_LE.h)
#include GAME_IMAGE(black_white/titlescreen_RLE565.h)
#include GAME_IMAGE(black_white/wall_RGB565_LE.h)
#endif
#endif

//the skin in use: the one FORCESKIN builds in (a 1 bpp buffer forces the black & white one), or
//the one picked in the options
uint8_t CurrentSkin(void)
{
#if FORCESKIN >= 0
	return FORCESKIN;
#else
	return skin;
#endif
}

#if SCREENBUFFER
//The board is drawn again every frame with a buffer, tens of thousands of pixels, so
//this is kept lean: the image is clipped once, every visible row comes out of flash in
//one copy and is written through a row pointer, nothing is worked out per pixel
void DrawImageToBuffer(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t* image, bool transparent)
{
	if (!image)
		return;
	int16_t c0 = (x < 0) ? -x : 0;
	int16_t c1 = (x + w > WINDOW_WIDTH) ? WINDOW_WIDTH - x : w;
	int16_t r0 = (y < 0) ? -y : 0;
	int16_t r1 = (y + h > WINDOW_HEIGHT) ? WINDOW_HEIGHT - y : h;
	if ((c0 >= c1) || (r0 >= r1))
		return;
	void* dst = SCREENBUFFER_PIXELS();
	if (!dst)
		return;
	const int16_t cols = c1 - c0;
	const int16_t sx = x + c0;
	//little endian RGB565 like both devices, so the bytes can be copied straight into it
	uint16_t row[WINDOW_WIDTH];
	for (int16_t r = r0; r < r1; r++)
	{
		const int16_t sy = y + r;
		PLATFORM_READ_BYTES((uint8_t*)row, image + (r * w + c0) * sizeof(uint16_t), cols * sizeof(uint16_t));
  #if SCREENBUFFER == 16
		uint16_t* d = &((uint16_t*)dst)[sy * WINDOW_WIDTH + sx];
		for (int16_t c = 0; c < cols; c++)
		{
			uint16_t color = row[c];
			//magenta is the transparent key
			if (!transparent || (color != 0xF81F))
				d[c] = (uint16_t)((color >> 8) | (color << 8));
		}
  #elif SCREENBUFFER == 8
		uint8_t* d = &((uint8_t*)dst)[sy * WINDOW_WIDTH + sx];
		for (int16_t c = 0; c < cols; c++)
		{
			uint16_t color = row[c];
			if (!transparent || (color != 0xF81F))
				d[c] = ToBuffer332(color, (int16_t)(sx + c), (int16_t)sy);
		}
  #else
		for (int16_t c = 0; c < cols; c++)
		{
			uint16_t color = row[c];
			if (!transparent || (color != 0xF81F))
				SetBufferBit((uint8_t*)dst, sx + c, sy, color);
		}
  #endif
	}
}
#endif

#if CARDIMAGES
//A picture from the card: the w by h part at sx,sy of it, at x,y on the screen.
//Everything the game draws goes through here in a card build. There is no pointer into flash to
//step, so a frame of a sprite sheet is named by the row it starts at (sy) rather than by a
//pointer that has already been moved, see CWorldPart_SpriteData
void DrawImageCardPart(int16_t x, int16_t y, int16_t sx, int16_t sy, int16_t w, int16_t h,
                       const uint8_t* image, bool transparent)
{
	if (!image || (w <= 0) || (h <= 0))
		return;
	const int16_t c0 = (x < 0) ? -x : 0;
	const int16_t c1 = (x + w > WINDOW_WIDTH) ? WINDOW_WIDTH - x : w;
	const int16_t r0 = (y < 0) ? -y : 0;
	const int16_t r1 = (y + h > WINDOW_HEIGHT) ? WINDOW_HEIGHT - y : h;
	if ((c0 >= c1) || (r0 >= r1))
		return;
	const int16_t cols = c1 - c0;
	const int16_t dx = x + c0;
	//little endian RGB565, the form the card holds and the display takes
	uint16_t row[WINDOW_WIDTH];
#if SCREENBUFFER
	void* dst = SCREENBUFFER_PIXELS();
	if (!dst)
		return;
	for (int16_t r = r0; r < r1; r++)
	{
		const int16_t dy = y + r;
		//a row that did not come leaves the buffer as it was rather than drawing what
		//happens to be in the scratch
		if (!CardImages_Row(image, sx + c0, sy + r, cols, row))
			continue;
  #if SCREENBUFFER == 16
		uint16_t* d = &((uint16_t*)dst)[dy * WINDOW_WIDTH + dx];
		for (int16_t c = 0; c < cols; c++)
			//magenta is the transparent key
			if (!transparent || (row[c] != 0xF81F))
				d[c] = (uint16_t)((row[c] >> 8) | (row[c] << 8));
  #elif SCREENBUFFER == 8
		uint8_t* d = &((uint8_t*)dst)[dy * WINDOW_WIDTH + dx];
		for (int16_t c = 0; c < cols; c++)
			if (!transparent || (row[c] != 0xF81F))
				d[c] = ToBuffer332(row[c], (int16_t)(dx + c), dy);
  #else
		for (int16_t c = 0; c < cols; c++)
			if (!transparent || (row[c] != 0xF81F))
				SetBufferBit((uint8_t*)dst, dx + c, dy, row[c]);
  #endif
	}
#else
	//THE BUS RULE: where the card shares the display's bus, reading it takes the bus over - the
	//panel is deselected and the SPI set up for the card, see Platform_CardRead. So a row is
	//fetched with nothing of the display's open and only then sent, each row in a window of its
	//own. A window around the whole part cannot survive a card read in the middle of filling it
	for (int16_t r = r0; r < r1; r++)
	{
		if (!CardImages_Row(image, sx + c0, sy + r, cols, row))
			continue;
		int16_t c = 0;
		while (c < cols)
		{
			//every run of opaque pixels goes out as one, what is left out keeps what is shown
			if (transparent)
				while ((c < cols) && (row[c] == 0xF81F))
					c++;
			const int16_t runX = c;
			while ((c < cols) && (!transparent || (row[c] != 0xF81F)))
				c++;
			if (c == runX)
				continue;
			SCREEN.startWrite();
  #if LOVYANGFX
			SCREEN.setAddrWindow(dx + runX, y + r, c - runX, 1);
			//true: the values are plain RGB565, the library puts them in display order
			SCREEN.writePixels(row + runX, c - runX, true);
  #else
			GFX.pushImage(dx + runX, y + r, c - runX, 1, row + runX);
  #endif
			SCREEN.endWrite();
		}
	}
#endif
}
#endif

//Draws an opaque RGB565 image. TFT_eSprite's 1 bpp pushImage expects 1 bpp image data,
//so there the pixels are converted and written into the buffer directly. LovyanGFX
//reads image data through plain pointers, but PROGMEM on the ESP8266 is flash that
//only takes 32 bit reads, so with that library every image is read here with
//PLATFORM_READ_BYTES and handed over as pixels
void DrawImage(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t* image)
{
	if (!image)
		return;
#if CARDIMAGES
	//the picture is on the card and carries its own size, so it is drawn whole from its first row
	DrawImageCardPart(x, y, 0, 0, w, h, image, false);
	return;
#endif
#if ONEBITIMAGES
	if (skinImagesOneBit)
	{
		//the picture carries its own size and is drawn whole
		drawImageOneBitPart(x, y, 0, 0, w, h, image, false);
		return;
	}
#endif
#if (SCREENBUFFER == 1) || (SCREENBUFFER && LOVYANGFX)
	DrawImageToBuffer(x, y, w, h, image, false);
#elif LOVYANGFX
	//straight to the display: only the part that is on screen, a row at a time
	int16_t c0 = (x < 0) ? -x : 0;
	int16_t c1 = (x + w > WINDOW_WIDTH) ? WINDOW_WIDTH - x : w;
	int16_t r0 = (y < 0) ? -y : 0;
	int16_t r1 = (y + h > WINDOW_HEIGHT) ? WINDOW_HEIGHT - y : h;
	if ((c0 >= c1) || (r0 >= r1))
		return;
	uint16_t line[WINDOW_WIDTH];
	SCREEN.startWrite();
	SCREEN.setAddrWindow(x + c0, y + r0, c1 - c0, r1 - r0);
	for (int16_t r = r0; r < r1; r++)
	{
		const uint8_t* src = image + (r * w + c0) * sizeof(uint16_t);
#if PLATFORM_DIRECT_FLASH
		//flash is plain memory here, so an evenly placed row goes to the display where it
		//lies instead of being copied first. A 16 bit read needs an even address, a core
		//like the Cortex-M0+ faults on an odd one
		if (((uintptr_t)src & 1) == 0)
		{
			SCREEN.writePixels((const uint16_t*)src, c1 - c0, true);
			continue;
		}
#endif
		//the visible part of the row in one copy out of flash
		PLATFORM_READ_BYTES((uint8_t*)line, src, (c1 - c0) * sizeof(uint16_t));
		//true: the values are plain RGB565, the library puts them in display order
		SCREEN.writePixels(line, c1 - c0, true);
	}
	SCREEN.endWrite();
#else
	GFX.pushImage(x, y, w, h, (const uint16_t*)image);
#endif
}

//Draws one of the magenta keyed sprites. The display can do the keying itself, but
//TFT_eSprite has no pushImage that skips a transparent colour (and its 1 bpp pushImage
//expects 1 bpp image data), so with a buffer the pixels are written into it directly,
//in whatever form that buffer keeps them. LovyanGFX can not read PROGMEM images on the
//ESP8266 (see DrawImage), so there even the display gets its pixels from here.
void DrawImageTransparent(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t* image)
{
	if (!image)
		return;
#if CARDIMAGES
	DrawImageCardPart(x, y, 0, 0, w, h, image, true);
	return;
#endif
#if ONEBITIMAGES
	if (skinImagesOneBit)
	{
		//the picture carries its own size and is drawn whole
		drawImageOneBitPart(x, y, 0, 0, w, h, image, true);
		return;
	}
#endif
	const uint16_t* src = (const uint16_t*)image;
#if (SCREENBUFFER == 0) && !LOVYANGFX
	GFX.pushImage(x, y, w, h, src, 0xF81F);
#elif SCREENBUFFER == 0
	//straight to the display, every run of opaque pixels on a row goes out as one
	uint16_t line[WINDOW_WIDTH];
	SCREEN.startWrite();
	for (int16_t r = 0; r < h; r++)
	{
		int16_t sy = y + r;
		if ((sy < 0) || (sy >= WINDOW_HEIGHT))
			continue;
		const uint16_t* srow = &src[r * w];
#if PLATFORM_DIRECT_FLASH
		//flash is plain memory here: an evenly placed row is read where it lies, a 16 bit
		//read of an odd address faults on a core like the Cortex-M0+
		const bool direct = (((uintptr_t)srow & 1) == 0);
#endif
		int16_t runX = 0, runLen = 0;
		for (int16_t c = 0; c <= w; c++)
		{
			int16_t sx = x + c;
			uint16_t color = 0xF81F;
			if ((c < w) && (sx >= 0) && (sx < WINDOW_WIDTH))
#if PLATFORM_DIRECT_FLASH
				color = direct ? srow[c] : PLATFORM_READ_PIXEL(&srow[c]);
#else
				color = PLATFORM_READ_PIXEL(&srow[c]);
#endif
			//magenta is the transparent key, it (and the end of the row) closes a run
			if (color != 0xF81F)
			{
				if (runLen == 0)
					runX = sx;
				line[runLen++] = color;
			}
			else if (runLen > 0)
			{
				SCREEN.setAddrWindow(runX, sy, runLen, 1);
				SCREEN.writePixels(line, runLen, true);
				runLen = 0;
			}
		}
	}
	SCREEN.endWrite();
#else
	(void)src;
	DrawImageToBuffer(x, y, w, h, image, true);
#endif
}

//draws a run length encoded RGB565 image made by tools/png2rle565.py straight from
//flash, the raw full screen images of all skins do not fit in the flash that can be
//mapped for code. A control byte with the top bit set is a run of (c & 0x7F) + 1 times
//the pixel after it, otherwise c + 1 literal pixels follow. No clipping is done.
//The data is read here with PLATFORM_READ_BYTE and PLATFORM_READ_BYTES: LovyanGFX reads
//image data through plain pointers, but PROGMEM on the ESP8266 is flash that only takes
//32 bit reads, so none of its image functions may be handed the data
//Draws one frame of a sprite sheet. The frames are stacked down a sheet one tile wide, so a
//RGB565 sheet has the frame picked out by CWorldPart_SpriteData stepping the pointer, while a one
//bit sheet is handed over whole and the frame is the part of it that starts at that row
void DrawSpriteFrame(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t* image, uint8_t frame)
{
	if (!image)
		return;
#if CARDIMAGES
	//the sheet is handed over whole and the frame is the part of it that starts at that row,
	//there being no pointer to step, see CWorldPart_SpriteData
	DrawImageCardPart(x, y, 0, (int16_t)(frame * h), w, h, image, true);
	return;
#endif
#if ONEBITIMAGES
	if (skinImagesOneBit)
	{
		drawImageOneBitPart(x, y, 0, frame * h, w, h, image, true);
		return;
	}
#endif
	DrawImageTransparent(x, y, w, h, image);
}

void pushImageRLE(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t* data)
{
#if CARDIMAGES
	//nothing on the card is encoded: every picture is there in full, so the full screen ones are
	//drawn like any other. Encoding exists to fit them in flash, which a card build does not do
	DrawImageCardPart(x, y, 0, 0, w, h, data, false);
	return;
#endif
#if ONEBITIMAGES
	if (skinImagesOneBit)
	{
		drawImageOneBitPart(x, y, 0, 0, w, h, data, false);
		return;
	}
#endif
#if SCREENBUFFER
	//decoded straight into the buffer instead of streamed to the display. The pixels of a
	//control go in a row at a time: the part of the row that is on screen is worked out once,
	//literal pixels come out of flash in one copy and the colour of a run is converted once
	if ((w <= 0) || (h <= 0))
		return;
	void* dst = SCREENBUFFER_PIXELS();
	if (!dst)
		return;
	//a control covers at most 128 pixels
	uint16_t pixels[128];
	uint32_t left = (uint32_t)w * h;
	int16_t cx = 0, cy = 0;
	while (left > 0)
	{
		uint8_t control = PLATFORM_READ_BYTE(data++);
		uint16_t count = (control & 0x7F) + 1;
		if (count > left)
			count = (uint16_t)left;
		bool run = (control & 0x80) != 0;
		uint16_t color = 0;
		if (run)
		{
			color = PLATFORM_READ_BYTE(data) | (PLATFORM_READ_BYTE(data + 1) << 8);
			data += 2;
		}
		else
		{
			//little endian RGB565 like both devices, so the bytes can be copied straight in
			PLATFORM_READ_BYTES((uint8_t*)pixels, data, count * sizeof(uint16_t));
			data += count * sizeof(uint16_t);
		}
		left -= count;
  #if SCREENBUFFER == 16
		//a 16 bpp sprite keeps its pixels byte swapped
		const uint16_t value = (uint16_t)((color >> 8) | (color << 8));
  #elif SCREENBUFFER == 8
    #if !DITHERING
		//RGB332, the same conversion both libraries apply to everything else. With DITHERING
		//the byte depends on where the pixel lands, so a run is worked out pixel by pixel
		//instead and there is nothing to keep here
		const uint8_t value = ToBuffer332(color, 0, 0);
    #endif
  #endif
		for (uint16_t done = 0; done < count; )
		{
			//the pixels of the control that are left on this row
			uint16_t n = w - cx;
			if (n > count - done)
				n = count - done;
			const int16_t sx = x + cx, sy = y + cy;
			//the columns of those that are on screen
			const int16_t c0 = (sx < 0) ? -sx : 0;
			const int16_t c1 = (sx + n > WINDOW_WIDTH) ? WINDOW_WIDTH - sx : n;
			if ((sy >= 0) && (sy < WINDOW_HEIGHT) && (c0 < c1))
			{
				const uint16_t* src = &pixels[done];
  #if SCREENBUFFER == 16
				uint16_t* d = &((uint16_t*)dst)[sy * WINDOW_WIDTH];
				if (run)
					for (int16_t c = c0; c < c1; c++)
						d[sx + c] = value;
				else
					for (int16_t c = c0; c < c1; c++)
						d[sx + c] = (uint16_t)((src[c] >> 8) | (src[c] << 8));
  #elif SCREENBUFFER == 8
				uint8_t* d = &((uint8_t*)dst)[sy * WINDOW_WIDTH];
    #if DITHERING
				//every pixel of a run falls in its own place in the pattern, so a run can not be
				//spread with one memset the way it is without dithering
				for (int16_t c = c0; c < c1; c++)
					d[sx + c] = ToBuffer332(run ? color : src[c], (int16_t)(sx + c), (int16_t)sy);
    #else
				if (run)
					memset(&d[sx + c0], value, c1 - c0);
				else
					for (int16_t c = c0; c < c1; c++)
						d[sx + c] = ToBuffer332(src[c], (int16_t)(sx + c), (int16_t)sy);
    #endif
  #else
				for (int16_t c = c0; c < c1; c++)
					SetBufferBit((uint8_t*)dst, sx + c, sy, run ? color : src[c]);
  #endif
			}
			done += n;
			cx += n;
			if (cx == w)
			{
				cx = 0;
				cy++;
			}
		}
	}
#else
	uint16_t buffer[128];
	uint32_t left = (uint32_t)w * h;
	//the chip select sits on the I/O expander, so the whole image goes out in one
	//transaction. LovyanGFX's pushBlock and pushPixels open one of their own per call,
	//there the write variants are used inside this one
	SCREEN.startWrite();
	SCREEN.setAddrWindow(x, y, w, h);
	while (left > 0)
	{
		uint8_t control = PLATFORM_READ_BYTE(data++);
		uint32_t count = (control & 0x7F) + 1;
		if (count > left)
			count = left;
		if (control & 0x80)
		{
			uint16_t color = PLATFORM_READ_BYTE(data) | (PLATFORM_READ_BYTE(data + 1) << 8);
#if LOVYANGFX
			//a uint16_t colour is taken as plain RGB565
			SCREEN.writeColor(color, count);
#else
			SCREEN.pushBlock(color, count);
#endif
			data += 2;
		}
		else
		{
			//all literal pixels in one copy, little endian RGB565 like both devices
			PLATFORM_READ_BYTES((uint8_t*)buffer, data, count * sizeof(uint16_t));
			data += count * sizeof(uint16_t);
#if LOVYANGFX
			//true: the values are plain RGB565, the library puts them in display order
			SCREEN.writePixels(buffer, count, true);
#else
			SCREEN.pushPixels(buffer, count);
#endif
		}
		left -= count;
	}
	SCREEN.endWrite();
#endif
}

//Without a screen buffer the strips leave out what an opaque sprite paints over anyway, and
//a sprite is only opaque when no pixel of it carries the transparent key. That is a property
//of the skin, so it is worked out here rather than assumed: a skin with, say, a wall that
//has see through parts simply does not get the shortcut
bool IMGBoxOpaque = false, IMGWallOpaque = false, IMGFloorOpaque = false;

//1 while the skin in use keeps its pictures one bit a pixel
bool skinImagesOneBit = false;

static bool ImageOpaque(const uint8_t* image, size_t bytes)
{
	if (!image)
		return false;
#if ONEBITIMAGES
	//a one bit picture carries a mask only when it has something to skip, which is
	//the whole of the question here
	if (skinImagesOneBit)
		return OneBitMaskAt(image) == 0;
#endif
	for (size_t i = 0; i < bytes; i += sizeof(uint16_t))
		//magenta is the transparent key, 0xF81F in RGB565
		if ((PLATFORM_READ_BYTE(image + i) | (PLATFORM_READ_BYTE(image + i + 1) << 8)) == 0xF81F)
			return false;
	return true;
}

//the three the strips can leave the background out for, of the skin being loaded
#define SKINOPAQUE(box, wall, floor) \
	IMGBoxOpaque = ImageOpaque((box), sizeof(box)); \
	IMGWallOpaque = ImageOpaque((wall), sizeof(wall)); \
	IMGFloorOpaque = ImageOpaque((floor), sizeof(floor))

void LoadGraphics(void)
{
	//the black & white skin keeps its pictures one bit a pixel, the others as RGB565
	skinImagesOneBit = ONEBITIMAGES && (CurrentSkin() == SKINBLACKWHITE);
#if CARDIMAGES
	//Every picture comes off the card, by the numbers tools/mkcard.py generated into
	//cardindex.h. The card holds every skin, so the one the game is on is chosen here first:
	//that rebuilds the descriptors and empties the arena, and the pointers below then come
	//from the new skin. This is what a skin change in the options runs, which is why it is
	//here and not only at startup.
	//The skins are on the card in the order the game numbers them, see cardindex.h
	CardImages_UseSkin(CurrentSkin());
#if FLATBACKGROUND
	//Drawn as one plain colour instead, which ColorWhite below is: NULL is what the drawing
	//already takes as "no picture, use the plain colour". See FLATBACKGROUND in defines.h
	IMGBackground = NULL;
#else
	IMGBackground = CardImages_Get(CARD_IMG_BACKGROUND);
#endif
	IMGBomb = CardImages_Get(CARD_IMG_BOMB);
	IMGBox = CardImages_Get(CARD_IMG_BOX);
	IMGDiamond = CardImages_Get(CARD_IMG_DIAMOND);
	IMGExplosion = CardImages_Get(CARD_IMG_EXPLOSION);
	IMGFloor = CardImages_Get(CARD_IMG_FLOOR);
	IMGInstructions = CardImages_Get(CARD_IMG_INSTRUCTIONS);
	IMGIntro1 = CardImages_Get(CARD_IMG_INTRO1);
	IMGIntro2 = CardImages_Get(CARD_IMG_INTRO2);
	IMGPlayer = CardImages_Get(CARD_IMG_PLAYER);
	IMGPlayer2 = CardImages_Get(CARD_IMG_PLAYER2);
	IMGTitleScreen = CardImages_Get(CARD_IMG_TITLESCREEN);
	IMGWall = CardImages_Get(CARD_IMG_WALL);
	//Worked out by reading the pictures, which off a card would be the whole sheet read for an
	//answer that only saves the strips some drawing. So the shortcut is simply not taken
	IMGBoxOpaque = IMGWallOpaque = IMGFloorOpaque = false;
	switch (CurrentSkin())
	{
		case CARD_SKIN_DEFAULT:
			ColorWhite = SCREEN.color565(132,155,189);
			ColorBlack = SCREEN.color565(33,75,123);
			break;
		case CARD_SKIN_BLACK_WHITE:
			ColorBlack = SCREEN.color565(0,0,0);
			ColorWhite = SCREEN.color565(255,255,255);
			break;
	}
	return;
#else
	switch (CurrentSkin())
	{
#if SKINBUILT(0)
		//Default
		case 0:
			IMGBackground = default_background_rle;
			IMGBomb = default_bomb_data;
			IMGBox = default_box_data;
			IMGDiamond = default_diamond_data;
			IMGExplosion = default_explosion_data;
			IMGFloor = default_floor_data;
			IMGInstructions = default_instructions_rle;
			IMGIntro1 = default_intro1_rle;
			IMGIntro2 = default_intro2_rle;
			IMGPlayer = default_player_data;
			IMGPlayer2 = default_player2_data;
			IMGTitleScreen = default_titlescreen_rle;
			IMGWall = default_wall_data;
			SKINOPAQUE(default_box_data, default_wall_data, default_floor_data);

			ColorWhite = SCREEN.color565(132,155,189);
			ColorBlack = SCREEN.color565(33,75,123);
			break;
#endif
#if SKINBUILT(SKINBLACKWHITE)
		//black white
		case 1:
			IMGBackground = black_white_background_rle;
			IMGBomb = black_white_bomb_data;
			IMGBox = black_white_box_data;
			IMGDiamond = black_white_diamond_data;
			IMGExplosion = black_white_explosion_data;
			IMGFloor = black_white_floor_data;
			IMGInstructions = black_white_instructions_rle;
			IMGIntro1 = black_white_intro1_rle;
			IMGIntro2 = black_white_intro2_rle;
			IMGPlayer = black_white_player_data;
			IMGPlayer2 = black_white_player2_data;
			IMGTitleScreen = black_white_titlescreen_rle;
			IMGWall = black_white_wall_data;
			SKINOPAQUE(black_white_box_data, black_white_wall_data, black_white_floor_data);
			ColorBlack = SCREEN.color565(0,0,0);
			ColorWhite = SCREEN.color565(255,255,255);			
			break;
#endif
	}
#endif
}

// ===========================================================================
// Saved data, kept in the platform's save storage
//
// Two records share the storage: the unlock progress and the settings.
// Each is stamped with a magic value and a version and ends in a CRC over
// everything before it, which is what tells a never written sector or a half
// finished write apart from real data. Anything that does not check out is
// reset rather than trusted.
// ===========================================================================

//2: a pack is remembered by a sum of its name and no longer by the name, so a record
//written by an older build is a different shape and is turned away by this
#define STORE_VERSION    2
#define UNLOCK_MAGIC     0x424C  //"BL"
#define SETTINGS_MAGIC   0x4253  //"BS"

#define SETTING_MUSIC    0x01
#define SETTING_SOUND    0x02

typedef struct UnlockRecord UnlockRecord;
struct UnlockRecord
{
	uint16_t magic;
	uint8_t  version;
	uint8_t  packCount;
	//what PackId() makes of each pack's name, in the order the packs claimed a slot
	uint16_t packId[MaxLevelPacks];
	uint16_t unlocked[MaxLevelPacks];
	uint16_t crc;   //covers every byte before it
};

typedef struct SettingsRecord SettingsRecord;
struct SettingsRecord
{
	uint16_t magic;
	uint8_t  version;
	uint8_t  flags;                          //SETTING_MUSIC, SETTING_SOUND
	int8_t  skin;
	//the compiler would leave this gap anyway, keeping what follows on an even offset; it is
	//named so that every byte the crc covers is one the records set
	uint8_t  pad;
	uint16_t packId;                         //the pack that was being played, see PackId()
	uint16_t crc;   //covers every byte before it
};

#define STORE_UNLOCK_ADDR    0
//The settings follow the unlock record instead of sitting at a fixed address: how long that
//record is depends on how many level packs there are, and splitting the two big packs made
//eight of four. It moved, so what was saved before no longer reads, which the magic and the
//crc of each record catch and treat as nothing having been saved
#define STORE_SETTINGS_ADDR  sizeof(UnlockRecord)
#define STORE_TOTAL          (STORE_SETTINGS_ADDR + sizeof(SettingsRecord))
//both records have to fit in what the platform stores
static_assert(STORE_TOTAL <= PLATFORM_STORAGE_SIZE, "the saved records do not fit in PLATFORM_STORAGE_SIZE");
//record addresses, lengths and the loops over them are uint8_t
static_assert(STORE_TOTAL <= 255, "store offsets do not fit in uint8_t");
//unlocked levels are kept as uint8_t
static_assert(MAXLEVELS <= 255, "unlocked levels do not fit in uint8_t");

//CRC16 CCITT, small and more than enough to spot a corrupted record
static uint16_t StoreCrc(const uint8_t* data, uint8_t len)
{
	uint16_t crc = 0xFFFF;
	while (len--)
	{
		crc ^= (uint16_t)(*data++) << 8;
		for (uint8_t bit = 0; bit < 8; bit++)
			crc = (crc & 0x8000) ? (uint16_t)((crc << 1) ^ 0x1021) : (uint16_t)(crc << 1);
	}
	return crc;
}

static void StoreRead(uint8_t addr, uint8_t* rec, uint8_t len)
{
	Platform_StorageRead(addr, rec, len);
}

//the crc sits in the last two bytes and covers everything ahead of it
static bool StoreCrcOk(const uint8_t* rec, uint8_t len)
{
	uint16_t stored;
	memcpy(&stored, rec + len - sizeof(uint16_t), sizeof(uint16_t));
	return StoreCrc(rec, len - sizeof(uint16_t)) == stored;
}

static void StoreWrite(uint8_t addr, uint8_t* rec, uint8_t len)
{
	uint16_t crc = StoreCrc(rec, len - sizeof(uint16_t));
	memcpy(rec + len - sizeof(uint16_t), &crc, sizeof(uint16_t));

	//an unchanged record costs no write at all
	Platform_StorageWrite(addr, rec, len);
}

// ---- unlock progress ------------------------------------------------------

static void UnlockReset(UnlockRecord* rec)
{
	memset(rec, 0, sizeof(UnlockRecord));
	rec->magic = UNLOCK_MAGIC;
	rec->version = STORE_VERSION;
	rec->packCount = 0;
}

//true when the storage held a record we can trust
static bool UnlockRead(UnlockRecord* rec)
{
	StoreRead(STORE_UNLOCK_ADDR, (uint8_t*)rec, sizeof(UnlockRecord));
	if (rec->magic != UNLOCK_MAGIC)
		return false;
	if (rec->version != STORE_VERSION)
		return false;
	if (rec->packCount > MaxLevelPacks)
		return false;
	return StoreCrcOk((const uint8_t*)rec, sizeof(UnlockRecord));
}

//index of a pack in the record, or -1 when it has never been played
//A pack is remembered by a sum of its name rather than by the name itself: the record has to tell
//one pack from another whatever order they are built in and whichever of them a build leaves out,
//and a sum does that in two bytes. Two packs that summed alike would share an unlock count, which
//is all a collision costs here
static uint16_t PackId(const char* name)
{
	if (!name)
		return 0;
	size_t len = strlen(name);
	if (len > 255)
		len = 255;
	return StoreCrc((const uint8_t*)name, (uint8_t)len);
}

static int8_t UnlockFindPack(const UnlockRecord* rec, const char* name)
{
	const uint16_t want = PackId(name);
	for (uint8_t Teller = 0; Teller < rec->packCount; Teller++)
		if (rec->packId[Teller] == want)
			return Teller;
	return -1;
}

void SaveUnlockData()
{
	UnlockRecord rec;
	if (!UnlockRead(&rec))
		UnlockReset(&rec);

	int8_t idx = UnlockFindPack(&rec, LevelPackName);
	if (idx < 0)
	{
		//first time this pack is finished with, claim a slot for it
		if (rec.packCount >= MaxLevelPacks)
			return;
		idx = rec.packCount++;
		rec.packId[idx] = PackId(LevelPackName);
	}

	if (rec.unlocked[idx] == (uint16_t)UnlockedLevels)
		return;   //nothing moved, leave the flash alone

	rec.unlocked[idx] = (uint16_t)UnlockedLevels;
	StoreWrite(STORE_UNLOCK_ADDR, (uint8_t*)&rec, sizeof(UnlockRecord));
}

void LoadUnlockData()
{
	UnlockRecord rec;
	UnlockedLevels = 1;

	if (!UnlockRead(&rec))
	{
		//never written, written by an older layout, or damaged. Start clean so the
		//next save has something valid to build on
		UnlockReset(&rec);
		StoreWrite(STORE_UNLOCK_ADDR, (uint8_t*)&rec, sizeof(UnlockRecord));
		return;
	}

	int8_t idx = UnlockFindPack(&rec, LevelPackName);
	if (idx < 0)
		return;   //pack has no progress yet, level 1 it is

	//the record keeps 16 bits, clamp before narrowing it to UnlockedLevels
	uint16_t unlocked = rec.unlocked[idx];
	if (unlocked < 1)
		unlocked = 1;
	if ((InstalledLevels > 0) && (unlocked > InstalledLevels))
		unlocked = InstalledLevels;
	UnlockedLevels = (uint8_t)unlocked;
}

// ---- settings -------------------------------------------------------------

static void SettingsReset(SettingsRecord* rec)
{
	memset(rec, 0, sizeof(SettingsRecord));
	rec->magic = SETTINGS_MAGIC;
	rec->version = STORE_VERSION;
	rec->skin = 0;
	rec->flags = SETTING_MUSIC | SETTING_SOUND;   //both on by default
}

static bool SettingsRead(SettingsRecord* rec)
{
	StoreRead(STORE_SETTINGS_ADDR, (uint8_t*)rec, sizeof(SettingsRecord));
	if (rec->magic != SETTINGS_MAGIC)
		return false;
	if (rec->version != STORE_VERSION)
		return false;
	if ((rec->skin < 0) || (rec->skin >= MAXSKINS))
		return false;
	return StoreCrcOk((const uint8_t*)rec, sizeof(SettingsRecord));
}


bool AskQuestionUpdate(int8_t* Id, bool* Answer, bool)
{
	*Answer = false;
	*Id = AskingQuestionID;

	if (AskingQuestionID > -1)
	{
		if ((currButtons & BUTTON_A) && !(prevButtons & BUTTON_A))
		{
			*Answer = true;
			ScreenForceRedraw();
			CWorldParts_MarkAllDirty();
			AskingQuestion = false;
			AskingQuestionID = -1;
			prevButtons = currButtons;
			playMenuSelectSound();
			return true;
		}

		if ((currButtons & BUTTON_B) && !(prevButtons & BUTTON_B))
		{
			*Answer = false;
			ScreenForceRedraw();
			CWorldParts_MarkAllDirty();
			AskingQuestion = false;
			AskingQuestionID = -1;
			prevButtons = currButtons;
			playMenuBackSound();
			return true;
		}
	}
	return false;
}

void AskQuestion(int8_t Id, const char* Msg)
{
	uint8_t fh = 8;
	//line count and line length (in characters) of the message, both grow with the screen size
	uint16_t count = 0;
	const char* s = Msg;
	while (*s) {
		if (*s == '\n')
			count++;
		s++;
	}
	//pixel offsets, sizes and positions, a message wider or taller than the screen puts MsgX / MsgY below 0
	int16_t diffy = 0;
	//add some lowering as title box is not centered
	if ((GameState == GSTitleScreen))
		diffy = 12;
	int16_t MsgHeight = (count + 2) * fh;
	int16_t MsgWidth = MaxLineLen(Msg)*6;
	int16_t MsgX = (WINDOW_WIDTH - MsgWidth) >> 1;
	int16_t MsgY = (WINDOW_HEIGHT - MsgHeight) >> 1;
	GFX.fillRect(MsgX - 5, MsgY - 5 + diffy, MsgWidth + 10, MsgHeight + 10, ColorWhite);
	GFX.drawRect(MsgX - 5, MsgY - 5 + diffy, MsgWidth + 10, MsgHeight + 10, ColorBlack);
	GFX.drawRect(MsgX - 3, MsgY - 3 + diffy, MsgWidth + 6, MsgHeight + 6, ColorBlack);
	tftPrint(MsgX, MsgY + diffy, Msg, ColorBlack, ColorBlack, 1);

	ScreenForceRedraw();
	CWorldParts_MarkAllDirty();
	AskingQuestionID = Id;
	AskingQuestion = (Id > -1);
	prevButtons = currButtons;
}


//The menu screens are static pictures, repainting them every frame is what
//makes them flicker. Each one hands over a value describing what it is about
//to show and only draws when that changed since the last frame.
//a signature is GameState * 100000 plus a few small values (up to about 1.4 million), -1 = none
static int32_t lastScreenSig = -1;

bool ScreenChanged(int32_t signature)
{
	if (signature == lastScreenSig)
		return false;
	lastScreenSig = signature;
	return true;
}

//something covered the screen, whatever is underneath has to be painted again
void ScreenForceRedraw()
{
	lastScreenSig = -1;
}

void FindLevels()
{
	InstalledLevels = LevelPackFile->LevelCount;
}

void printTitleInfo()
{
	char Tekst[250];
	int16_t w;
	if (LevelPackFile->Loaded)
	{
		if (strlen(LevelPackFile->author) > 0)
		{
			snprintf(Tekst, sizeof(Tekst), "Levels by\n%s", LevelPackFile->author);
			w = MaxLineLen(Tekst)*6;
			tftPrint((WINDOW_WIDTH - w) / 2, 100, Tekst, ColorBlack, ColorBlack, 1);
		}
	}
}

//multi line text straight to the screen. tft.drawChar already matches what the
//framebuffer version did per character, a 6x8 cell with the background only
//painted when it differs from the text colour, so only the line breaks are
//handled here. Advances match the old code, 6 pixels per char and 9 per line
void tftPrint(int16_t x, int16_t y, const char* str, uint16_t color, uint16_t bg, uint8_t size)
{
	int16_t cursorX = x;
	int16_t cursorY = y;
	if (!str)
		return;
#if LOVYANGFX
	//LovyanGFX's drawChar that takes the colours hands them to the font the other way
	//round, set them as the text colour instead. Its default font is the same 6x8 GLCD
	//font and a background equal to the text colour is left out here as well
	GFX.setTextColor(color, bg);
	GFX.setTextSize(size);
#endif
#if SCREENBUFFER == 0
	//Straight to the display every character would be a write transaction of its own, and
	//the chip select sits on the I/O expander: that is I2C traffic per character. One
	//transaction for the whole text instead. Into a buffer nothing is sent, so nothing to do
	SCREEN.startWrite();
#endif
	while (*str)
	{
		if (*str == '\n')
		{
			cursorY += 9 * size;
			cursorX = x;
			str++;
			continue;
		}
#if LOVYANGFX
		GFX.drawChar((uint8_t)*str, cursorX, cursorY);
#else
		GFX.drawChar(cursorX, cursorY, *str, color, bg, size);
#endif
		cursorX += 6 * size;
		str++;
	}
#if SCREENBUFFER == 0
	SCREEN.endWrite();
#endif
}

//length of the longest line in Text, the count restarts on every newline so a
//multi line message reports the width it actually needs and not its total length
uint16_t MaxLineLen(const char* Text)
{
	uint16_t Result = 0;
	uint16_t Len = 0;
	if (!Text)
		return 0;
	while (*Text)
	{
		if (*Text == '\n')
			Len = 0;
		else
		{
			Len++;
			if (Len > Result)
				Result = Len;
		}
		Text++;
	}
	return Result;
}

//a char is -128 .. 127 or 0 .. 255 depending on the platform, int16_t holds either
char chr(int16_t ascii)
{
	return((char)ascii);
}

int16_t ord(char chr)
{
	return((int16_t)chr);
}


void LoadSettings()
{
	SettingsRecord rec;

	if (!SettingsRead(&rec))
	{
		//nothing valid stored yet, keep the defaults and lay down a good record
		SettingsReset(&rec);
		StoreWrite(STORE_SETTINGS_ADDR, (uint8_t*)&rec, sizeof(SettingsRecord));
		setMusicOn(true);
		setSoundOn(true);
		skin = 0;
		return;
	}

	setMusicOn((rec.flags & SETTING_MUSIC) != 0);
	setSoundOn((rec.flags & SETTING_SOUND) != 0);
	skin = rec.skin;
	//the stored pack only counts if it is still installed, otherwise whatever
	//SearchForLevelPacks picked stays selected
	for (uint8_t Teller = 0; Teller < InstalledLevelPacksCount; Teller++)
		if (rec.packId == PackId(CLevelPackFile_BuiltInName(Teller)))
		{
			SelectedLevelPack = Teller;
			LevelPackName = CLevelPackFile_BuiltInName(SelectedLevelPack);
			break;
		}
}

void SaveSettings()
{
	SettingsRecord rec;
	if (!SettingsRead(&rec))
		SettingsReset(&rec);

	uint8_t flags = (uint8_t)((isMusicOn() ? SETTING_MUSIC : 0) | (isSoundOn() ? SETTING_SOUND : 0));

	const uint16_t packId = PackId(LevelPackName);

	if ((rec.flags == flags) && (rec.skin == skin) && (rec.packId == packId))
		return;   //nothing changed, leave the flash alone

	rec.flags = flags;
	rec.skin = skin;
	rec.packId = packId;
	StoreWrite(STORE_SETTINGS_ADDR, (uint8_t*)&rec, sizeof(SettingsRecord));
}


void SearchForLevelPacks()
{
	//every pack is built in, nothing to search for
	//not MaxLevelPacks: LEVELPACKS may have left some of them out of the build
	InstalledLevelPacksCount = CLevelPackFile_BuiltInCount();
	SelectedLevelPack = 0;
	LevelPackName = CLevelPackFile_BuiltInName(SelectedLevelPack);
}

