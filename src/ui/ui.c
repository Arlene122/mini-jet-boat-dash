/**
 * ui — owns the screens and pushes dash_data changes into them.
 */
#include "ui.h"

#include "lvgl.h"
#include "screen_main.h"
#include "../dash_data/dash_data.h"

/* ---------- Config ---------- */

#define UI_POLL_MS 20   /* how often we look for new data */

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

/* ---------- API ---------- */

void ui_init(void)
{
    lv_obj_t * scr = screen_main_create();
    lv_screen_load(scr);
    screen_main_update(dash_data_get());
    lv_timer_create(ui_poll_cb, UI_POLL_MS, NULL);
}
