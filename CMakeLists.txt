# Windows build of Blips: the game's own sources with the PLATFORM_SDL platform
# (Source/blips_embedded/PlatformSDL.*), LovyanGFX's SDL panel for the window and SDL2.
#
# With MSYS2 (mingw64 packages cmake, ninja or make, gcc and SDL2), from this folder:
#   cmake -S . -B build-windows -G Ninja            (add -DSCREENBUFFER=1 / -DIMAGESET=1 to change)
#   cmake --build build-windows
#   build-windows/blips.exe
# The exe is linked statically, it runs on its own without any dll from MSYS2. It is a Windows
# program without a console window, and what it does not use is left out of it.
# The save file (Blips.sav) is written to the folder the game is started from.

cmake_minimum_required(VERSION 3.16)
project(blips C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# LovyanGFX's lgfx_qrcode.c defines its own bool, which C23 (the newer gcc default) forbids
set(CMAKE_C_STANDARD 11)

# the same LovyanGFX the Arduino build uses
set(LOVYANGFX_DIR "C:/arduino/portable/sketchbook/libraries/LovyanGFX-master" CACHE PATH "LovyanGFX library folder")
if(NOT EXISTS "${LOVYANGFX_DIR}/src/LovyanGFX.hpp")
    message(FATAL_ERROR "LovyanGFX not found in ${LOVYANGFX_DIR}, pass -DLOVYANGFX_DIR=<folder>")
endif()

# where drawing goes, see SCREENBUFFER in Platform.h: 0, 1, 8 or 16. A PC has the memory
# for a full colour buffer, so that is the default here. Change it with -DSCREENBUFFER=<n>
set(SCREENBUFFER 16 CACHE STRING "SCREENBUFFER setting: 0, 1, 8 or 16")
set_property(CACHE SCREENBUFFER PROPERTY STRINGS 0 1 8 16)
if(NOT SCREENBUFFER MATCHES "^(0|1|8|16)$")
    message(FATAL_ERROR "SCREENBUFFER has to be 0, 1, 8 or 16, not ${SCREENBUFFER}")
endif()

# the image set, see IMAGESET in Defines.h: 1 = images (16x16 tiles), 2 = images2 (8x8 tiles).
# Change it with -DIMAGESET=<n>
set(IMAGESET 2 CACHE STRING "IMAGESET setting: 1 or 2")
set_property(CACHE IMAGESET PROPERTY STRINGS 1 2)
if(NOT IMAGESET MATCHES "^(1|2)$")
    message(FATAL_ERROR "IMAGESET has to be 1 or 2, not ${IMAGESET}")
endif()

# -1 = every skin, n = only skin n is built in (0 default, 1 black & white),
# see FORCESKIN in Defines.h. Change it with -DFORCESKIN=<n>
set(FORCESKIN -1 CACHE STRING "FORCESKIN setting: -1 or 0 to 1")
set_property(CACHE FORCESKIN PROPERTY STRINGS -1 0 1)
if(NOT FORCESKIN MATCHES "^(-1|[0-1])$")
    message(FATAL_ERROR "FORCESKIN has to be -1 or 0 to 1, not ${FORCESKIN}")
endif()

# 1 = frames are held to the game's frame rate, 0 = they run as fast as they go, see FPSLOCK
# in Defines.h. Change it with -DFPSLOCK=<n>
set(FPSLOCK 1 CACHE STRING "FPSLOCK setting: 1 or 0")
set_property(CACHE FPSLOCK PROPERTY STRINGS 1 0)
if(NOT FPSLOCK MATCHES "^(0|1)$")
    message(FATAL_ERROR "FPSLOCK has to be 0 or 1, not ${FPSLOCK}")
endif()

# 1 = the reachable floor is found with a floodfill and drawn, 0 = no floor, see
# FLOODFILLFLOOR in Defines.h. Change it with -DFLOODFILLFLOOR=<n>
set(FLOODFILLFLOOR 1 CACHE STRING "FLOODFILLFLOOR setting: 1 or 0")
set_property(CACHE FLOODFILLFLOOR PROPERTY STRINGS 1 0)
if(NOT FLOODFILLFLOOR MATCHES "^(0|1)$")
    message(FATAL_ERROR "FLOODFILLFLOOR has to be 0 or 1, not ${FLOODFILLFLOOR}")
endif()

# 1 = the debug header is always shown, 0 = Up + Down shows and hides it, see FORCEDEBUG in
# Defines.h. Change it with -DFORCEDEBUG=<n>
set(FORCEDEBUG 0 CACHE STRING "FORCEDEBUG setting: 0 or 1")
set_property(CACHE FORCEDEBUG PROPERTY STRINGS 0 1)
if(NOT FORCEDEBUG MATCHES "^(0|1)$")
    message(FATAL_ERROR "FORCEDEBUG has to be 0 or 1, not ${FORCEDEBUG}")
endif()

set(GAME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source/blips_embedded")

# the game's files, not the ESPboy platform or its library (lib/ is not globbed)
file(GLOB GAME_SOURCES CONFIGURE_DEPENDS "${GAME_DIR}/*.cpp")
list(FILTER GAME_SOURCES EXCLUDE REGEX "PlatformESPboy\\.cpp$")

# the parts of LovyanGFX an SDL build needs, as in its examples_for_PC/CMake_SDL
file(GLOB LGFX_SOURCES CONFIGURE_DEPENDS
    "${LOVYANGFX_DIR}/src/lgfx/Fonts/efont/*.c"
    "${LOVYANGFX_DIR}/src/lgfx/Fonts/IPA/*.c"
    "${LOVYANGFX_DIR}/src/lgfx/utility/*.c"
    "${LOVYANGFX_DIR}/src/lgfx/v1/*.cpp"
    "${LOVYANGFX_DIR}/src/lgfx/v1/misc/*.cpp"
    "${LOVYANGFX_DIR}/src/lgfx/v1/panel/Panel_Device.cpp"
    "${LOVYANGFX_DIR}/src/lgfx/v1/panel/Panel_FrameBufferBase.cpp"
    "${LOVYANGFX_DIR}/src/lgfx/v1/platforms/sdl/*.cpp")

# the static SDL2 library, so the exe needs no SDL2.dll next to it
find_package(SDL2 REQUIRED COMPONENTS SDL2-static)

# WIN32: a Windows (GUI) program, started it opens no console window next to the game's own
add_executable(blips WIN32 ${GAME_SOURCES} ${LGFX_SOURCES})
target_compile_definitions(blips PRIVATE PLATFORM_SDL LGFX_SDL SCREENBUFFER=${SCREENBUFFER} FPSLOCK=${FPSLOCK} FORCESKIN=${FORCESKIN} FORCEDEBUG=${FORCEDEBUG} FLOODFILLFLOOR=${FLOODFILLFLOOR} IMAGESET=${IMAGESET})
target_include_directories(blips PRIVATE "${GAME_DIR}" "${LOVYANGFX_DIR}/src")
target_link_libraries(blips PRIVATE SDL2::SDL2-static)

if(MINGW)
    # a fully static exe: libgcc, libstdc++ and winpthread are linked in as well, it only
    # needs the dlls every Windows has
    target_link_options(blips PRIVATE -static)
    # LovyanGFX's Chinese, Japanese and Korean fonts are over 40 MB and the game uses none of them:
    # every function and piece of data gets a section of its own, and the linker leaves out the
    # sections nothing uses. Except in a Debug build the symbols are left out as well
    target_compile_options(blips PRIVATE -ffunction-sections -fdata-sections)
    target_link_options(blips PRIVATE -Wl,--gc-sections $<$<NOT:$<CONFIG:Debug>>:-s>)
endif()
