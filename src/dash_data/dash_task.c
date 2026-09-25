/**
 * dash_task — see header.
 */
#include "dash_task.h"

#include "dash_data.h"
#include "../can/spark_can.h"

/* ---------- Config ---------- */

#define ECU_TIMEOUT_MS   1000
#define FUEL_LOW_PCT     15.0f
#define BATT_LOW_V       11.8f
#define MOVING_KMH       1.0f

/* ---------- State ---------- */

static uint32_t s_last_rx_ms;
static uint32_t s_last_ms;
static bool s_started;
static float s_ride_s;

/* ---------- Steps ---------- */

static void drain_can(dash_data_t * d, uint32_t now)
{
    can_frame_t f;
    while(can_bus_receive(&f)) {
        if(spark_can_decode(&f, d)) s_last_rx_ms = now;
    }
    bool ok = s_started && (now - s_last_rx_ms) < ECU_TIMEOUT_MS;
    if(!ok && d->ecu_ok) {
        /* Stale: never show old engine numbers as if live */
        d->rpm = 0;
        d->fuel_rate_lph = 0;
        d->warnings &= ~SPARK_FAULT_MASK;
    }
    d->ecu_ok = ok;
}

static void derive_warnings(dash_data_t * d)
{
    uint32_t w = d->warnings & ~(DASH_WARN_LOW_FUEL | DASH_WARN_LOW_BATTERY);
    if(d->ecu_ok) {
        if(d->fuel_pct <= FUEL_LOW_PCT) w |= DASH_WARN_LOW_FUEL;
        if(d->battery_v <= BATT_LOW_V) w |= DASH_WARN_LOW_BATTERY;
    }
    d->warnings = w;
}

static void update_trip(dash_data_t * d, float dt)
{
    d->trip_km += d->speed_kmh * dt / 3600.0f;
    if(d->speed_kmh > MOVING_KMH) s_ride_s += dt;
    d->trip.ride_time_s = (uint32_t)s_ride_s;
    if(d->speed_kmh > d->trip.top_speed_kmh) d->trip.top_speed_kmh = d->speed_kmh;
    if(d->rpm > d->trip.max_rpm) d->trip.max_rpm = d->rpm;
    d->trip.fuel_used_l += d->fuel_rate_lph * dt / 3600.0f;
}

/* ---------- API ---------- */

void dash_task_init(void)
{
    s_started = false;
}

void dash_task_step(uint32_t now_ms)
{
    dash_data_t * d = dash_data_edit();
    if(!s_started) {
        s_started = true;
        s_last_ms = now_ms;
        s_last_rx_ms = now_ms;
    }
    float dt = (now_ms - s_last_ms) / 1000.0f;
    s_last_ms = now_ms;

    drain_can(d, now_ms);
    derive_warnings(d);
    update_trip(d, dt);
    dash_data_commit();
}
