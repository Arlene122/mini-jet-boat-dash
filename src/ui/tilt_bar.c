/**
 * tilt_bar — centre rectangle (horizontal gradient) + two slanted end
 * triangles + a bright outer edge. Drawn once; redrawn on mode change.
 */
#include "tilt_bar.h"

#include "ui_theme.h"

/* ---------- Style ---------- */

#define TILT_DARK   lv_color_hex(0x0A0F16)
#define TILT_TINT   LV_OPA_20        /* accent mixed into the middle */

typedef struct {
    int32_t inset;
    bool narrow_bottom;
} tilt_bar_cfg_t;

/* ---------- Drawing ---------- */

static void draw_tri(lv_layer_t * layer, lv_draw_triangle_dsc_t * dsc,
                     int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    dsc->p[0].x = x0; dsc->p[0].y = y0;
    dsc->p[1].x = x1; dsc->p[1].y = y1;
    dsc->p[2].x = x2; dsc->p[2].y = y2;
    lv_draw_triangle(layer, dsc);
}

static void tilt_bar_draw_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target_obj(e);
    lv_layer_t * layer = lv_event_get_layer(e);
    const tilt_bar_cfg_t * cfg = lv_obj_get_user_data(obj);
    lv_color_t acc = ui_theme_accent();

    lv_area_t a;
    lv_obj_get_coords(obj, &a);
    int32_t y_outer = cfg->narrow_bottom ? a.y1 : a.y2;   /* wide edge */
    int32_t y_inner = cfg->narrow_bottom ? a.y2 : a.y1;   /* narrow edge */
    int32_t in = cfg->inset;

    /* Middle: dark -> tinted -> dark */
    lv_draw_rect_dsc_t rd;
    lv_draw_rect_dsc_init(&rd);
    rd.bg_grad.dir = LV_GRAD_DIR_HOR;
    rd.bg_grad.stops_count = 3;
    rd.bg_grad.stops[0].color = TILT_DARK;
    rd.bg_grad.stops[0].opa = LV_OPA_COVER;
    rd.bg_grad.stops[0].frac = 0;
    rd.bg_grad.stops[1].color = lv_color_mix(acc, TILT_DARK, TILT_TINT);
    rd.bg_grad.stops[1].opa = LV_OPA_COVER;
    rd.bg_grad.stops[1].frac = 128;
    rd.bg_grad.stops[2].color = TILT_DARK;
    rd.bg_grad.stops[2].opa = LV_OPA_COVER;
    rd.bg_grad.stops[2].frac = 255;
    lv_area_t mid = { a.x1 + in, a.y1, a.x2 - in, a.y2 };
    lv_draw_rect(layer, &rd, &mid);

    /* Slanted ends (1 px overlap hides the seam) */
    lv_draw_triangle_dsc_t td;
    lv_draw_triangle_dsc_init(&td);
    td.color = TILT_DARK;
    td.opa = LV_OPA_COVER;
    draw_tri(layer, &td, a.x1, y_outer, a.x1 + in + 1, y_outer, a.x1 + in + 1, y_inner);
    draw_tri(layer, &td, a.x2, y_outer, a.x2 - in - 1, y_outer, a.x2 - in - 1, y_inner);

    /* Bright line along the outer (wide) edge, fading at the ends */
    lv_draw_line_dsc_t ld;
    lv_draw_line_dsc_init(&ld);
    ld.color = acc;
    ld.width = 2;
    ld.opa = LV_OPA_70;
    ld.p1.x = a.x1 + in * 2; ld.p1.y = y_outer;
    ld.p2.x = a.x2 - in * 2; ld.p2.y = y_outer;
    lv_draw_line(layer, &ld);
}

static void tilt_bar_delete_cb(lv_event_t * e)
{
    lv_free(lv_obj_get_user_data(lv_event_get_target_obj(e)));
}

/* ---------- API ---------- */

lv_obj_t * tilt_bar_create(lv_obj_t * parent, int32_t x, int32_t y, int32_t w, int32_t h,
                           int32_t inset, bool narrow_bottom)
{
    lv_obj_t * obj = lv_obj_create(parent);
    lv_obj_remove_style_all(obj);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, w, h);
    lv_obj_set_scrollable(obj, false);
    lv_obj_set_clickable(obj, false);
    lv_obj_add_style(obj, ui_style_accent_text(), 0);   /* redraw on mode change */

    tilt_bar_cfg_t * cfg = lv_malloc(sizeof(*cfg));
    cfg->inset = inset;
    cfg->narrow_bottom = narrow_bottom;
    lv_obj_set_user_data(obj, cfg);

    lv_obj_add_event_cb(obj, tilt_bar_draw_cb, LV_EVENT_DRAW_MAIN, NULL);
    lv_obj_add_event_cb(obj, tilt_bar_delete_cb, LV_EVENT_DELETE, NULL);
    return obj;
}
