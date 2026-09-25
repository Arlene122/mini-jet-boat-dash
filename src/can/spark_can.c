/**
 * spark_can — placeholder encode / decode (see header: UNVERIFIED).
 */
#include "spark_can.h"

#include <string.h>

/* ---------- Helpers ---------- */

static void put_u16(uint8_t * p, uint32_t v) { p[0] = v & 0xFF; p[1] = (v >> 8) & 0xFF; }
static uint16_t get_u16(const uint8_t * p) { return (uint16_t)(p[0] | (p[1] << 8)); }
static uint8_t clamp_u8(float v) { return v < 0 ? 0 : v > 255 ? 255 : (uint8_t)(v + 0.5f); }

static void frame(can_frame_t * f, uint32_t id, uint8_t dlc)
{
    memset(f, 0, sizeof(*f));
    f->id = id;
    f->dlc = dlc;
}

/* ---------- Decode ---------- */

bool spark_can_decode(const can_frame_t * f, dash_data_t * d)
{
    const uint8_t * b = f->data;
    switch(f->id) {
        case SPARK_ID_ENGINE:
            if(f->dlc < 4) return false;
            d->rpm = get_u16(b);
            d->engine_temp_c = (float)b[2] - 40.0f;
            d->battery_v = b[3] / 10.0f;
            return true;
        case SPARK_ID_STATUS:
            if(f->dlc < 6) return false;
            d->ibr = b[0] <= DASH_IBR_BRAKE ? (dash_ibr_t)b[0] : DASH_IBR_NEUTRAL;
            d->mode = b[1] < DASH_MODE_COUNT ? (dash_mode_t)b[1] : DASH_MODE_TOURING;
            d->warnings = (d->warnings & ~SPARK_FAULT_MASK) | (b[2] & SPARK_FAULT_MASK);
            d->dess_ok = b[3] != 0;
            d->error_code = get_u16(&b[4]);
            return true;
        case SPARK_ID_FUEL:
            if(f->dlc < 3) return false;
            d->fuel_pct = b[0] / 2.0f;
            d->fuel_rate_lph = get_u16(&b[1]) / 10.0f;
            return true;
        case SPARK_ID_HOURS:
            if(f->dlc < 4) return false;
            d->engine_hours = (float)((uint32_t)b[0] | ((uint32_t)b[1] << 8) |
                                      ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24)) / 100.0f;
            return true;
        default:
            return false;
    }
}

/* ---------- Encode (simulator) ---------- */

void spark_can_encode_engine(can_frame_t * f, uint16_t rpm, float temp_c, float batt_v)
{
    frame(f, SPARK_ID_ENGINE, 4);
    put_u16(f->data, rpm);
    f->data[2] = clamp_u8(temp_c + 40.0f);
    f->data[3] = clamp_u8(batt_v * 10.0f);
}

void spark_can_encode_status(can_frame_t * f, dash_ibr_t ibr, dash_mode_t mode, uint32_t faults,
                             bool dess, uint16_t code)
{
    frame(f, SPARK_ID_STATUS, 6);
    f->data[0] = (uint8_t)ibr;
    f->data[1] = (uint8_t)mode;
    f->data[2] = (uint8_t)(faults & SPARK_FAULT_MASK);
    f->data[3] = dess ? 1 : 0;
    put_u16(&f->data[4], code);
}

void spark_can_encode_fuel(can_frame_t * f, float level_pct, float rate_lph)
{
    frame(f, SPARK_ID_FUEL, 3);
    f->data[0] = clamp_u8(level_pct * 2.0f);
    float r = rate_lph * 10.0f;
    put_u16(&f->data[1], r < 0 ? 0 : r > 65535 ? 65535 : (uint32_t)(r + 0.5f));
}

void spark_can_encode_hours(can_frame_t * f, float hours)
{
    frame(f, SPARK_ID_HOURS, 4);
    uint32_t v = (uint32_t)(hours * 100.0f);
    for(int i = 0; i < 4; i++) f->data[i] = (v >> (8 * i)) & 0xFF;
}
