/**
 * tide_chart — window = 12 h centred on now; curve h(t) = -cos(2π·phase).
 * Labels (LOW hh:mm / HIGH hh:mm) are lv_labels placed at the next
 * extremes inside the window.
 */
#include "tide_chart.h"

#include <math.h>
#include <stdio.h>

#include "ui_theme.h"
#include "ui_util.h"

/* ---------- Config ---------- */

#define STEPS        48
#define WINDOW_MIN   720      /* 12 h shown */
#define PAD_Y        22       /* room for labels above/below the curve */
#define TWO_PI       6.2831853f

typedef struct {
    float phase;
    int32_t now_min, period, w;
    lv_obj_t * lbl_hi, * lbl_lo;
} tide_t;

/* ---------- Helpers ---------- */

static float level(float phase)       /* 0 (low) .. 1 (high) */
{
    return 0.5f - 0.5f * cosf(TWO_PI * phase);
}

static float phase_at(const tide_t * t, float dmin)
{
    return t->phase + dmin / (float)t->period;
}

static void fmt_time(char * buf, size_t n, int32_t min, bool h12)
{
    min = ((min % 1440) + 1440) % 1440;
    int h = min / 60, m = min % 60;
    if(h12) snprintf(buf, n, "%d:%02d%s", (h + 11) % 12 + 1, m, h < 12 ? "a" : "p");
    else snprintf(buf, n, "%02d:%02d", h, m);
}

/* ---------- Drawing ---------- */

static void tide_draw_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target_obj(e);
    lv_layer_t * layer = lv_event_get_layer(e);
    const tide_t * t = lv_obj_get_user_data(obj);
    if(t->period <= 0) return;

    lv_area_t a;
    lv_obj_get_coords(obj, &a);
    float w = lv_area_get_width(&a), h = lv_area_get_height(&a) - 2 * PAD_Y;
    float y0 = a.y1 + PAD_Y;

    /* Mid line */
    lv_draw_line_dsc_t ld;
    lv_draw_line_dsc_init(&ld);
    ld.color = C_LINE;
    ld.width = 1;
    ld.p1.x = a.x1; ld.p1.y = y0 + h / 2;
    ld.p2.x = a.x2; ld.p2.y = y0 + h / 2;
    lv_draw_line(layer, &ld);

    /* Curve: past dim, future accent */
    ld.width = 3;
    ld.round_start = ld.round_end = 1;
    lv_point_precise_t prev = { 0, 0 };
    for(int i = 0; i <= STEPS; i++) {
        float dmin = -WINDOW_MIN / 2.0f + WINDOW_MIN * i / (float)STEPS;
        lv_point_precise_t p = { a.x1 + w * i / STEPS, y0 + h * (1.0f - level(phase_at(t, dmin))) };
        if(i > 0) {
            ld.color = dmin <= 0 ? C_OFF : ui_theme_accent();
            ld.p1 = prev;
            ld.p2 = p;
            lv_draw_line(layer, &ld);
        }
        prev = p;
    }

    /* Now marker: vertical hairline + dot */
    float nx = a.x1 + w / 2, ny = y0 + h * (1.0f - level(t->phase));
    ld.color = C_DIM;
    ld.width = 1;
    ld.round_start = ld.round_end = 0;
    ld.p1.x = nx; ld.p1.y = a.y1 + 4;
    ld.p2.x = nx; ld.p2.y = a.y2 - 4;
    lv_draw_line(layer, &ld);
    lv_draw_rect_dsc_t rd;
    lv_draw_rect_dsc_init(&rd);
    rd.radius = LV_RADIUS_CIRCLE;
    rd.bg_color = lv_color_white();
    rd.border_color = ui_theme_accent();
    rd.border_width = 3;
    lv_area_t d = { (int32_t)nx - 7, (int32_t)ny - 7, (int32_t)nx + 7, (int32_t)ny + 7 };
    lv_draw_rect(layer, &rd, &d);
}

static void tide_delete_cb(lv_event_t * e)
{
    lv_free(lv_obj_get_user_data(lv_event_get_target_obj(e)));
}

/* ---------- API ---------- */

lv_obj_t * tide_chart_create(lv_obj_t * parent, int32_t x, int32_t y, int32_t w, int32_t h)
{
    lv_obj_t * obj = ui_box(parent, x, y, w, h);
    lv_obj_add_style(obj, ui_style_accent_text(), 0);   /* redraw on mode change */
    tide_t * t = lv_malloc_zeroed(sizeof(*t));
    t->now_min = -2;
    t->w = w;
    t->lbl_hi = ui_label(obj, &lv_font_montserrat_16, C_TEXT, "");
    t->lbl_lo = ui_label(obj, &lv_font_montserrat_16, C_DIM, "");
    lv_obj_set_user_data(obj, t);
    lv_obj_add_event_cb(obj, tide_draw_cb, LV_EVENT_DRAW_MAIN, NULL);
    lv_obj_add_event_cb(obj, tide_delete_cb, LV_EVENT_DELETE, NULL);
    return obj;
}

void tide_chart_set(lv_obj_t * obj, float phase, int32_t now_min, int32_t period_min, bool clock_12h)
{
    tide_t * t = lv_obj_get_user_data(obj);
    if(t->now_min == now_min && t->period == period_min && fabsf(t->phase - phase) < 0.002f) return;
    t->phase = phase;
    t->now_min = now_min;
    t->period = period_min;

    /* Next high (phase .5) and low (phase 0/1) from now */
    float to_hi = fmodf(0.5f - phase + 1.0f, 1.0f) * period_min;
    float to_lo = fmodf(1.0f - phase, 1.0f) * period_min;
    int32_t w = t->w;
    char buf[24], tm[12];

    struct { lv_obj_t * l; float dm; const char * name; bool top; } ex[2] = {
        { t->lbl_hi, to_hi, "HIGH", true }, { t->lbl_lo, to_lo, "LOW", false },
    };
    for(int i = 0; i < 2; i++) {
        /* show the extreme nearest to now inside the window (future first) */
        float dm = ex[i].dm;
        if(dm > WINDOW_MIN / 2) dm -= period_min;
        if(now_min < 0) snprintf(buf, sizeof(buf), "%s", ex[i].name);
        else {
            fmt_time(tm, sizeof(tm), now_min + (int32_t)dm, clock_12h);
            snprintf(buf, sizeof(buf), "%s %s", ex[i].name, tm);
        }
        lv_label_set_text(ex[i].l, buf);
        lv_obj_update_layout(ex[i].l);
        int32_t x = (int32_t)(w / 2 + w * dm / WINDOW_MIN) - lv_obj_get_width(ex[i].l) / 2;
        x = LV_CLAMP(0, x, w - lv_obj_get_width(ex[i].l));
        lv_obj_align(ex[i].l, ex[i].top ? LV_ALIGN_TOP_LEFT : LV_ALIGN_BOTTOM_LEFT, x, 0);
    }
    lv_obj_invalidate(obj);
}
