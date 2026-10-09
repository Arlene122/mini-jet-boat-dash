/**
 * key_on — every step is an lv_anim on a widget's opacity / translate /
 * scale, ending exactly on the normal values so nothing costs extra once
 * the animation is over. Timings are relative to key_on_play().
 */
#include "key_on.h"

#include "frame_line.h"
#include "ui.h"

/* ---------- Config ---------- */

#define GROUP_MAX   12

typedef struct {
    uint32_t delay, dur;
    int32_t slide;        /* px; >0 from the outside edge, <0 from the centre */
    int32_t scale0;       /* start scale (256 = 1.0), 0 = no scale */
    bool scale_x_only;    /* unfold sideways */
    lv_anim_path_cb_t path;
} step_t;

static const step_t STEP[KO_COUNT] = {
    [KO_BG]       = {   0, 600,   0,   0, false, NULL },
    [KO_FRAME]    = { 100, 560,   0,   0, false, NULL },   /* trace, see frame_line */
    [KO_BAR]      = { 380, 420,   0,  40, true,  NULL },
    [KO_GAUGE]    = { 180, 560,   0, 205, false, NULL },
    [KO_BRACKETS] = { 520, 460, -60,   0, false, NULL },
    [KO_SIDE]     = { 640, 520,  48,   0, false, NULL },
};

/* ---------- State ---------- */

static lv_obj_t * s_obj[KO_COUNT][GROUP_MAX];
static uint8_t s_n[KO_COUNT];

/* ---------- Anim callbacks ---------- */

/* Layered: also fades widgets that draw themselves (brackets, bars) */
static void opa_cb(void * o, int32_t v) { lv_obj_set_style_opa_layered(o, (lv_opa_t)v, 0); }
static void tx_cb(void * o, int32_t v) { lv_obj_set_style_translate_x(o, v, 0); }
static void scale_cb(void * o, int32_t v)
{
    lv_obj_set_style_transform_scale_x(o, v, 0);
    lv_obj_set_style_transform_scale_y(o, v, 0);
}
static void scale_x_cb(void * o, int32_t v) { lv_obj_set_style_transform_scale_x(o, v, 0); }
static void reveal_cb(void * o, int32_t v) { frame_line_set_reveal(o, v); }

static void run(lv_obj_t * o, lv_anim_exec_xcb_t cb, int32_t from, int32_t to,
                uint32_t delay, uint32_t dur, lv_anim_path_cb_t path)
{
    cb(o, from);                         /* hold the start value during the delay */
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, o);
    lv_anim_set_exec_cb(&a, cb);
    lv_anim_set_values(&a, from, to);
    lv_anim_set_delay(&a, delay);
    lv_anim_set_duration(&a, dur);
    lv_anim_set_path_cb(&a, path ? path : lv_anim_path_ease_out);
    lv_anim_start(&a);
}

/* ---------- API ---------- */

void key_on_reset(void)
{
    for(int g = 0; g < KO_COUNT; g++) s_n[g] = 0;
}

void key_on_add(key_on_group_t g, lv_obj_t * obj)
{
    if(obj && s_n[g] < GROUP_MAX) s_obj[g][s_n[g]++] = obj;
}

void key_on_add_from(key_on_group_t g, lv_obj_t * scr, uint32_t from)
{
    uint32_t n = lv_obj_get_child_count(scr);
    for(uint32_t i = from; i < n; i++) key_on_add(g, lv_obj_get_child(scr, (int32_t)i));
}

void key_on_play(void)
{
    lv_obj_update_layout(lv_screen_active());   /* real coords for slide directions */
    for(int g = 0; g < KO_COUNT; g++) {
        const step_t * s = &STEP[g];
        for(int i = 0; i < s_n[g]; i++) {
            lv_obj_t * o = s_obj[g][i];
            if(g == KO_FRAME) {
                run(o, reveal_cb, 0, 1000, s->delay, s->dur, lv_anim_path_ease_in_out);
                continue;
            }
            run(o, opa_cb, LV_OPA_TRANSP, LV_OPA_COVER, s->delay, s->dur, NULL);
            if(s->slide) {
                lv_area_t a;
                lv_obj_get_coords(o, &a);
                int32_t side = (a.x1 + a.x2) / 2 < UI_HOR_RES / 2 ? -1 : 1;
                run(o, tx_cb, side * s->slide, 0, s->delay, s->dur, NULL);
            }
            if(s->scale0) {
                lv_obj_set_style_transform_pivot_x(o, LV_PCT(50), 0);
                lv_obj_set_style_transform_pivot_y(o, LV_PCT(50), 0);
                run(o, s->scale_x_only ? scale_x_cb : scale_cb, s->scale0, LV_SCALE_NONE,
                    s->delay, s->dur, s->scale_x_only ? NULL : lv_anim_path_overshoot);
            }
        }
    }
}
