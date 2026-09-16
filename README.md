# Blips Embedded Version
![DownloadCountTotal](https://img.shields.io/github/downloads/joyrider3774/blips_embedded/total?label=total%20downloads&style=plastic) ![DownloadCountLatest](https://img.shields.io/github/downloads/joyrider3774/blips_embedded/latest/total?style=plastic) ![LatestVersion](https://img.shields.io/github/v/tag/joyrider3774/blips_embedded?label=Latest%20version&style=plastic) ![License](https://img.shields.io/github/license/joyrider3774/blips_embedded?style=plastic)

Blips is a remake of the dos bips game made by Bryant Brownell, its a sokoban style puzzle game but with exploding dynamite and collecting coins

## Devices
Every [release](https://github.com/joyrider3774/blips_embedded/releases) has a build for every device. `releases/` is where a build of your own puts them, it is not part of the repository:

| Device | File | How to install |
| ------ | ---- | -------------- |
| [ESPboy](https://www.espboy.com/) | ESPboy_Blips.bin | flash it, the board is a LOLIN(WEMOS) D1 mini |
| [Gamebuino META](https://gamebuino.com/gamebuino-meta) | GamebuinoMeta_Blips.bin | copy it into a folder on the SD card, the .hex is for flashing it directly |
| [Adafruit PyBadge](https://www.adafruit.com/product/4200) | PyBadge_Blips.uf2 | double press reset and copy it onto the drive that appears |
| [Adafruit PyGamer](https://www.adafruit.com/product/4242) | PyGamer_Blips.uf2 | same as the PyBadge |
| [Pimoroni PicoSystem](https://shop.pimoroni.com/products/picosystem) | PicoSystem_Blips.uf2 | hold X while switching on and copy it onto the drive that appears |
| [Pimoroni Explorer](https://shop.pimoroni.com/products/explorer?variant=42092697845843) | Explorer_Blips.uf2 | hold BOOT while pressing RESET and copy it onto the drive that appears |
| [Pimoroni Tufty 2350](https://shop.pimoroni.com/products/tufty-2350?variant=55811986227579) | Tufty_Blips.uf2 | hold HOME while pressing RESET and copy it onto the drive that appears |
| [TinyCircuits Thumby Color](https://tinycircuits.com/products/thumby-color) | ThumbyColor_Blips.uf2 | put it into bootloader mode and copy it onto the RPI-RP2 drive that appears |
| [Playdate](https://play.date/) | Playdate_Blips.pdx.zip | unzip it and sideload Blips.pdx, the same pdx runs in the Playdate simulator |
| [Libretro / RetroArch](https://www.retroarch.com/) | Libretro_Blips.zip | copy blips_libretro.dll into RetroArch's cores folder and blips_libretro.info into its info folder, then Load Core and Start Core |
| [Game Boy Advance](https://en.wikipedia.org/wiki/Game_Boy_Advance) | GBA_Blips.gba | put it on a flash cart or open it in an emulator, the progress is saved in the cartridge's SRAM |
| [PlayStation Portable](https://en.wikipedia.org/wiki/PlayStation_Portable) | PSP_Blips.PBP | rename it to EBOOT.PBP and put it in ms0:/PSP/GAME/Blips/ on the memory stick, or open it in PPSSPP |
| [PlayStation Vita](https://en.wikipedia.org/wiki/PlayStation_Vita) | Vita_Blips.vpk | install it with VitaShell on a Vita with homebrew enabled, or open it in Vita3K |
| Windows | Windows_Blips.exe | runs on its own, the progress is saved next to it in Blips.sav |

`python tools/build_releases.py` builds all of them, `python tools/convert_skins.py` turns the images in `assets/skins` and `assets/skins2` into the headers the game includes and `python tools/convert_levelpacks.py` does the same for the level packs in `assets/levelpacks`. The Playdate build also needs the Playdate SDK, see `playdate/CMakeLists.txt`, the libretro core libretro-common, see `libretro/CMakeLists.txt`, the Game Boy Advance build devkitARM and libgba, see `gba/CMakeLists.txt`, the PSP build the pspdev toolchain, see `psp/CMakeLists.txt` (pspdev has no Windows build, so on Windows it is built from WSL), and the Vita build VitaSDK, see `vita/CMakeLists.txt`.

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
| PlayStation Portable | d-pad or the analog stick | Cross | Circle | L | R |
| PlayStation Vita | d-pad or the left stick | Cross | Circle | L | R |
| Windows | arrow keys | X | C | S | D |

On the Explorer BOOT is held as a shift, the direction pressed with it is not sent.

On the Tufty 2350 a tap of HOME is B when it is let go, held HOME is a shift like BOOT on the Explorer. It has no speaker, the game is silent there. Holding RESET until the rear LEDs are dark puts it to sleep, a front button wakes it up again, with UP and DOWN held as well it goes into shipping mode instead.

The Thumby Color's display is 128x128, the game's own size, so it is shown 1:1 over the whole screen. That build has not been tried on the device itself yet.

The Playdate shows the black & white skin, scaled up in the middle of its display. It has no side buttons, L and R are the restart and free view entries of its system menu.

The Game Boy Advance shows the game scaled to 160x160 in the middle of its screen, with black bars at the sides. Blips runs at about 20 frames a second there, the other devices show it at 30. Scrolling around a level that is bigger than the screen is slow there: the game engine draws the whole screen again while the view moves.

On the PlayStation Portable the game is doubled to 256x256 in the middle of the display, and the high scores are saved next to the EBOOT.PBP in Blips.sav.

On the PlayStation Vita the game is blown up four times to 512x512 in the middle of the display, and the high scores are saved in ux0:data/Blips/Blips.sav.

On the Gamebuino META holding HOME for a second goes back to its loader.

## Screenshots
![screenshot 1](screenshots/screenshot1.png)
![screenshot 2](screenshots/screenshot2.png)
![screenshot 3](screenshots/screenshot3.png)
![screenshot 4](screenshots/screenshot4.png)

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
| Up + Down | Show or hide the debug info |

## Credits
Game is a remake of dos bips game made by Bryant Brownell and based on the gp2x version i initially made

### Level authors
- Bryant Brownell
- Landon Brownell
- Caryn Brownell
- The PocoMan Team

### Music
Music was made by donskeeto

### Graphics
- dollar coin - [kenney game assets all in 1](https://kenney.itch.io/kenney-game-assets) - [CC0 1.0 Universal](https://creativecommons.org/publicdomain/zero/1.0/)
- wall: [1001.com](https://opengameart.org/content/sokoban-pack) - [Attribution-ShareAlike 3.0 Unported](https://creativecommons.org/licenses/by-sa/3.0/)
- floor: [Kenney Sokoban tiles](https://opengameart.org/content/sokoban-100-tiles) - [CC0 1.0 Universal](https://creativecommons.org/publicdomain/zero/1.0/)
- player: [Kenney Sokoban tiles](https://opengameart.org/content/sokoban-100-tiles) - [CC0 1.0 Universal](https://creativecommons.org/publicdomain/zero/1.0/)
- box: [SpriteAttack boxes and crates](https://opengameart.org/content/boxes-and-crates-svg-and-pngs) - [CC0 1.0 Universal](https://creativecommons.org/publicdomain/zero/1.0/)
- dynamite: [GUI Icons by Rexard](https://www.gamedevmarket.net/asset/gui-icons-8656) - I Payed for this asset do not reuse !



