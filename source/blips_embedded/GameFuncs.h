#ifndef GAMEFUNCS_H
#define GAMEFUNCS_H

#include <stdint.h>
#include <stddef.h>

void FindLevels();
bool ScreenChanged(int32_t signature);
void ScreenForceRedraw();
int16_t ord(char chr);
char chr(int16_t ascii);
uint16_t MaxLineLen(const char* Text);
void tftPrint(int16_t x, int16_t y, const char* str, uint16_t color, uint16_t bg, uint8_t size);
//whether the sprites of the skin in use cover their whole tile, worked out by LoadGraphics
extern bool IMGBoxOpaque, IMGWallOpaque, IMGFloorOpaque;
void printTitleInfo();
void LoadUnlockData();
void SaveUnlockData();
void AskQuestion(int8_t Id, const char* Msg);
bool AskQuestionUpdate(int8_t* Id, bool* Answer, bool MustBeAButton);
void SearchForLevelPacks();
void SaveSettings();
void LoadSettings();
void LoadGraphics();
uint8_t CurrentSkin(void);
void DrawImage(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t* image);
//the same, magenta pixels are left out
void DrawImageTransparent(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t* image);
//draws a run length encoded full screen image (tools/png2rle565.py)
void pushImageRLE(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t* data);
//only exists with a screen buffer: draws an image into it, transparent skips magenta pixels
void DrawImageToBuffer(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t* image, bool transparent);
#endif