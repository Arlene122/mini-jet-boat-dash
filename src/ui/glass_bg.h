/**
 * glass_bg — Glass theme backdrop: deep-water gradient, slowly drifting
 * low-contrast ripple, and frosted plates behind the side zones and the
 * speed ring. All pre-rendered images (tools/make_glass_assets.py): no
 * live blur. Created first so everything else draws on top.
 */
#ifndef GLASS_BG_H
#define GLASS_BG_H

#include "lvgl.h"

void glass_bg_create(lv_obj_t * scr);

#endif /* GLASS_BG_H */
