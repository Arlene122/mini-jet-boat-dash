/**
 * page_system — engine / electrical details.
 */
#include "pages.h"

#include "../ui_theme.h"
#include "../ui_util.h"

static lv_obj_t * s_hours, * s_batt, * s_temp, * s_rate, * s_code, * s_src;

static void create(lv_obj_t * p)
{
    s_hours = page_row(p, 0, 0, "ENGINE HOURS", &lv_font_montserrat_32);
    s_batt = page_row(p, 240, 0, "BATTERY", &lv_font_montserrat_32);
    s_temp = page_row(p, 0, 95, "ENGINE TEMP", &lv_font_montserrat_32);
    s_rate = page_row(p, 240, 95, "FUEL USE", &lv_font_montserrat_32);
    s_code = page_row(p, 0, 190, "ERROR CODE", &lv_font_montserrat_32);
    s_src = page_row(p, 240, 190, "DATA SOURCE", &lv_font_montserrat_32);
}

static void update(const dash_data_t * d)
{
    ui_label_printf(s_hours, "%.1f h", (double)d->engine_hours);
    ui_label_printf(s_batt, "%.1f V", (double)d->battery_v);
    ui_label_printf(s_temp, "%d°C", (int)(d->engine_temp_c + 0.5f));
    ui_label_printf(s_rate, "%.1f L/h", (double)d->fuel_rate_lph);
    if(d->error_code) ui_label_printf(s_code, "P%04X", d->error_code);
    else ui_label_printf(s_code, "None");
    ui_set_text_color(s_code, d->error_code ? C_RED : C_TEXT);
    static const char * src[] = { "Simulator", "Replay", "CAN" };
    ui_label_printf(s_src, "%s", src[d->source]);
}

const ui_page_t page_system = { "SYSTEM", create, update, NULL };
