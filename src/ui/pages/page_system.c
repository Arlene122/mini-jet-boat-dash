/**
 * page_system — engine / electrical details as a clean list.
 */
#include "pages.h"

#include "../ui_theme.h"
#include "../ui_util.h"
#include "../../settings/settings.h"

static lv_obj_t * s_rpm, * s_hours, * s_temp, * s_batt, * s_rate, * s_code, * s_link;

static void create(lv_obj_t * p)
{
    int y = 0;
    s_rpm = page_list_row(p, y, "RPM", NULL);            y += LIST_ROW_H + 6;
    s_hours = page_list_row(p, y, "ENGINE HOURS", NULL); y += LIST_ROW_H + 6;
    s_temp = page_list_row(p, y, "ENGINE TEMP", NULL);   y += LIST_ROW_H + 6;
    s_batt = page_list_row(p, y, "BATTERY", NULL);       y += LIST_ROW_H + 6;
    s_rate = page_list_row(p, y, "FUEL USE", NULL);      y += LIST_ROW_H + 6;
    s_code = page_list_row(p, y, "ERROR CODE", NULL);    y += LIST_ROW_H + 6;
    s_link = page_list_row(p, y, "ECU LINK", NULL);
}

static void update(const dash_data_t * d)
{
    static const char * src[] = { "Sim", "Replay", "CAN" };
    if(d->ecu_ok) {
        ui_label_printf(s_rpm, "%d", d->rpm);
        ui_label_printf(s_temp, "%d%s", settings_temp(d->engine_temp_c), settings_temp_unit());
        ui_label_printf(s_batt, "%.1f V", (double)d->battery_v);
        ui_label_printf(s_rate, "%.1f L/h", (double)d->fuel_rate_lph);
        ui_label_printf(s_link, "%s  OK", src[d->source]);
    }
    else {
        ui_label_printf(s_rpm, "--");
        ui_label_printf(s_temp, "--");
        ui_label_printf(s_batt, "--");
        ui_label_printf(s_rate, "--");
        ui_label_printf(s_link, "NO DATA");
    }
    ui_set_text_color(s_link, d->ecu_ok ? C_TEXT : C_AMBER);
    ui_label_printf(s_hours, "%.1f h", (double)d->engine_hours);
    if(d->error_code) ui_label_printf(s_code, "P%04X", d->error_code);
    else ui_label_printf(s_code, "None");
    ui_set_text_color(s_code, d->error_code ? C_RED : C_TEXT);
}

const ui_page_t page_system = { "SYSTEM", create, update, NULL };
