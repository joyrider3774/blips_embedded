# Blips Embedded Version
![DownloadCountTotal](https://img.shields.io/github/downloads/joyrider3774/blips_embedded/total?label=total%20downloads&style=plastic) ![DownloadCountLatest](https://img.shields.io/github/downloads/joyrider3774/blips_embedded/latest/total?style=plastic) ![LatestVersion](https://img.shields.io/github/v/tag/joyrider3774/blips_embedded?label=Latest%20version&style=plastic) ![License](https://img.shields.io/github/license/joyrider3774/blips_embedded?style=plastic)

Blips is a remake of the dos bips game made by Bryant Brownell, its a sokoban style puzzle game but with exploding dynamite and collecting coins

## Screenshots
The browser build, at twice the game's own 128x128:

| Title screen | In game |
| --- | --- |
| ![Blips title screen](metadata/screenshots/title.png) | ![Blips in game](metadata/screenshots/ingame.png) |

## Game Features:
- 4 level packs from the original Bips games with 69 levels (Bips, Bips Gold, Bips Platinum and Bips Gold 2 Players)
- Adapted graphics over original gp2x game
- 2 skins to choose from (default and black & white, the Gamebuino META build only has the default skin)
- Sound can be switched on or off
- Free view mode to look around levels that are bigger than the screen
- Autosaves progress per level pack

## Playing the Game:
The aim of the game is to collect all the dollar coins in a level, you need to create pathways to these coins by pushing boxes around or even make them explode by pushing them into the dynamite

## Controls

| Button | Action |
| ------ | ------ |
| DPAD | Select menu's, options, level packs or levels. Move the player in game, pan around the level in free view |
| A | Confirm in menus, level selector and questions asked. In 2 player levels switch between the players |
| B | Back in options and level selector, ask to quit to the level selector in game, leave free view |
| L | Ask to restart the level |
| R | Enter or leave free view |
| (A) + Left + Down | Show or hide the debug info |

### Buttons
The game's buttons on every device:

| Device | D-pad | A | B | L | R |
| ------ | ----- | - | - | - | - |
| ESPboy | d-pad | ACT | ESC | LFT | RGT |
| Gamebuino META | d-pad | A | B | MENU | HOME |
| Adafruit PyBadge | d-pad | A | B | SELECT | START |
| Adafruit PyGamer | joystick | A | B | SELECT | START |
| Pimoroni PicoSystem | d-pad | A | B | Y | X |
| Pimoroni Explorer | A up, C down, B left, Y right | X | Z | BOOT + B | BOOT + Y |
| Pimoroni Tufty 2350 | UP up, DOWN down, A left, C right | B | HOME | HOME + A | HOME + C |
| TinyCircuits Thumby Color | d-pad | A | B | left bumper | right bumper |
| Playdate | d-pad | A | B | menu: restart | menu: free view |
| Libretro | d-pad | A | B | L | R |
| Game Boy Advance | d-pad | A | B | L | R |
| Nintendo DS | d-pad | A | B | L | R |
| Nintendo 3DS | d-pad or circle pad | A | B | L | R |
| Nintendo 64 | d-pad | A | B | L | R |
| PlayStation | d-pad | Cross | Circle | L1 | R1 |
| PlayStation Portable | d-pad or the analog stick | Cross | Circle | L | R |
| PlayStation Vita | d-pad or the left stick | Cross | Circle | L | R |
| Windows | arrow keys | X | C | S | D |
| MS-DOS | arrow keys | X | C | S | D |
| Browser | arrow keys | X | C | S | D |

On the Explorer BOOT is held as a shift, the direction pressed with it is not sent.

On the Tufty 2350 a tap of HOME is B when it is let go, held HOME is a shift like BOOT on the Explorer.

On the Gamebuino META holding HOME for a second goes back to its loader.

## Devices
Every [release](https://github.com/joyrider3774/blips_embedded/releases) has a build for every device. `releases/` is where a build of your own puts them, it is not part of the repository:

| Device | File | How to install |
| ------ | ---- | -------------- |
| [ESPboy](https://www.espboy.com/) | ESPboy_Blips.bin | flash it, the board is a LOLIN(WEMOS) D1 mini |
| [Gamebuino META](https://gamebuino.com/gamebuino-meta) | GamebuinoMeta_Blips.zip | unzip it onto the SD card, it holds a Blips_embedded folder with the game, its save and the loader's images, the .hex in it is for flashing the game directly |
| [Adafruit PyBadge](https://www.adafruit.com/product/4200) | PyBadge_Blips.uf2 | double press reset and copy it onto the drive that appears |
| [Adafruit PyGamer](https://www.adafruit.com/product/4242) | PyGamer_Blips.uf2 | same as the PyBadge |
| [Pimoroni PicoSystem](https://shop.pimoroni.com/products/picosystem) | PicoSystem_Blips.uf2 | hold X while switching on and copy it onto the drive that appears |
| [Pimoroni Explorer](https://shop.pimoroni.com/products/explorer?variant=42092697845843) | Explorer_Blips.uf2 | hold BOOT while pressing RESET and copy it onto the drive that appears |
| [Pimoroni Tufty 2350](https://shop.pimoroni.com/products/tufty-2350?variant=55811986227579) | Tufty_Blips.uf2 | hold HOME while pressing RESET and copy it onto the drive that appears |
| [TinyCircuits Thumby Color](https://tinycircuits.com/products/thumby-color) | ThumbyColor_Blips.uf2 | put it into bootloader mode and copy it onto the RPI-RP2 drive that appears |
| [Playdate](https://play.date/) | Playdate_Blips.pdx.zip | unzip it and sideload Blips.pdx, the same pdx runs in the Playdate simulator |
| [Libretro / RetroArch](https://www.retroarch.com/) | Libretro_Blips.zip | copy blips_libretro.dll into RetroArch's cores folder and blips_libretro.info into its info folder, then Load Core and Start Core |
| [Game Boy Advance](https://en.wikipedia.org/wiki/Game_Boy_Advance) | GBA_Blips.gba | put it on a flash cart or open it in an emulator, the progress is saved in the cartridge's SRAM |
| [Nintendo DS](https://en.wikipedia.org/wiki/Nintendo_DS) | NDS_Blips.nds | put it on a flash card or open it in an emulator, the progress is saved next to it in Blips.sav |
| [Nintendo 3DS](https://en.wikipedia.org/wiki/Nintendo_3DS) | 3DS_Blips.3dsx | copy it into /3ds/ on the SD card and start it from the Homebrew Launcher, or open it in an emulator, the progress is saved in sdmc:/3ds/Blips/ |
| [Nintendo 64](https://en.wikipedia.org/wiki/Nintendo_64) | N64_Blips.z64 | put it on a flash cart or open it in an emulator, the progress is saved in the cartridge EEPROM |
| [PlayStation](https://en.wikipedia.org/wiki/PlayStation_(console)) | PSX_Blips.exe | open it in an emulator or send it to a console that runs unsigned code, the progress is not saved yet |
| [PlayStation Portable](https://en.wikipedia.org/wiki/PlayStation_Portable) | PSP_Blips.PBP | rename it to EBOOT.PBP and put it in ms0:/PSP/GAME/Blips/ on the memory stick, or open it in PPSSPP |
| [PlayStation Vita](https://en.wikipedia.org/wiki/PlayStation_Vita) | Vita_Blips.vpk | install it with VitaShell on a Vita with homebrew enabled, or open it in Vita3K |
| Windows | Windows_Blips.exe | runs on its own, the progress is saved next to it in Blips.sav |
| MS-DOS | DOS_Blips.zip | unzip BLIPS.EXE onto a DOS machine or into DOSBox and run it, the progress is saved next to it in BLIPS.SAV |
| Browser | Web_Blips.zip | upload it to an itch.io HTML project, or unzip it and open index.html from a web server, the progress is saved in the browser |

The Tufty 2350 has no speaker, the game is silent there. Holding RESET until the rear LEDs are dark puts it to sleep, a front button wakes it up again, with UP and DOWN held as well it goes into shipping mode instead.

The Thumby Color's display is 128x128, the game's own size, so it is shown 1:1 over the whole screen. That build has not been tried on the device itself yet.

The Playdate shows the black & white skin, scaled up in the middle of its display. It has no side buttons, L and R are the restart and free view entries of its system menu.

The Game Boy Advance shows the game scaled to 160x160 in the middle of its screen, with black bars at the sides. Blips runs at about 20 frames a second there, the other devices show it at 30. Scrolling around a level that is bigger than the screen is slow there: the game engine draws the whole screen again while the view moves.

On the Nintendo DS the game is scaled to 192x192 in the middle of the top screen, with black bars at the sides, and the bottom screen stays dark. The progress is saved as Blips.sav on the card the game was started from, so a card that libfat can not write to (or an emulator without one) plays the game but forgets it afterwards. Its tones are square waves played as a sample: the DS's own tone channels can not go below about 256 Hz, which is under the notes of the explosion.

On the Nintendo 3DS the game is scaled to 240x240 in the middle of the top screen, with black bars at the sides, and the bottom screen stays dark. What the game saves goes into sdmc:/3ds/Blips/Blips.sav. Its tones play through the console's DSP when the DSP firmware has been dumped to the SD card (sdmc:/3ds/dspfirm.cdc), and through CSND when it has not: on hardware either one plays, in an emulator only the DSP one does.

On MS-DOS the game runs in VGA mode X, 320x240 in 256 colours, blown up to 240x240 in the middle of the screen with black bars at the sides. That mode rather than the usual 320x200 one because its pixels are square, where 320x200 is stretched over the same screen and would show the game a fifth too tall. The 256 colours are set to the RGB332 cube, which is exactly what the game's 8 bpp screen buffer holds, so a frame reaches the card without a colour being worked out. Its tones are a square wave on the PC speaker, the progress is saved next to the program in BLIPS.SAV, and Escape quits. The program is 32 bit and carries the CWSDPMI host inside it, so it needs nothing beside it on the disk.

In a browser the game is drawn into a canvas of its own 128x128 pixels, which the page stretches to whatever room it is given while keeping it square and keeping the pixels sharp. What the game saves goes into the browser's localStorage under the game's name, so a private window plays it but forgets it afterwards. The zip holds index.html, index.js and index.wasm and is what an itch.io HTML project takes as it is.

On the Nintendo 64 the game is drawn into memory and the RDP puts it on screen scaled to 240x240 in the middle of its 320x240 screen, with black bars at the sides. Its tones are a square wave written into the buffers the sound hardware plays from. The progress is saved in the cartridge EEPROM, which the ROM says it has, so a cartridge or an emulator without one plays the game but forgets it afterwards.

On the PlayStation the game is drawn into memory, handed to the GPU as a texture and shown scaled to 240x240 in the middle of its 320x240 screen, with black bars at the sides. Its tones are a square wave the SPU plays from a single looping block. The memory card is not written yet, so the progress is gone when the console is switched off.

On the PlayStation Portable the game is doubled to 256x256 in the middle of the display, and the high scores are saved next to the EBOOT.PBP in Blips.sav.

On the PlayStation Vita the game is blown up four times to 512x512 in the middle of the display, and the high scores are saved in ux0:data/Blips/Blips.sav.

## Building
`python tools/build_releases.py` builds a release for every device  
`python tools/convert_skins.py` turns the images in `assets/skins` and `assets/skins2` into the headers the game includes  
`python tools/convert_levelpacks.py` does the same for the level packs in `assets/levelpacks`

### Where the tools are
The script looks for everything in the place it is installed in here. A tool somewhere else is passed on the command line, or set as the environment variable in the last column and left off the command line:

| Option | What it points at | Default, or environment variable |
| ------ | ----------------- | -------------------------------- |
| `--arduino-cli PATH` | arduino-cli, which builds the Arduino devices | `ARDUINO_CLI` |
| `--arduino DIR` | the Arduino IDE 1.8 folder, used when there is no arduino-cli | `C:/arduino`, `ARDUINO_DIR` |
| `--lovyangfx DIR` | LovyanGFX for the Windows build, when it is not the one in the sketchbook | `LOVYANGFX_DIR` |
| `--msys2 DIR` | MSYS2's mingw64 bin folder, for cmake and ninja | `C:/msys64/mingw64/bin`, `MSYS2_BIN` |
| `--playdate-sdk DIR` | the Playdate SDK | `C:/playdate/PlaydateSDK`, `PLAYDATE_SDK_PATH` |
| `--playdate-arm DIR` | the bin folder of the ARM gcc the Playdate needs | `PLAYDATE_ARM_BIN` |
| `--libretro-common DIR` | libretro-common | `C:/github/libretro-common`, `LIBRETRO_COMMON_DIR` |
| `--devkitpro DIR` | devkitARM with libgba, libnds, calico, libctru and tools | `C:/devkitarm`, `DEVKITPRO` |
| `--psn00bsdk DIR` | PSn00bSDK | `C:/psn00bsdk`, `PSN00BSDK_PREFIX` |
| `--n64 DIR` | the mips64-elf toolchain with libdragon | `C:/n64_dev`, `N64_INST` |
| `--emsdk DIR` | the Emscripten SDK | `C:/github/emsdk`, `EMSDK` |
| `--dosdev DIR` | DJGPP with CWSDPMI | `C:/dos_dev`, `DOSDEV` |
| `--pspdev DIR` | the pspdev toolchain | `C:/psp_dev`, `PSPDEV_DIR` |
| `--vitasdk DIR` | VitaSDK | `C:/psvita_dev`, `VITASDK` |
| `--sdl2-mingw DIR` | SDL2's mingw package, its x86_64-w64-mingw32 folder | `SDL2_MINGW` |

Only the devices being built need their tool, so one missing toolchain does not stop the rest:

```
python tools/build_releases.py --only N64 DOS --n64 D:/n64_dev --dosdev D:/dos_dev
python tools/build_releases.py --list          shows what would be built
python tools/build_releases.py --only Web      one device only
```

### Build settings
Every device is built with its own settings. These change them for all of the devices at once, and `--list` shows what the defines would be without building anything. They are the same defines the device headers and the `platforms/*/CMakeLists.txt` files take, so a single device can be built with `-D<name>=<value>` from cmake instead:

| Option | What it sets | Values |
| ------ | ------------ | ------ |
| `--forceskin N` | `FORCESKIN`, the skin built in | `-1`, or `0` to `1` |
| `--forcescreenbuffer N` | `SCREENBUFFER`, where drawing goes | `0`, `1`, `8` or `16` bits per pixel |
| `--forcedebug` | `FORCEDEBUG 1`, the debug header is always shown | no value, on when it is given |

`-1` builds every skin in and lets the game's own options pick one. A number builds only that skin in, which is what a device short of room needs.

Not every device takes every buffer mode, `platforms/<device>/CMakeLists.txt` says which, and one it does not take stops that build with a message. A 1 bpp buffer can only show the one skin that is black and white, so it forces that skin whatever `--forceskin` says, see `FORCESKIN` in `defines.h`.

```
python tools/build_releases.py --forceskin 1                only skin 1, on every device
python tools/build_releases.py --only Windows --forcescreenbuffer 1
python tools/build_releases.py --list --forcedebug          what the defines would be
```

### Board packages and libraries
The Arduino devices are built with arduino-cli 1.5.1 and the versions below. They are the ones every release is built with, `.github/workflows/build-releases.yml` pins them:

| Device | Board package | Libraries |
| ------ | ------------- | --------- |
| ESPboy | esp8266:esp8266 3.1.2 | LovyanGFX 1.1.9, TFT_eSPI 2.4.72 |
| Gamebuino META | gamebuino:samd 1.2.2 | Gamebuino META 1.3.3 |
| Adafruit PyBadge, PyGamer | adafruit:samd 1.7.16 | Adafruit GFX Library 1.12.6, Adafruit ST7735 and ST7789 Library 1.5.15, Adafruit BusIO 1.17.4, Adafruit NeoPixel 1.15.5, Adafruit SPIFlash 5.1.1 |
| PicoSystem, Explorer, Tufty 2350, Thumby Color | rp2040:rp2040 5.5.0 | none, everything they use comes with the core |

The ESPboy draws through LovyanGFX and only includes TFT_eSPI's header, so the exact TFT_eSPI does not matter much.  
The Gamebuino's core needs Arduino's own arduino:samd 1.8.14 beside it for sam.h, without it the build stops at "sam.h: No such file or directory".  
The Windows build draws through the same LovyanGFX 1.1.9, see `platforms/windows/CMakeLists.txt`.

### Toolchains
The Playdate build also needs the Playdate SDK, see `platforms/playdate/CMakeLists.txt`  
The libretro core needs libretro-common, see `platforms/libretro/CMakeLists.txt`  
The Game Boy Advance build needs devkitARM and libgba, see `platforms/gba/CMakeLists.txt`  
The Nintendo DS build needs devkitARM, libnds and calico, see `platforms/nds/CMakeLists.txt`  
The Nintendo 3DS build needs devkitARM and libctru, see `platforms/3ds/CMakeLists.txt`  
The PlayStation build needs PSn00bSDK, see `platforms/psx/CMakeLists.txt`  
The Nintendo 64 build needs the mips64-elf toolchain and libdragon, see `platforms/n64/CMakeLists.txt`  
The PSP build needs the pspdev toolchain, see `platforms/psp/CMakeLists.txt` (pspdev has no Windows build, so on Windows it is built from WSL)  
The Vita build needs VitaSDK, see `platforms/vita/CMakeLists.txt`  
The browser build needs Emscripten, see `platforms/web/CMakeLists.txt`  
The MS-DOS build needs DJGPP, see `platforms/dos/CMakeLists.txt`


## Credits
Game is a remake of dos bips game made by Bryant Brownell and based on the gp2x version i initially made

### Level authors
- Bryant Brownell
- Landon Brownell
- Caryn Brownell
- The PocoMan Team

### Graphics
- dollar coin - [kenney game assets all in 1](https://kenney.itch.io/kenney-game-assets) - [CC0 1.0 Universal](https://creativecommons.org/publicdomain/zero/1.0/)
- wall: [1001.com](https://opengameart.org/content/sokoban-pack) - [Attribution-ShareAlike 3.0 Unported](https://creativecommons.org/licenses/by-sa/3.0/)
- floor: [Kenney Sokoban tiles](https://opengameart.org/content/sokoban-100-tiles) - [CC0 1.0 Universal](https://creativecommons.org/publicdomain/zero/1.0/)
- player: [Kenney Sokoban tiles](https://opengameart.org/content/sokoban-100-tiles) - [CC0 1.0 Universal](https://creativecommons.org/publicdomain/zero/1.0/)
- box: [SpriteAttack boxes and crates](https://opengameart.org/content/boxes-and-crates-svg-and-pngs) - [CC0 1.0 Universal](https://creativecommons.org/publicdomain/zero/1.0/)
- dynamite: [GUI Icons by Rexard](https://www.gamedevmarket.net/asset/gui-icons-8656) - I Payed for this asset do not reuse !



