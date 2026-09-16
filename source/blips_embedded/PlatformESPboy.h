#ifndef PLATFORM_ESPBOY_H
#define PLATFORM_ESPBOY_H

//The ESPboy part of Platform.h, included by it. It supplies what Platform.h can not
//write portably, see "The device" there. The functions are in PlatformESPboy.cpp.

#include <Arduino.h>

//display library: 1 = LovyanGFX, 0 = TFT_eSPI
#define LOVYANGFX 1

#define FLOODFILLFLOOR 1

#define FPSLOCK 0

//where drawing goes:
//  0  = no buffer, only the board cells that changed are composed and sent to the display
//  1  = 1 bpp buffer, every pixel is either ColorWhite or ColorBlack, so the black &
//       white skin is always used whatever skin is selected. 2K of RAM
//  8  = 8 bpp buffer, the whole frame is drawn off screen and pushed once. It is kept
//       as RGB332, so the images drop to about 116 colours. 16K of RAM
//  16 = 16 bpp buffer, same but full colour. 32K of RAM, only ~2K of heap left over
//a build can still set it itself
#ifndef SCREENBUFFER
#define SCREENBUFFER 8
#endif

//image set to build with, 1 or 2, see IMAGESET in Defines.h. A build can still set it itself
#ifndef IMAGESET
#define IMAGESET 2
#endif

//-1 = every skin, n = only skin n is built in, see FORCESKIN in the game's defines. A build
//can still set it itself
#ifndef FORCESKIN
#define FORCESKIN -1
#endif

#if LOVYANGFX
//only the library here, the panel and its wiring are configured in PlatformESPboy.cpp.
//Its display class adds nothing but a constructor to LGFX_Device, so the game draws
//through the base class
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
typedef lgfx::LGFX_Device PlatformDisplay;
typedef LGFX_Sprite PlatformBuffer;
//start of the sprite's pixels, both libraries lay them out the same way: 16 bpp byte
//swapped RGB565, 8 bpp RGB332, 1 bpp packed most significant bit first
#define SCREENBUFFER_PIXELS() screenBuffer.getBuffer()
#else
#include <TFT_eSPI.h>
typedef TFT_eSPI PlatformDisplay;
typedef TFT_eSprite PlatformBuffer;
#define SCREENBUFFER_PIXELS() screenBuffer.getPointer()
#endif

//Data in flash: the ESP8266 maps it into memory, but that memory only takes 32 bit reads.
//pgm_read_byte / pgm_read_word read it safely whatever the alignment
#define PLATFORM_PROGMEM PROGMEM
#define PLATFORM_READ_BYTE(addr) pgm_read_byte(addr)
#define PLATFORM_READ_WORD(addr) pgm_read_word(addr)
//a run of bytes at once, memcpy_P reads 32 bits at a time and is far cheaper per byte
#define PLATFORM_READ_BYTES(dst, addr, len) memcpy_P((dst), (addr), (len))

#endif
