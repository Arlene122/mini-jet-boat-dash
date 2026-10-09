#!/usr/bin/env python3
"""Pre-render the Glass theme images (no live blur on the ESP32-P4).

Writes assets/glass/:
  bg.png         1920x720 navy gradient + subtle carbon-fibre twill that
                 fades out toward the screen edges (static, opaque)
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


# Carbon fibre: 2/2 twill of 6 px tows, each with a soft sheen across it.
CARBON_TOW = 6
CARBON_AMP = 15.0          # max brightness swing (0-255) at full strength
CARBON_TINT = (0x5A, 0x6A, 0x80)   # cool graphite highlight


def carbon_pattern():
    y, x = np.mgrid[0:H, 0:W]
    i, j = x // CARBON_TOW, y // CARBON_TOW
    u = (x % CARBON_TOW + 0.5) / CARBON_TOW
    v = (y % CARBON_TOW + 0.5) / CARBON_TOW
    horiz = ((i + j) % 4) < 2
    across = np.where(horiz, v, u)
    sheen = np.sin(np.pi * across) ** 1.5                # bright tow middle, dark gaps
    gain = np.where(horiz, 1.0, 0.55)                    # light catches one direction more
    return sheen * gain - 0.35                           # roughly zero-mean


def carbon_fade():
    """1 near the speedometer, 0 at the outer screen (soft ellipse)."""
    y, x = np.mgrid[0:H, 0:W]
    d = np.hypot((x - GAUGE_CX) / (W * 0.46), (y - GAUGE_CY) / (H * 0.62))
    t = np.clip((d - 0.35) / 0.65, 0, 1)
    return (1 - t * t * (3 - 2 * t))                     # smoothstep falloff


def bg_carbon():
    bg = bg_full()
    k = (carbon_pattern() * carbon_fade() * CARBON_AMP)[..., None]
    tint = np.array(CARBON_TINT, float) / max(CARBON_TINT)
    out = bg + k * tint + rng.uniform(-0.6, 0.6, bg.shape)   # dither
    return np.clip(out, 0, 255)


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
    full = bg_carbon()
    Image.fromarray(full.astype(np.uint8), "RGB").save(os.path.join(OUT, "bg.png"), optimize=True)
    plate(full, ENGINE_X - PLATE_GROW, SIDE_Y1, PAGE_W + PLATE_GROW, SIDE_H + PLATE_DROP).save(os.path.join(OUT, "plate_l.png"), optimize=True)
    plate(full, PAGE_X, SIDE_Y1, PAGE_W + PLATE_GROW, SIDE_H + PLATE_DROP).save(os.path.join(OUT, "plate_r.png"), optimize=True)
    disc(full).save(os.path.join(OUT, "disc.png"), optimize=True)
    print("glass assets written")


if __name__ == "__main__":
    main()
