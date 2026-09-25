/**
 * tilt_bar — centre rectangle + two slanted triangles, drawn once;
 * children (labels) redraw only their own area.
 */
#include "tilt_bar.h"

#include "ui_theme.h"

/* ---------- Style ---------- */

#define TILT_COLOR_A lv_color_hex(0x111820)   /* wide edge */
#define TILT_COLOR_B lv_color_hex(0x0A0F14)   /* narrow edge */

typedef struct {
    int32_t in_l, in_r;
    bool wide_top;
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

    lv_area_t a;
    lv_obj_get_coords(obj, &a);
    int32_t y_wide = cfg->wide_top ? a.y1 : a.y2;
    int32_t y_narrow = cfg->wide_top ? a.y2 : a.y1;

    /* Centre rectangle, gradient from wide edge to narrow edge */
    lv_draw_rect_dsc_t rd;
    lv_draw_rect_dsc_init(&rd);
    rd.bg_grad.dir = LV_GRAD_DIR_VER;
    rd.bg_grad.stops_count = 2;
    rd.bg_grad.stops[0].color = cfg->wide_top ? TILT_COLOR_A : TILT_COLOR_B;
    rd.bg_grad.stops[0].opa = LV_OPA_COVER;
    rd.bg_grad.stops[0].frac = 0;
    rd.bg_grad.stops[1].color = cfg->wide_top ? TILT_COLOR_B : TILT_COLOR_A;
    rd.bg_grad.stops[1].opa = LV_OPA_COVER;
    rd.bg_grad.stops[1].frac = 255;
    lv_area_t mid = { a.x1 + cfg->in_l, a.y1, a.x2 - cfg->in_r, a.y2 };
    lv_draw_rect(layer, &rd, &mid);

    /* Slanted ends (1 px overlap hides the seam) */
    lv_draw_triangle_dsc_t td;
    lv_draw_triangle_dsc_init(&td);
    td.color = lv_color_mix(TILT_COLOR_A, TILT_COLOR_B, LV_OPA_50);
    td.opa = LV_OPA_COVER;
    draw_tri(layer, &td, a.x1, y_wide, a.x1 + cfg->in_l + 1, y_wide, a.x1 + cfg->in_l + 1, y_narrow);
    draw_tri(layer, &td, a.x2, y_wide, a.x2 - cfg->in_r - 1, y_wide, a.x2 - cfg->in_r - 1, y_narrow);

    /* Accent hairline on the inner (screen-facing) edge */
    lv_draw_line_dsc_t ld;
    lv_draw_line_dsc_init(&ld);
    ld.color = ui_theme_accent();
    ld.width = 2;
    ld.opa = LV_OPA_60;
    int32_t y_inner = cfg->wide_top ? a.y1 : a.y2;
    ld.p1.x = a.x1 + 40; ld.p1.y = y_inner;
    ld.p2.x = a.x2 - 40; ld.p2.y = y_inner;
    lv_draw_line(layer, &ld);
}

static void tilt_bar_delete_cb(lv_event_t * e)
{
    lv_free(lv_obj_get_user_data(lv_event_get_target_obj(e)));
}

/* ---------- API ---------- */

lv_obj_t * tilt_bar_create(lv_obj_t * parent, int32_t x, int32_t y, int32_t w, int32_t h,
                           int32_t inset_l, int32_t inset_r, bool wide_top)
{
    lv_obj_t * obj = lv_obj_create(parent);
    lv_obj_remove_style_all(obj);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, w, h);
    lv_obj_set_scrollable(obj, false);
    lv_obj_set_clickable(obj, false);
    /* Redraw on mode change (accent line) */
    lv_obj_add_style(obj, ui_style_accent_text(), 0);

    tilt_bar_cfg_t * cfg = lv_malloc(sizeof(*cfg));
    cfg->in_l = inset_l;
    cfg->in_r = inset_r;
    cfg->wide_top = wide_top;
    lv_obj_set_user_data(obj, cfg);

    lv_obj_add_event_cb(obj, tilt_bar_draw_cb, LV_EVENT_DRAW_MAIN, NULL);
    lv_obj_add_event_cb(obj, tilt_bar_delete_cb, LV_EVENT_DELETE, NULL);
    return obj;
}
