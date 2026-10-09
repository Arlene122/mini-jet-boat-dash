#!/usr/bin/env python3
"""Pre-render the Glass theme images (no live blur on the ESP32-P4).

Writes assets/glass/:
  bg_tile.png    64x720   deep-water gradient, dithered (tiled across)
  ripple.png     480x240  seamless, low-contrast caustics (tiled, drifts slowly)
  plate_l.png    frosted panel behind the engine stats
  plate_r.png    frosted panel behind the page zone
  disc.png       frosted round plate behind the speed ring
Geometry must match src/ui/ui_layout.h.
"""
import os

import numpy as np
from PIL import Image, ImageFilter

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "assets", "glass")
W, H = 1920, 720
# ui_layout.h
PAGE_X, PAGE_W, SIDE_Y1, SIDE_H = 1345, 385, 132, 456
ENGINE_X = W - PAGE_X - PAGE_W
PLATE_GROW, PLATE_DROP = 16, 12   # outward / downward margin around the page text
GAUGE_CX, GAUGE_CY, DISC_R = 960, 360, 202

NAVY_950, NAVY_900, OCEAN_700, AQUA_200 = (0x04, 0x0A, 0x14), (0x07, 0x14, 0x26), (0x12, 0x3A, 0x66), (0x9C, 0xEF, 0xF0)
rng = np.random.default_rng(7)


def lerp(a, b, t):
    return np.array(a, float) * (1 - t) + np.array(b, float) * t


def gradient_column():
    y = np.linspace(0, 1, H)[:, None]
    top = lerp(NAVY_950, NAVY_900, np.clip(y / 0.55, 0, 1))
    deep = lerp(NAVY_900, lerp(NAVY_900, OCEAN_700, 0.35), np.clip((y - 0.55) / 0.45, 0, 1))
    return np.where(y < 0.55, top, deep)          # (H,3)


def bg_full():
    col = gradient_column()
    return np.repeat(col[:, None, :], W, axis=1)  # (H,W,3)


def ripple_tile(tw=480, th=240):
    x = np.arange(tw)[None, :] / tw
    y = np.arange(th)[:, None] / th
    v = (np.sin(2 * np.pi * (3 * x + 2 * y)) + np.sin(2 * np.pi * (-2 * x + 3 * y) + 1.3)
         + np.sin(2 * np.pi * (5 * x - 1 * y) + 2.1) * 0.6 + np.sin(2 * np.pi * (1 * x + 4 * y) + 0.4) * 0.6)
    c = np.clip(1 - np.abs(v) / 1.2, 0, 1) ** 5     # thin bright caustic lines
    a = (c * 46).astype(np.uint8)                   # low contrast
    rgba = np.zeros((th, tw, 4), np.uint8)
    rgba[..., :3] = AQUA_200
    rgba[..., 3] = a
    return rgba


def composite(bg, tile):
    th, tw = tile.shape[:2]
    reps = (H // th + 1, W // tw + 1, 1)
    t = np.tile(tile, reps)[:H, :W]
    a = t[..., 3:4] / 255.0
    return bg * (1 - a) + t[..., :3] * a


def frost(region, tint=0.07):
    img = Image.fromarray(np.clip(region, 0, 255).astype(np.uint8))
    img = img.filter(ImageFilter.GaussianBlur(18))
    f = np.array(img, float)
    f = lerp(f, (255, 255, 255), 0.0) * (1 - tint) + np.array(AQUA_200) * tint * 0.5 + 255 * tint * 0.5
    f += rng.normal(0, 2.0, f.shape)                # fine frost grain
    return f


def rounded_mask(w, h, r, ss=4):
    m = Image.new("L", (w * ss, h * ss), 0)
    from PIL import ImageDraw
    ImageDraw.Draw(m).rounded_rectangle((0, 0, w * ss - 1, h * ss - 1), r * ss, fill=255)
    return np.array(m.resize((w, h), Image.LANCZOS), float) / 255


def plate(full, x, y, w, h, r=26):
    f = frost(full[y:y + h, x:x + w])
    m = rounded_mask(w, h, r)
    inner = rounded_mask(w - 2, h - 2, r - 1)
    edge = np.clip(m - np.pad(inner, 1), 0, 1)      # 1 px rim
    yy = np.linspace(0, 1, h)[:, None]
    rim = edge * (0.10 + 0.22 * (1 - yy))            # brighter at the top
    f = f * (1 - rim[..., None]) + 255 * rim[..., None]
    rgba = np.dstack([np.clip(f, 0, 255), m * 225]).astype(np.uint8)
    return Image.fromarray(rgba, "RGBA")


def disc(full):
    d = DISC_R * 2
    x0, y0 = GAUGE_CX - DISC_R, GAUGE_CY - DISC_R
    f = frost(full[y0:y0 + d, x0:x0 + d], tint=0.05)
    yy, xx = np.mgrid[0:d, 0:d]
    rr = np.hypot(xx - DISC_R + 0.5, yy - DISC_R + 0.5)
    m = np.clip(DISC_R - rr, 0, 1)
    shade = 0.10 * np.clip(1 - yy / d, 0, 1)        # light from above
    f = f * (1 - shade[..., None]) + 255 * shade[..., None]
    rim = np.clip(1 - np.abs(rr - (DISC_R - 1.5)), 0, 1) * 0.18
    f = f * (1 - rim[..., None]) + 255 * rim[..., None]
    rgba = np.dstack([np.clip(f, 0, 255), m * 215]).astype(np.uint8)
    return Image.fromarray(rgba, "RGBA")


def main():
    os.makedirs(OUT, exist_ok=True)
    col = gradient_column()
    tile = np.repeat(col[:, None, :], 64, axis=1) + rng.uniform(-0.6, 0.6, (H, 64, 3))   # dither
    Image.fromarray(np.clip(tile, 0, 255).astype(np.uint8), "RGB").save(os.path.join(OUT, "bg_tile.png"), optimize=True)
    rt = ripple_tile()
    Image.fromarray(rt, "RGBA").save(os.path.join(OUT, "ripple.png"), optimize=True)
    full = composite(bg_full(), rt)
    plate(full, ENGINE_X - PLATE_GROW, SIDE_Y1, PAGE_W + PLATE_GROW, SIDE_H + PLATE_DROP).save(os.path.join(OUT, "plate_l.png"), optimize=True)
    plate(full, PAGE_X, SIDE_Y1, PAGE_W + PLATE_GROW, SIDE_H + PLATE_DROP).save(os.path.join(OUT, "plate_r.png"), optimize=True)
    disc(full).save(os.path.join(OUT, "disc.png"), optimize=True)
    print("glass assets written")


if __name__ == "__main__":
    main()
