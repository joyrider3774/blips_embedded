# libretro core of Blips: the game's own sources with the PLATFORM_LIBRETRO platform
# (source/blips_embedded/PlatformLibretro.*), for RetroArch and the other libretro frontends. Needs
# libretro.h from libretro-common (https://github.com/libretro/libretro-common).
#
# With MSYS2 (mingw64 packages cmake, ninja and gcc), from this folder:
#   cmake -S . -B build -G Ninja
#   cmake --build build
# gives build/blips_libretro.dll. Copy it into RetroArch's cores folder and blips_libretro.info into its
# info folder, then start it with "Load Core" and "Start Core". On Linux the same gives
# blips_libretro.so.

cmake_minimum_required(VERSION 3.16)
project(blips_libretro C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(LIBRETRO_COMMON_DIR "C:/github/libretro-common" CACHE PATH "libretro-common folder")
if(NOT EXISTS "${LIBRETRO_COMMON_DIR}/include/libretro.h")
    message(FATAL_ERROR "libretro.h not found in ${LIBRETRO_COMMON_DIR}/include, pass -DLIBRETRO_COMMON_DIR=<folder>")
endif()

# the game's folder, next to this one unless a build copies this folder somewhere else
set(GAME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../source/blips_embedded" CACHE PATH "the game's source folder")

# where drawing goes, see SCREENBUFFER in PlatformLibretro.h: 1, 8 or 16
set(SCREENBUFFER 16 CACHE STRING "SCREENBUFFER setting: 1, 8 or 16")
set_property(CACHE SCREENBUFFER PROPERTY STRINGS 1 8 16)
if(NOT SCREENBUFFER MATCHES "^(1|8|16)$")
    message(FATAL_ERROR "SCREENBUFFER has to be 1, 8 or 16, not ${SCREENBUFFER}")
endif()

# the image set, 1 (16x16 tiles) or 2 (8x8 tiles), see IMAGESET in defines.h
set(IMAGESET 2 CACHE STRING "IMAGESET setting: 1 or 2")

# -1 = every skin, n = skin n, with a 1 bpp buffer always the black & white one, see FORCESKIN in
# defines.h
set(FORCESKIN -1 CACHE STRING "FORCESKIN setting")

# 1 = the debug header is always shown, 0 = Up + Down shows and hides it, see FORCEDEBUG in defines.h
set(FORCEDEBUG 0 CACHE STRING "FORCEDEBUG setting: 0 or 1")

# the game's files, not the ESPboy platform or its library (lib/ is not globbed)
file(GLOB GAME_SOURCES CONFIGURE_DEPENDS "${GAME_DIR}/*.cpp")
list(FILTER GAME_SOURCES EXCLUDE REGEX "PlatformESPboy\\.cpp$")

add_library(blips_libretro SHARED ${GAME_SOURCES})
# the name frontends look for: blips_libretro.dll / .so, without the lib gcc puts in front
set_target_properties(blips_libretro PROPERTIES PREFIX "" POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(blips_libretro PRIVATE PLATFORM_LIBRETRO LIBRETRO_GAME_NAME="Blips" LIBRETRO_GAME_VERSION="1.0"
    SCREENBUFFER=${SCREENBUFFER} FORCESKIN=${FORCESKIN} FORCEDEBUG=${FORCEDEBUG} IMAGESET=${IMAGESET})
target_include_directories(blips_libretro PRIVATE "${GAME_DIR}" "${LIBRETRO_COMMON_DIR}/include")
target_compile_options(blips_libretro PRIVATE -ffunction-sections -fdata-sections)

if(MINGW)
    # libgcc, libstdc++ and winpthread linked in: the dll needs nothing from MSYS2 next to it
    target_link_options(blips_libretro PRIVATE -static -Wl,--gc-sections $<$<NOT:$<CONFIG:Debug>>:-s>)
elseif(UNIX AND NOT APPLE)
    # only the retro_ functions are exported
    target_link_options(blips_libretro PRIVATE -Wl,--gc-sections -Wl,--no-undefined)
endif()

# the core's info file next to the core
add_custom_command(TARGET blips_libretro POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/blips_libretro.info" "$<TARGET_FILE_DIR:blips_libretro>/blips_libretro.info")
