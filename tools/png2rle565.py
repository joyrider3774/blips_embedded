#!/usr/bin/env python3
"""Convert the full screen skin images to run length encoded RGB565 flash data headers.

The raw RGB565 full screen images (5 per skin, 32KB each) take more flash than the
smaller devices have, so they are stored compressed and drawn with pushImageRLE()
in GameFuncs.cpp.
The arrays are marked PLATFORM_PROGMEM, the game's Platform.h says what that is.

Stream format, repeated until width * height pixels are produced:
  control byte c
    c & 0x80 : run,     (c & 0x7F) + 1 pixels of the one RGB565 LE pixel that follows
    else     : literal, c + 1 RGB565 LE pixels follow

Usage: python png2rle565.py [skins_dir] [images_dir]
defaults to ../assets/skins and ../source/blips_embedded/images next to this script
"""
import os
import sys
from PIL import Image

FULLSCREEN_IMAGES = ["background", "instructions", "intro1", "intro2", "titlescreen"]
SKIN_PREFIX = {"default": "default", "black_white": "black_white"}
MAX_COUNT = 128


def to_rgb565(path):
    img = Image.open(path).convert("RGB")
    rgb = img.tobytes()
    pixels = [((rgb[i] >> 3) << 11) | ((rgb[i + 1] >> 2) << 5) | (rgb[i + 2] >> 3) for i in range(0, len(rgb), 3)]
    return img.width, img.height, pixels


def rle_encode(pixels):
    out = bytearray()
    literal = []

    def flush_literal():
        while literal:
            chunk = literal[:MAX_COUNT]
            del literal[:MAX_COUNT]
            out.append(len(chunk) - 1)
            for p in chunk:
                out.extend(p.to_bytes(2, "little"))

    i = 0
    while i < len(pixels):
        run = 1
        while i + run < len(pixels) and run < MAX_COUNT and pixels[i + run] == pixels[i]:
            run += 1
        if run >= 2:
            flush_literal()
            out.append(0x80 | (run - 1))
            out += pixels[i].to_bytes(2, "little")
        else:
            literal.append(pixels[i])
        i += run
    flush_literal()
    return bytes(out)


def rle_decode(data, count):
    pixels = []
    i = 0
    while len(pixels) < count:
        c = data[i]
        n = (c & 0x7F) + 1
        i += 1
        if c & 0x80:
            pixels += [int.from_bytes(data[i:i + 2], "little")] * n
            i += 2
        else:
            pixels += [int.from_bytes(data[i + k * 2:i + k * 2 + 2], "little") for k in range(n)]
            i += n * 2
    return pixels


def write_header(path, source_name, var, width, height, data):
    lines = [
        "// Generated from: %s by tools/png2rle565.py" % source_name,
        "// Format: RGB565_LE, run length encoded, draw with pushImageRLE()",
        "// Original size: %dx%d = %d bytes, encoded: %d bytes" % (width, height, width * height * 2, len(data)),
        "",
        "const uint16_t %s_width = %d;" % (var, width),
        "const uint16_t %s_height = %d;" % (var, height),
        "",
        "const uint8_t %s_rle[] PLATFORM_PROGMEM = {" % var,
    ]
    for off in range(0, len(data), 16):
        lines.append("    " + ", ".join("0x%02X" % b for b in data[off:off + 16]) + ",")
    lines.append("};")
    with open(path, "w", newline="\n") as f:
        f.write("\n".join(lines) + "\n")


def main():
    here = os.path.dirname(os.path.abspath(__file__))
    skins_dir = sys.argv[1] if len(sys.argv) > 1 else os.path.join(here, "..", "assets", "skins")
    images_dir = sys.argv[2] if len(sys.argv) > 2 else os.path.join(here, "..", "source", "blips_embedded", "images")
    for skin, prefix in SKIN_PREFIX.items():
        for name in FULLSCREEN_IMAGES:
            width, height, pixels = to_rgb565(os.path.join(skins_dir, skin, name + ".png"))
            data = rle_encode(pixels)
            assert rle_decode(data, len(pixels)) == pixels
            out = os.path.join(images_dir, skin, name + "_RLE565.h")
            write_header(out, name + ".png", "%s_%s" % (prefix, name), width, height, data)
            print("%-8s %-12s %6d -> %6d bytes" % (skin, name, len(pixels) * 2, len(data)))


if __name__ == "__main__":
    main()
