/**
 * glass_bg — one static pre-rendered background (navy gradient + carbon
 * fibre fading out toward the edges) plus frosted plates. Nothing animates,
 * so it only costs a blit when the area under it redraws.
 */
#include "glass_bg.h"

#include "key_on.h"
#include "ui.h"
#include "ui_layout.h"
#include "ui_util.h"

LV_IMAGE_DECLARE(glass_bg);
LV_IMAGE_DECLARE(glass_plate_l);
LV_IMAGE_DECLARE(glass_plate_r);
LV_IMAGE_DECLARE(glass_disc);

/* ---------- Config ---------- */

#define DISC_R          (GAUGE_R - 34)

/* ---------- Helpers ---------- */

static lv_obj_t * plate(lv_obj_t * parent, const lv_image_dsc_t * src, int32_t x, int32_t y)
{
    lv_obj_t * img = lv_image_create(parent);
    lv_image_set_src(img, src);
    lv_obj_set_pos(img, x, y);
    return img;
}

/* ---------- API ---------- */

void glass_bg_create(lv_obj_t * scr)
{
    key_on_add(KO_BG, plate(scr, &glass_bg, 0, 0));
    key_on_add(KO_SIDE, plate(scr, &glass_plate_l, ENGINE_X - 16, SIDE_Y1));   /* plates are 16 px wider outward */
    key_on_add(KO_SIDE, plate(scr, &glass_plate_r, PAGE_X, SIDE_Y1));
    key_on_add(KO_GAUGE, plate(scr, &glass_disc, GAUGE_CX - DISC_R, GAUGE_CY - DISC_R));
}
