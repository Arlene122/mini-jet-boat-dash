/**
 * ui — owns the screen, routes control inputs, runs the key-on sweep and
 * pushes dash_data changes into the widgets.
 */
#include "ui.h"

#include "gauge_speed.h"
#include "lvgl.h"
#include "page_host.h"
#include "screen_main.h"
#include "ui_input.h"
#include "ui_theme.h"
#include "warn_banner.h"
#include "../dash_data/dash_data.h"

/* ---------- Config ---------- */

#define UI_POLL_MS   20    /* how often we look for new data */
#define SWEEP_MS     900   /* each direction of the key-on sweep */

/* ---------- State ---------- */

static uint32_t s_last_seq = UINT32_MAX;

/* ---------- Refresh ---------- */

static void ui_poll_cb(lv_timer_t * t)
{
    LV_UNUSED(t);
    uint32_t seq = dash_data_seq();
    if(seq == s_last_seq) return;   /* nothing new: no redraw */
    s_last_seq = seq;
    screen_main_update(dash_data_get());
}

/* ---------- Key-on sweep ---------- */

static void sweep_exec(void * var, int32_t v)
{
    LV_UNUSED(var);
    gauge_speed_set_arcs(v * GAUGE_SPEED_MAX / 1000, v * GAUGE_RPM_MAX / 1000);
}

static void sweep_done(lv_anim_t * a)
{
    LV_UNUSED(a);
    screen_main_set_sweep(false);
    s_last_seq = UINT32_MAX;        /* force a live update */
}

static void start_sweep(void)
{
    screen_main_set_sweep(true);
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_exec_cb(&a, sweep_exec);
    lv_anim_set_values(&a, 0, 1000);
    lv_anim_set_duration(&a, SWEEP_MS);
    lv_anim_set_reverse_duration(&a, SWEEP_MS);
    lv_anim_set_delay(&a, 200);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_set_completed_cb(&a, sweep_done);
    lv_anim_start(&a);
}

/* ---------- Input ---------- */

void ui_input(ui_input_t in)
{
    if(warn_banner_visible() && in == UI_IN_SELECT) {
        warn_banner_ack();
        return;
    }
    switch(in) {
        case UI_IN_NEXT: page_host_step(1); break;
        case UI_IN_PREV: page_host_step(-1); break;
        case UI_IN_HOME: page_host_home(); break;
        default: page_host_input(in); break;
    }
}

/* ---------- API ---------- */

void ui_init(void)
{
    ui_theme_init();
    lv_obj_t * scr = screen_main_create();
    lv_screen_load(scr);
    start_sweep();
    lv_timer_create(ui_poll_cb, UI_POLL_MS, NULL);
}
