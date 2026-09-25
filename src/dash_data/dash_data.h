/**
 * dash_data — the ONLY place the UI gets values from.
 *
 * Data sources write their own fields via dash_data_edit():
 *   engine  <- CAN frames decoded by dash_task (fake ECU, replay or boat)
 *   GPS     <- GPS module (simulator: fake GPS)
 *   music   <- audio board   · lights <- relay board
 * dash_task commits once per step; the UI reads dash_data_get() and
 * redraws only what changed. Same code on PC and P4.
 * UI -> boat actions go the other way through dash_cmd.h.
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

/* Riding modes. 2015 Spark 900 HO ACE: Touring (default) + Sport per
 * operator's guide. ECO: UNVERIFIED for the 2015 Spark — confirm on the
 * stock gauge / CAN logs. How the ECU reports the mode: UNVERIFIED. */
typedef enum {
    DASH_MODE_TOURING = 0,
    DASH_MODE_SPORT,
    DASH_MODE_ECO,
    DASH_MODE_COUNT
} dash_mode_t;

/* Warning bit flags (order = priority, highest first) */
enum {
    DASH_WARN_OIL_PRESSURE = 1u << 0,
    DASH_WARN_OVERHEAT     = 1u << 1,
    DASH_WARN_CHECK_ENGINE = 1u << 2,
    DASH_WARN_LOW_FUEL     = 1u << 3,
    DASH_WARN_LOW_BATTERY  = 1u << 4,
    DASH_WARN_COUNT        = 5
};

#define DASH_LIGHT_COUNT 5
#define DASH_TEXT_LEN    48

/* ---------- Grouped data ---------- */

/* Phone link (audio board). Auto-reconnects to the last phone on power-up. */
typedef enum {
    DASH_BT_OFF = 0,        /* no phone, not looking */
    DASH_BT_SEARCHING,      /* trying the last paired phone */
    DASH_BT_PAIRING,        /* visible, waiting for a new phone */
    DASH_BT_CONNECTED,
} dash_bt_t;

typedef struct {
    dash_bt_t bt;
    bool     connected;     /* == (bt == DASH_BT_CONNECTED) */
    bool     playing;
    char     phone[DASH_TEXT_LEN];
    char     title[DASH_TEXT_LEN];
    char     artist[DASH_TEXT_LEN];
    uint16_t pos_s;          /* playback position */
    uint16_t len_s;          /* track length */
} dash_music_t;

typedef struct {
    bool  gps_fix;
    float lat, lon;
    float heading_deg;
    bool  water_temp_ok;
    float water_temp_c;
    bool  depth_ok;         /* depth sounder fitted + reading (future sensor) */
    float depth_m;
    bool  tide_ok;          /* tide data from phone hotspot */
    float tide_m, tide_min_m, tide_max_m;
    float tide_phase;       /* 0 = low water, 0.5 = high water, wraps at 1 */
    uint16_t tide_period_min;   /* low-to-low, ~745 min */
} dash_marine_t;

typedef struct {
    float    top_speed_kmh;
    uint16_t max_rpm;
    uint32_t ride_time_s;
    float    fuel_used_l;
} dash_trip_t;

/* ---------- Data snapshot ---------- */

typedef struct {
    float    speed_kmh;      /* GPS speed */
    uint16_t rpm;
    float    fuel_pct;       /* 0..100 */
    float    fuel_rate_lph;
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
    bool     ecu_ok;         /* engine CAN data is fresh */
    bool     wifi_ok;        /* phone hotspot joined (weather, tide, OTA) */
    uint8_t  lights;         /* bit n = light n+1 on (relay state) */
    dash_music_t  music;
    dash_marine_t marine;
    dash_trip_t   trip;
    dash_source_t source;
} dash_data_t;

/* ---------- API ---------- */

void dash_data_init(void);

/* Latest snapshot (read-only). */
const dash_data_t * dash_data_get(void);

/* Sources change their fields in place, then (dash_task) commits. */
dash_data_t * dash_data_edit(void);
void dash_data_commit(void);

/* Increments on every commit; lets the UI skip idle frames. */
uint32_t dash_data_seq(void);

#endif /* DASH_DATA_H */
