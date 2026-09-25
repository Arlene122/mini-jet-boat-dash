/**
 * page_settings — UP/DOWN picks a row, knob push changes its value.
 * Changes apply immediately and are saved (settings.c).
 */
#include "pages.h"

#include "../ui_theme.h"
#include "../ui_util.h"
#include "../../settings/settings.h"

/* ---------- Rows ---------- */

typedef enum { ROW_SPEED = 0, ROW_TEMP, ROW_CLOCK, ROW_BRIGHT, ROW_LOG, ROW_COUNT } row_t;
static const char * ROW_NAME[ROW_COUNT] = { "SPEED UNIT", "TEMPERATURE", "CLOCK", "BRIGHTNESS", "RIDE LOGGING" };

#define ROW_STEP (LIST_ROW_H + 8)

static lv_obj_t * s_row[ROW_COUNT], * s_val[ROW_COUNT], * s_mark;
static int s_focus;
static uint32_t s_seq = UINT32_MAX;

/* ---------- Helpers ---------- */

static void show(void)
{
    const settings_t * s = settings_get();
    static const char * speed[] = { "km/h", "knots", "mph" };
    ui_label_printf(s_val[ROW_SPEED], "%s", speed[s->speed_unit]);
    ui_label_printf(s_val[ROW_TEMP], "%s", s->temp_f ? "°F" : "°C");
    ui_label_printf(s_val[ROW_CLOCK], "%s", s->clock_12h ? "12 h" : "24 h");
    ui_label_printf(s_val[ROW_BRIGHT], "%d %%", s->brightness);
    ui_label_printf(s_val[ROW_LOG], "%s", s->logging ? "On" : "Off");
    for(int i = 0; i < ROW_COUNT; i++) {
        if(i == s_focus) lv_obj_add_style(s_val[i], ui_style_accent_text(), 0);
        else lv_obj_remove_style(s_val[i], ui_style_accent_text(), 0);
    }
    lv_obj_set_y(s_mark, s_focus * ROW_STEP + 12);
}

static void change(void)
{
    settings_t s = *settings_get();
    switch(s_focus) {
        case ROW_SPEED:  s.speed_unit = (s.speed_unit + 1) % UNIT_SPEED_COUNT; break;
        case ROW_TEMP:   s.temp_f ^= 1; break;
        case ROW_CLOCK:  s.clock_12h ^= 1; break;
        case ROW_BRIGHT: s.brightness = s.brightness >= 100 ? 20 : s.brightness + 10; break;
        case ROW_LOG:    s.logging ^= 1; break;
    }
    settings_update(&s);
}

/* ---------- Page ---------- */

static void create(lv_obj_t * p)
{
    for(int i = 0; i < ROW_COUNT; i++) {
        s_val[i] = page_list_row(p, i * ROW_STEP, ROW_NAME[i], &s_row[i]);
    }
    /* Focus marker: short accent bar at the left of the chosen row */
    s_mark = ui_box(p, 0, 0, 3, LIST_ROW_H - 24);
    lv_obj_set_style_radius(s_mark, 2, 0);
    lv_obj_add_style(s_mark, ui_style_accent_bg(), 0);
    lv_obj_t * hint = ui_caption(p, "UP/DOWN: CHOOSE     PUSH: CHANGE");
    lv_obj_set_style_text_font(hint, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_letter_space(hint, 1, 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    show();
}

static void update(const dash_data_t * d)
{
    LV_UNUSED(d);
    if(settings_seq() == s_seq) return;
    s_seq = settings_seq();
    show();
}

static bool input(ui_input_t in)
{
    switch(in) {
        case UI_IN_UP:     s_focus = (s_focus + ROW_COUNT - 1) % ROW_COUNT; break;
        case UI_IN_DOWN:   s_focus = (s_focus + 1) % ROW_COUNT; break;
        case UI_IN_SELECT: change(); break;
        default: return false;
    }
    show();
    return true;
}

const ui_page_t page_settings = { "SETTINGS", create, update, input };
