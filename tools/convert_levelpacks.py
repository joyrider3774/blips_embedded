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
DEFINES = os.path.join(ROOT, "Source", "blips_embedded", "Defines.h")
#the region of Defines.h this tool owns, see build_switches()
BEGIN = "//>>> written by tools/convert_levelpacks.py from assets/levelpacks, do not edit by hand"
END = "//<<<"
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


MAX_RUN = 128


def rle(data):
    """The control byte scheme of png2rle565.py, over the bytes of a level pack.

        c & 0x80 : a run,     (c & 0x7F) + 1 copies of the byte that follows
        else     : a literal, c + 1 bytes follow

    Level text is mostly runs of wall and of floor, so this takes about 45% off a pack. A run of
    two is left as a literal: it would cost the same and reading it is more work."""
    out = bytearray()
    literal = []

    def flush():
        while literal:
            chunk = literal[:MAX_RUN]
            del literal[:MAX_RUN]
            out.append(len(chunk) - 1)
            out.extend(chunk)

    i = 0
    while i < len(data):
        run = 1
        while i + run < len(data) and run < MAX_RUN and data[i + run] == data[i]:
            run += 1
        if run >= 3:
            flush()
            out.append(0x80 | (run - 1))
            out.append(data[i])
            i += run
        else:
            literal.append(data[i])
            i += 1
    flush()
    return bytes(out)


def unrle(data):
    """What the reader in CLevelPackFile.cpp makes of it, so nothing is written that it cannot read."""
    out = bytearray()
    i = 0
    while i < len(data):
        control = data[i]
        i += 1
        if control & 0x80:
            out.extend(bytes([data[i]]) * ((control & 0x7F) + 1))
            i += 1
        else:
            count = control + 1
            out.extend(data[i:i + count])
            i += count
    return bytes(out)


def build_header(packs):
    blocks = []
    for file_name, data in packs:
        encoded = rle(data)
        #never write a pack that does not come back out of the reader as it went in
        assert unrle(encoded) == data, file_name
        rows = ["  " + ", ".join("0x%02x" % b for b in encoded[off:off + BYTES_PER_LINE])
                for off in range(0, len(encoded), BYTES_PER_LINE)]
        blocks.append("\n".join([
            "// %s, %d bytes of level text run length encoded to %d" % (file_name, len(data), len(encoded)),
            "const unsigned char %s[] PLATFORM_PROGMEM  = {" % c_name(file_name),
            ", \n".join(rows),
            "};",
        ]))
    return ("#ifndef LEVELPACKS_H\n#define LEVELPACKS_H\n" + "\n\n".join(blocks) + "\n\n"
            + build_table(packs) + "\n\n#endif")


def busiest_level(data):
    """How many parts the busiest level of a pack has.

    This walks a pack the way CLevelPackFile_parseText does: a level starts at the first line
    with a wall in it, a line with a colon in it is a metadata field and not a row of the
    level, an empty line ends it, and a part is anything that is not floor."""
    inlevel = False
    parts = 0
    most = 0
    for line in data.decode("latin-1").split("\n"):
        row = line.rstrip("\r")
        if inlevel and not row.strip():
            inlevel = False
            most = max(most, parts)
            continue
        if inlevel and ":" in row:
            continue
        if (not inlevel) and (":" not in row) and ("#" in row):
            inlevel = True
            parts = 0
        if inlevel:
            parts += sum(1 for ch in row if ch != " ")
    return max(most, parts)


def build_switches(packs):
    """The LEVELPACKS switch: which of the packs a build takes.

    A pack that is left out is named nowhere, so the compiler drops its array and the game does
    not offer it. Which packs there are is what lies in assets/levelpacks, the same list the
    arrays are written from, so this is written from there and not kept by hand."""
    names = [c_name(file_name)[len("levelpack_"):] for file_name, _ in packs]
    pad = max(len(n) for n in names)
    file_pad = max(len(f) for f, _ in packs)
    lines = [
        "//LEVELPACKS: the level packs that are built in, an LP_ bit each (the size is what the",
        "//pack takes in flash). All of them unless the device header or the build picks fewer; a",
        "//pack that is left out takes no flash and is not offered in the game",
    ]
    for i, ((file_name, data), name) in enumerate(zip(packs, names)):
        lines.append("#define LP_%-*s (1ul << %2d)    //%-*s %6d bytes"
                     % (pad, name, i, file_pad, file_name, len(data)))
    lines += [
        "#define LP_ALL ((1ul << %d) - 1)" % len(packs),
        "#ifndef LEVELPACKS",
        "#define LEVELPACKS LP_ALL",
        "#endif",
        "#if (LEVELPACKS & LP_ALL) == 0",
        '#error "LEVELPACKS has to leave at least one level pack in"',
        "#endif",
        "//how many packs there are in all, which is what the saved unlocks are sized by",
        "#define MaxLevelPacks %d" % len(packs),
        "//how many of them this build takes, which is how many the game lists",
        "#define LEVELPACKCOUNT ("
        #on one line: a macro spanning lines would need a continuation on each of them
        + " + ".join("((LEVELPACKS & LP_%s) != 0)" % n for n in names) + ")",
        "",
        "//The busiest level each pack has. The game keeps a pool of world parts and it is the",
        "//largest thing it asks the heap for, so a build wants no more slots than the packs it",
        "//holds can fill, see MAXWORLDPARTS in the device header",
    ]
    for (file_name, data), name in zip(packs, names):
        lines.append("#define LP_PARTS_%-*s %4d" % (pad, name, busiest_level(data)))
    most = "0"
    for name in names:
        most = "LP_PARTS_MAX(%s, LP_PARTS_OF(%s))" % (most, name)
    lines += [
        "",
        "//how many parts the busiest level of the packs this build holds has. A pack that is",
        "//left out counts for nothing, so the count follows what LEVELPACKS says",
        "//One comparison a pack. LP_PARTS_MAX is a function and not a macro on purpose: a macro",
        "//naming its first argument twice doubles the text at every step, which with nineteen",
        "//packs put the compiler out of memory. constexpr keeps it usable where a constant is",
        "//wanted, such as the static_assert below and the size of the pool",
        "static inline constexpr int LP_PARTS_MAX(int a, int b) { return (a > b) ? a : b; }",
        "#define LP_PARTS_OF(p) (((LEVELPACKS & LP_##p) != 0) ? LP_PARTS_##p : 0)",
        "#define LEVELPACKMAXPARTS %s" % most,
    ]
    return "\n".join(lines)


