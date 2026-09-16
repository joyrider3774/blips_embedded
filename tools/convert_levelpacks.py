#!/usr/bin/env python3
"""Convert the level packs in assets/levelpacks to Source/blips_embedded/Levelpacks.h.

Every <name>.bip becomes one array, levelpack_<name>, holding the file byte for byte.
The arrays carry no terminator: CLevelPackFile_parseText is handed sizeof() of the
array, which is what stops one pack running into the next where the linker packs
them back to back in flash.

Packs are written in alphabetical order, matching the order CLevelPackFile_loadFile
names them in.

Usage:
  python convert_levelpacks.py            write Levelpacks.h
  python convert_levelpacks.py --verify   build Levelpacks.h in memory and compare it
                                          with the current one, nothing is written
  --output FILE                           write (or verify) this file instead
"""
import argparse
import difflib
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.join(HERE, "..")
PACKS_DIR = os.path.join(ROOT, "assets", "levelpacks")
OUTPUT = os.path.join(ROOT, "Source", "blips_embedded", "Levelpacks.h")
BYTES_PER_LINE = 16


def c_name(file_name):
    return "levelpack_" + re.sub(r"\W", "_", os.path.splitext(file_name)[0])


def read_packs(packs_dir):
    """[(file name, bytes), ...] in alphabetical order."""
    packs = []
    for file_name in sorted(os.listdir(packs_dir)):
        if not file_name.lower().endswith(".bip"):
            continue
        with open(os.path.join(packs_dir, file_name), "rb") as f:
            packs.append((file_name, f.read()))
    return packs


def build_header(packs):
    blocks = []
    for file_name, data in packs:
        rows = ["  " + ", ".join("0x%02x" % b for b in data[off:off + BYTES_PER_LINE])
                for off in range(0, len(data), BYTES_PER_LINE)]
        blocks.append("\n".join([
            "// array size is %d" % len(data),
            "const unsigned char %s[] PLATFORM_PROGMEM  = {" % c_name(file_name),
            ", \n".join(rows),
            "};",
        ]))
    return "#ifndef LEVELPACKS_H\n#define LEVELPACKS_H\n" + "\n\n".join(blocks) + "\n\n#endif"


def parse_header(text):
    """What the game gets out of a Levelpacks.h: every array, as text and as bytes."""
    arrays = {}
    for m in re.finditer(r"(// [^\n]+\n)const unsigned char (\w+)\[\] PLATFORM_PROGMEM\s+= \{\n(.*?)\n\};", text, re.S):
        arrays[m.group(2)] = {
            "text": m.group(0),
            "bytes": bytes(int(x, 16) for x in re.findall(r"0x([0-9A-Fa-f]{2})", m.group(3))),
        }
    return arrays


def verify(new_text, path):
    with open(path, "r", newline="") as f:
        old_raw = f.read()
    # both are compared with plain newlines, the line endings are checked separately
    old_text = old_raw.replace("\r\n", "\n")
    new_arrays, old_arrays = parse_header(new_text), parse_header(old_text)
    ok = True

    same = 0
    for name in sorted(set(new_arrays) | set(old_arrays)):
        if name not in old_arrays:
            ok = False
            print("  MISSING  %s is not in the current file" % name)
        elif name not in new_arrays:
            ok = False
            print("  EXTRA    %s is in the current file but made from no .bip file" % name)
        elif new_arrays[name]["bytes"] != old_arrays[name]["bytes"]:
            ok = False
            print("  DIFFERS  %s holds other level data" % name)
        elif new_arrays[name]["text"] != old_arrays[name]["text"]:
            ok = False
            print("  DIFFERS  %s has the same bytes but is formatted differently" % name)
        else:
            same += 1
            print("  identical  %-32s %6d bytes" % (name, len(new_arrays[name]["bytes"])))
    print("  level pack arrays: %d of %d identical (data and text)" % (same, len(new_arrays)))

    if "\r\n" not in old_raw:
        print("  note: the current file does not use CRLF line endings, a written one would")

    if new_text == old_text:
        print("  whole file: identical")
    else:
        print("  whole file: differs outside the arrays, this has no effect on the game:")
        for line in difflib.unified_diff(old_text.split("\n"), new_text.split("\n"),
                                         "current", "generated", lineterm="", n=1):
            print("      " + line)
    return ok


def main():
    parser = argparse.ArgumentParser(description="Convert assets/levelpacks to Levelpacks.h")
    parser.add_argument("--verify", action="store_true", help="compare with the current Levelpacks.h, write nothing")
    parser.add_argument("--output", default=OUTPUT, help="Levelpacks.h to write or verify")
    args = parser.parse_args()

    packs = read_packs(PACKS_DIR)
    text = build_header(packs)
    summary = ", ".join("%s %d bytes" % (file_name, len(data)) for file_name, data in packs)

    if args.verify:
        print("verifying %s against assets/levelpacks (%s)" % (os.path.relpath(args.output, ROOT), summary))
        ok = verify(text, args.output)
        print("everything matches" if ok else "there are differences")
        return 0 if ok else 1

    # CRLF like the rest of the sketch sources, and no newline after #endif as before
    with open(args.output, "w", newline="\r\n") as f:
        f.write(text)
    print("wrote %s (%s)" % (os.path.relpath(args.output, ROOT), summary))
    return 0


if __name__ == "__main__":
    sys.exit(main())
