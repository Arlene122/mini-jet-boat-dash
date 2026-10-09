/**
 * glass_bg — the ripple drifts 1 px per tick at RIPPLE_TICK_MS. Each tick
 * repaints the screen (tiled image), so keep it slow; on the P4 this is
 * the main cost of the Glass theme.
 */
#include "glass_bg.h"

#include "ui.h"
#include "ui_layout.h"
#include "ui_util.h"

LV_IMAGE_DECLARE(glass_bg_tile);
LV_IMAGE_DECLARE(glass_ripple);
LV_IMAGE_DECLARE(glass_plate_l);
LV_IMAGE_DECLARE(glass_plate_r);
LV_IMAGE_DECLARE(glass_disc);

/* ---------- Config ---------- */

#define RIPPLE_TICK_MS  220      /* slow drift */
#define DISC_R          (GAUGE_R - 34)

/* ---------- State ---------- */

static lv_obj_t * s_ripple;
static lv_timer_t * s_timer;
static int32_t s_off;

/* ---------- Helpers ---------- */

static lv_obj_t * tiled(lv_obj_t * parent, const lv_image_dsc_t * src)
{
    lv_obj_t * img = lv_image_create(parent);
    lv_image_set_src(img, src);
    lv_image_set_inner_align(img, LV_IMAGE_ALIGN_TILE);
    lv_obj_set_size(img, UI_HOR_RES, UI_VER_RES);
    lv_obj_set_pos(img, 0, 0);
    return img;
}

static lv_obj_t * plate(lv_obj_t * parent, const lv_image_dsc_t * src, int32_t x, int32_t y)
{
    lv_obj_t * img = lv_image_create(parent);
    lv_image_set_src(img, src);
    lv_obj_set_pos(img, x, y);
    return img;
}

static void drift_cb(lv_timer_t * t)
{
    LV_UNUSED(t);
    s_off++;
    lv_image_set_offset_x(s_ripple, s_off);
    lv_image_set_offset_y(s_ripple, s_off / 2);
}

static void delete_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    if(s_timer) lv_timer_delete(s_timer);
    s_timer = NULL;
    s_ripple = NULL;
}

/* ---------- API ---------- */

void glass_bg_create(lv_obj_t * scr)
{
    lv_obj_t * root = ui_box(scr, 0, 0, UI_HOR_RES, UI_VER_RES);
    tiled(root, &glass_bg_tile);
    s_ripple = tiled(root, &glass_ripple);
    plate(root, &glass_plate_l, ENGINE_X - 16, SIDE_Y1);   /* plates are 16 px wider outward */
    plate(root, &glass_plate_r, PAGE_X, SIDE_Y1);
    plate(root, &glass_disc, GAUGE_CX - DISC_R, GAUGE_CY - DISC_R);
    lv_obj_add_event_cb(root, delete_cb, LV_EVENT_DELETE, NULL);
    s_timer = lv_timer_create(drift_cb, RIPPLE_TICK_MS, NULL);
}
