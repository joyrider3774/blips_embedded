#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "Common.h"
#include "GameFuncs.h"
#include "Sound.h"
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
				d[c] = (uint8_t)(((color & 0xE000) >> 8) | ((color & 0x0700) >> 6) | ((color & 0x0018) >> 3));
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

//Draws an opaque RGB565 image. TFT_eSprite's 1 bpp pushImage expects 1 bpp image data,
//so there the pixels are converted and written into the buffer directly. LovyanGFX
//reads image data through plain pointers, but PROGMEM on the ESP8266 is flash that
//only takes 32 bit reads, so with that library every image is read here with
//PLATFORM_READ_BYTES and handed over as pixels
void DrawImage(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t* image)
{
	if (!image)
		return;
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
		//the visible part of the row in one copy out of flash
		PLATFORM_READ_BYTES((uint8_t*)line, image + (r * w + c0) * sizeof(uint16_t), (c1 - c0) * sizeof(uint16_t));
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
		int16_t runX = 0, runLen = 0;
		for (int16_t c = 0; c <= w; c++)
		{
			int16_t sx = x + c;
			uint16_t color = 0xF81F;
			if ((c < w) && (sx >= 0) && (sx < WINDOW_WIDTH))
				color = PLATFORM_READ_WORD(&src[r * w + c]);
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
void pushImageRLE(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t* data)
{
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
		//RGB332, the same conversion both libraries apply to everything else
		const uint8_t value = (uint8_t)(((color & 0xE000) >> 8) | ((color & 0x0700) >> 6) | ((color & 0x0018) >> 3));
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
				if (run)
					memset(&d[sx + c0], value, c1 - c0);
				else
					for (int16_t c = c0; c < c1; c++)
						d[sx + c] = (uint8_t)(((src[c] & 0xE000) >> 8) | ((src[c] & 0x0700) >> 6) | ((src[c] & 0x0018) >> 3));
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

void LoadGraphics(void)
{
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
			ColorBlack = SCREEN.color565(0,0,0);
			ColorWhite = SCREEN.color565(255,255,255);			
			break;
#endif
	}
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

#define STORE_VERSION    1
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
	char     packName[MaxLevelPacks][MaxLevelPackNameLength];
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
	char     levelPack[MaxLevelPackNameLength];
	uint16_t crc;   //covers every byte before it
};

#define STORE_UNLOCK_ADDR    0
#define STORE_SETTINGS_ADDR  128
#define STORE_TOTAL          (STORE_SETTINGS_ADDR + sizeof(SettingsRecord))

//the unlock record must never grow into the settings, this stops compiling if it does
static_assert(sizeof(UnlockRecord) <= STORE_SETTINGS_ADDR, "the unlock record overlaps the settings");
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
static int8_t UnlockFindPack(const UnlockRecord* rec, const char* name)
{
	for (uint8_t Teller = 0; Teller < rec->packCount; Teller++)
		if (strncmp(rec->packName[Teller], name, MaxLevelPackNameLength - 1) == 0)
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
		memset(rec.packName[idx], 0, MaxLevelPackNameLength);
		snprintf(rec.packName[idx], MaxLevelPackNameLength, "%s", LevelPackName);
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
	//a name that never terminates would run off the end of the record
	if (rec->levelPack[MaxLevelPackNameLength - 1] != 0)
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
		if (strncmp(rec.levelPack, InstalledLevelPacks[Teller], MaxLevelPackNameLength - 1) == 0)
		{
			SelectedLevelPack = Teller;
			LevelPackName = InstalledLevelPacks[SelectedLevelPack];
			break;
		}
}

void SaveSettings()
{
	SettingsRecord rec;
	if (!SettingsRead(&rec))
		SettingsReset(&rec);

	uint8_t flags = (uint8_t)((isMusicOn() ? SETTING_MUSIC : 0) | (isSoundOn() ? SETTING_SOUND : 0));

	char name[MaxLevelPackNameLength];
	memset(name, 0, MaxLevelPackNameLength);
	snprintf(name, sizeof(name), "%s", LevelPackName);

	if ((rec.flags == flags) && (rec.skin == skin) && (memcmp(rec.levelPack, name, MaxLevelPackNameLength) == 0))
		return;   //nothing changed, leave the flash alone

	rec.flags = flags;
	rec.skin = skin;
	memcpy(rec.levelPack, name, MaxLevelPackNameLength);
	StoreWrite(STORE_SETTINGS_ADDR, (uint8_t*)&rec, sizeof(SettingsRecord));
}


void SearchForLevelPacks()
{
	//every pack is built in, nothing to search for
	InstalledLevelPacksCount = MaxLevelPacks;
	SelectedLevelPack = 0;
	LevelPackName = InstalledLevelPacks[SelectedLevelPack];
}

