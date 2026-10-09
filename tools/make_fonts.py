#!/usr/bin/env python3
"""Generate the Glass-theme LVGL fonts from assets/fonts (needs `lv_font_conv`,
npm i -g lv_font_conv).

  make_fonts.py [family]      family = outfit (default) | sora | inter

Writes src/ui/fonts/glass/:
  glass_text_<16..48>.c   UI text + LVGL symbols (Font Awesome), weight 500
  glass_digits_<n>.c      speed / RPM digits, weight 600
  cmp_<fam>_*.c           small comparison set for all three families
All digits are made TABULAR (equal advance) so numbers don't jiggle.
"""
import os
import re
import subprocess
import sys

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
SRC = os.path.join(ROOT, "assets", "fonts")
OUT = os.path.join(ROOT, "src", "ui", "fonts", "glass")
FA = os.path.join(SRC, "FontAwesome5.woff")
# Symbols LVGL's built-in fonts carry (LV_SYMBOL_*)
SYMS = ("61441,61448,61451,61452,61453,61457,61459,61461,61465,61468,61473,61478,61479,61480,"
        "61502,61507,61512,61515,61516,61517,61521,61522,61523,61524,61543,61544,61550,61552,"
        "61553,61556,61559,61560,61561,61563,61587,61589,61636,61637,61639,61641,61664,61671,"
        "61674,61683,61724,61732,61787,61931,62016,62017,62018,62019,62020,62087,62099,62212,"
        "62189,62810,63426,63650")
TEXT_RANGE = "0x20-0x7E,0xB0,0x2022"
DIGIT_RANGE = "0x20,0x25,0x2D-0x3A"
TEXT_SIZES = [16, 20, 24, 28, 32, 40, 48]
DIGIT_SIZES = [160, 72, 34]


def conv(name, size, font, rng, symbols):
    path = os.path.join(OUT, name + ".c")
    cmd = ["lv_font_conv", "--font", font, "-r", rng]
    if symbols:
        cmd += ["--font", FA, "-r", SYMS]
    cmd += ["--size", str(size), "--bpp", "4", "--format", "lvgl", "--no-compress", "--no-kerning",
            "--lv-font-name", name, "--lv-include", "lvgl.h", "-o", path]
    subprocess.run(cmd, check=True, cwd=SRC)
    tabular(path)
    # keep the generated header free of local paths
    txt = open(path).read()
    txt = re.sub(r"--font \S*/", "--font ", txt)
    open(path, "w").write(txt)


def tabular(path):
    """Give digits 0-9 the same advance width (centre each glyph)."""
    txt = open(path).read()
    codes = [int(c, 16) for c in re.findall(r"/\* U\+([0-9A-F]+) ", txt)]
    m = re.search(r"glyph_dsc\[\] = \{(.*?)\n\};", txt, re.S)
    entries = re.findall(r"\{[^{}]*\}", m.group(1))
    glyphs = entries[1:]                       # id 0 is reserved
    digit_idx = [i for i, c in enumerate(codes) if 0x30 <= c <= 0x39]
    adv = lambda e: int(re.search(r"\.adv_w = (\d+)", e).group(1))
    maxw = max(adv(glyphs[i]) for i in digit_idx)
    for i in digit_idx:
        e = glyphs[i]
        d = maxw - adv(e)
        ox = int(re.search(r"\.ofs_x = (-?\d+)", e).group(1)) + round(d / 16 / 2)
        e2 = re.sub(r"\.adv_w = \d+", ".adv_w = %d" % maxw, e)
        e2 = re.sub(r"\.ofs_x = -?\d+", ".ofs_x = %d" % ox, e2)
        glyphs[i] = e2
    body = m.group(1)
    for old, new in zip(entries[1:], glyphs):
        body = body.replace(old, new, 1)
    open(path, "w").write(txt.replace(m.group(1), body))


def main():
    fam = sys.argv[1] if len(sys.argv) > 1 else "outfit"
    os.makedirs(OUT, exist_ok=True)
    for s in TEXT_SIZES:
        conv("glass_text_%d" % s, s, "%s-500.woff" % fam, TEXT_RANGE, True)
    for s in DIGIT_SIZES:
        conv("glass_digits_%d" % s, s, "%s-600.woff" % fam, DIGIT_RANGE, False)
    for f in ("outfit", "sora", "inter"):
        conv("cmp_%s_digits_160" % f, 160, "%s-600.woff" % f, DIGIT_RANGE, False)
        conv("cmp_%s_text_28" % f, 28, "%s-500.woff" % f, TEXT_RANGE, False)
    open(os.path.join(OUT, "FAMILY.txt"), "w").write(fam + "\n")
    print("fonts generated for", fam)


if __name__ == "__main__":
    main()
