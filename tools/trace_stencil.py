#!/usr/bin/env python3
"""Rebuild assets/stencil.png from the bezel outline traced (by eye) from the
owner's stencil drawing. Corners are sharp vertices in drawing pixels; the
outline is rounded by CORNER_R and scaled to fill the 1920 px screen width.
Replace with a direct SVG conversion once the SVG file is in assets/.
"""
import math
import os

from PIL import Image, ImageDraw

W, H = 1920, 720
SS = 4                      # supersampling
CORNER_R = 40               # drawing px
# Sharp corners (drawing px): top-left, left point, bottom-left,
# bottom-right, right point, top-right
PTS = [(311, 407), (63, 705), (302, 1005), (1715, 1005), (1926, 577), (1785, 407)]
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "assets", "stencil.png")


def inset(pts, r):
    """Offset a convex CCW/CW polygon inward by r (intersect shifted edges)."""
    n = len(pts)
    cx = sum(p[0] for p in pts) / n
    cy = sum(p[1] for p in pts) / n
    lines = []
    for i in range(n):
        (x1, y1), (x2, y2) = pts[i], pts[(i + 1) % n]
        dx, dy = x2 - x1, y2 - y1
        ln = math.hypot(dx, dy)
        nx, ny = -dy / ln, dx / ln
        mx, my = (x1 + x2) / 2, (y1 + y2) / 2
        if (cx - mx) * nx + (cy - my) * ny < 0:
            nx, ny = -nx, -ny
        lines.append(((x1 + nx * r, y1 + ny * r), (dx, dy)))
    out = []
    for i in range(n):
        (p, d), (q, e) = lines[i - 1], lines[i]
        den = d[0] * e[1] - d[1] * e[0]
        t = ((q[0] - p[0]) * e[1] - (q[1] - p[1]) * e[0]) / den
        out.append((p[0] + d[0] * t, p[1] + d[1] * t))
    return out


def main():
    xs = [p[0] for p in PTS]
    ys = [p[1] for p in PTS]
    ox, oy = min(xs) - 10, min(ys) - 10
    cw, ch = (max(xs) - ox + 10) * SS, (max(ys) - oy + 10) * SS
    mask = Image.new("L", (int(cw), int(ch)), 0)
    d = ImageDraw.Draw(mask)
    core = [((x - ox) * SS, (y - oy) * SS) for x, y in inset(PTS, CORNER_R)]
    r = CORNER_R * SS
    d.polygon(core, fill=255)
    for i, p in enumerate(core):
        q = core[(i + 1) % len(core)]
        d.line([p, q], fill=255, width=int(2 * r))
        d.ellipse((p[0] - r, p[1] - r, p[0] + r, p[1] + r), fill=255)
    mask = mask.crop(mask.getbbox())
    scale = W / mask.width
    mh = round(mask.height * scale)
    mask = mask.resize((W, mh), Image.LANCZOS)
    full = Image.new("L", (W, H), 0)
    full.paste(mask, (0, (H - mh) // 2))
    img = Image.new("RGBA", (W, H), (14, 16, 19, 255))
    img.putalpha(full.point(lambda v: 255 - v))
    img.save(OUT, optimize=True)
    print("stencil.png written; visible band y %d..%d" % ((H - mh) // 2, (H - mh) // 2 + mh))


if __name__ == "__main__":
    main()
