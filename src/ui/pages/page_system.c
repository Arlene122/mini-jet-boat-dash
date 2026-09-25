/**
 * page_system — engine / electrical details, ECU link state.
 */
#include "pages.h"

#include "../ui_theme.h"
#include "../ui_util.h"
#include "../../settings/settings.h"

static lv_obj_t * s_rpm, * s_hours, * s_temp, * s_batt, * s_rate, * s_fuel, * s_code, * s_link;

static void create(lv_obj_t * p)
{
    const lv_font_t * f = &lv_font_montserrat_24;
    s_rpm = page_row(p, 0, 0, "RPM", f);
    s_hours = page_row(p, PAGE_COL2, 0, "ENGINE HOURS", f);
    s_temp = page_row(p, 0, 76, "ENGINE TEMP", f);
    s_batt = page_row(p, PAGE_COL2, 76, "BATTERY", f);
    s_rate = page_row(p, 0, 152, "FUEL USE", f);
    s_fuel = page_row(p, PAGE_COL2, 152, "FUEL LEVEL", f);
    s_code = page_row(p, 0, 228, "ERROR CODE", f);
    s_link = page_row(p, PAGE_COL2, 228, "ECU LINK", f);
}

static void update(const dash_data_t * d)
{
    static const char * src[] = { "Sim", "Replay", "CAN" };
    if(d->ecu_ok) {
        ui_label_printf(s_rpm, "%d", d->rpm);
        ui_label_printf(s_temp, "%d%s", settings_temp(d->engine_temp_c), settings_temp_unit());
        ui_label_printf(s_batt, "%.1f V", (double)d->battery_v);
        ui_label_printf(s_rate, "%.1f L/h", (double)d->fuel_rate_lph);
        ui_label_printf(s_fuel, "%d%%", (int)(d->fuel_pct + 0.5f));
        ui_label_printf(s_link, "%s  OK", src[d->source]);
    }
    else {
        ui_label_printf(s_rpm, "--");
        ui_label_printf(s_temp, "--");
        ui_label_printf(s_batt, "--");
        ui_label_printf(s_rate, "--");
        ui_label_printf(s_fuel, "--");
        ui_label_printf(s_link, "NO DATA");
    }
    ui_set_text_color(s_link, d->ecu_ok ? C_TEXT : C_AMBER);
    ui_label_printf(s_hours, "%.1f h", (double)d->engine_hours);
    if(d->error_code) ui_label_printf(s_code, "P%04X", d->error_code);
    else ui_label_printf(s_code, "None");
    ui_set_text_color(s_code, d->error_code ? C_RED : C_TEXT);
}

const ui_page_t page_system = { "SYSTEM", create, update, NULL };