def build_table(packs):
    """The table of the packs a build takes: the name the game shows and the data to parse.

    Only the packs LEVELPACKS asks for are named, and an array that is named nowhere is one the
    compiler leaves out, which is the whole of what leaving a pack out saves."""
    names = [c_name(file_name) for file_name, _ in packs]
    lines = [
        "//every line of a level is at least two bytes (a character and the newline), so the line",
        "//counter y (uint16_t) can not wrap while a pack stays below 131072 bytes. The arrays are",
        "//run length encoded, so it is the length they come back to that is checked, and that is",
        "//known here rather than to the compiler",
        "static_assert(" + " &&\n\t".join("(%d < 131072)" % len(data) for _, data in packs)
        + ', "a level pack is too big for the uint16_t line counter");',
        "",
        "//The packs LEVELPACKS builds in: the name the game offers and the text to parse. Only the",
        "//packs that are asked for are named here, and an array that is named nowhere is one the",
        "//compiler leaves out, which is what leaving a pack out saves",
        "typedef struct LevelPackEntry LevelPackEntry;",
        "struct LevelPackEntry",
        "{",
        "\tconst char* filename;",
        "\tconst unsigned char* text;",
        "\tuint32_t size;",
        "};",
        "",
        "static const LevelPackEntry builtInPacks[] = {",
    ]
    for (file_name, _), name in zip(packs, names):
        lines.append("#if LEVELPACKS & LP_%s" % name[len("levelpack_"):])
        lines.append('\t{ "%s", %s, sizeof(%s) },' % (file_name, name, name))
        lines.append("#endif")
    lines.append("};")
    return "\n".join(lines)


def splice(text, block):
    """Puts block between the markers, which is the part of Defines.h this tool writes."""
    start = text.find(BEGIN)
    end = text.find(END, start + 1) if start >= 0 else -1
    if start < 0 or end < 0:
        raise SystemExit("the markers are gone from %s, put them back" % DEFINES)
    return text[:start] + BEGIN + "\n" + block + "\n" + text[end:]


def parse_header(text):
    """What the game gets out of a Levelpacks.h: every array, as text and as bytes."""
    arrays = {}
    for m in re.finditer(r"(// [^\n]+\n)const unsigned char (\w+)\[\] PLATFORM_PROGMEM\s+= \{\n(.*?)\n\};", text, re.S):
        stored = bytes(int(x, 16) for x in re.findall(r"0x([0-9A-Fa-f]{2})", m.group(3)))
        arrays[m.group(2)] = {
            "text": m.group(0),
            #what the game reads is the decoded pack, so that is what is compared
            "bytes": unrle(stored),
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

    switches = build_switches(packs)
    with open(DEFINES, "r", newline="") as f:
        defines_raw = f.read()
    defines_old = defines_raw.replace("\r\n", "\n")
    defines_new = splice(defines_old, switches)

    if args.verify:
        print("verifying %s against assets/levelpacks (%s)" % (os.path.relpath(args.output, ROOT), summary))
        ok = verify(text, args.output)
        if defines_new == defines_old:
            print("  the LEVELPACKS switch in %s matches" % os.path.basename(DEFINES))
        else:
            ok = False
            print("  the LEVELPACKS switch in %s differs:" % os.path.basename(DEFINES))
            for line in difflib.unified_diff(defines_old.split("\n"), defines_new.split("\n"),
                                             "current", "generated", lineterm="", n=1):
                print("      " + line)
        print("everything matches" if ok else "there are differences")
        return 0 if ok else 1

    with open(DEFINES, "w", newline="\r\n") as f:
        f.write(defines_new)
    print("wrote the LEVELPACKS switch into %s" % os.path.relpath(DEFINES, ROOT))

    # CRLF like the rest of the sketch sources, and no newline after #endif as before
    with open(args.output, "w", newline="\r\n") as f:
        f.write(text)
    print("wrote %s (%s)" % (os.path.relpath(args.output, ROOT), summary))
    return 0


if __name__ == "__main__":
    sys.exit(main())
