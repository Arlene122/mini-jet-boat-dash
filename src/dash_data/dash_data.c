/**
 * dash_data — single snapshot shared by data sources and the UI.
 * On the P4 this will get a mutex (CAN task vs LVGL task); on PC it is
 * single-threaded.
 */
#include "dash_data.h"

#include <string.h>

/* ---------- State ---------- */

static dash_data_t s_data;
static uint32_t s_seq;

/* ---------- API ---------- */

void dash_data_init(void)
{
    memset(&s_data, 0, sizeof(s_data));
    s_data.fuel_pct = 100.0f;
    s_data.battery_v = 12.6f;
    s_data.engine_temp_c = 20.0f;
    s_data.ibr = DASH_IBR_NEUTRAL;
    s_data.mode = DASH_MODE_TOURING;
    s_data.clock_h = -1;
    s_seq = 0;
}

const dash_data_t * dash_data_get(void)
{
    return &s_data;
}

void dash_data_set(const dash_data_t * d)
{
    s_data = *d;
    s_seq++;
}

uint32_t dash_data_seq(void)
{
    return s_seq;
}
