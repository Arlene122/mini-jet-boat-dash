/**
 * dash_data — the ONLY place the UI gets values from.
 *
 * A data source (fake ECU simulator, CAN replay, or real CAN + GPS) writes
 * values with dash_data_set(). The UI reads a snapshot with dash_data_get()
 * and redraws only what changed. Same code on PC and on the ESP32-P4.
 */
#ifndef DASH_DATA_H
#define DASH_DATA_H

#include <stdbool.h>
#include <stdint.h>

/* ---------- Enums ---------- */

typedef enum {
    DASH_SRC_SIM = 0,   /* fake Spark ECU (PC simulator) */
    DASH_SRC_REPLAY,    /* recorded CAN log */
    DASH_SRC_CAN,       /* real boat */
} dash_source_t;

/* iBR state (UNVERIFIED how the ECU reports it — naming only) */
typedef enum {
    DASH_IBR_NEUTRAL = 0,
    DASH_IBR_FORWARD,
    DASH_IBR_REVERSE,
    DASH_IBR_BRAKE,
} dash_ibr_t;

/* Riding modes (UNVERIFIED list — confirm from stock gauge / CAN logs) */
typedef enum {
    DASH_MODE_TOURING = 0,
    DASH_MODE_SPORT,
    DASH_MODE_ECO,
    DASH_MODE_SLOW,
    DASH_MODE_COUNT
} dash_mode_t;

/* Warning bit flags */
enum {
    DASH_WARN_CHECK_ENGINE = 1u << 0,
    DASH_WARN_OIL_PRESSURE = 1u << 1,
    DASH_WARN_OVERHEAT     = 1u << 2,
    DASH_WARN_LOW_FUEL     = 1u << 3,
    DASH_WARN_LOW_BATTERY  = 1u << 4,
};

/* ---------- Data snapshot ---------- */

typedef struct {
    float    speed_kmh;      /* GPS speed */
    uint16_t rpm;
    float    fuel_pct;       /* 0..100 */
    float    engine_temp_c;
    float    battery_v;
    float    engine_hours;
    float    trip_km;
    int8_t   clock_h;        /* 0..23, -1 = unknown (GPS/RTC) */
    int8_t   clock_m;        /* 0..59 */
    dash_ibr_t  ibr;
    dash_mode_t mode;
    uint32_t warnings;       /* DASH_WARN_* flags */
    uint16_t error_code;     /* 0 = none */
    bool     dess_ok;        /* DESS key recognised */
    dash_source_t source;
} dash_data_t;

/* ---------- API ---------- */

void dash_data_init(void);

/* Latest snapshot (read-only). */
const dash_data_t * dash_data_get(void);

/* Replace the snapshot (called by the active data source). */
void dash_data_set(const dash_data_t * d);

/* Increments on every dash_data_set(); lets the UI skip idle frames. */
uint32_t dash_data_seq(void);

#endif /* DASH_DATA_H */
