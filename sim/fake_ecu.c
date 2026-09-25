/**
 * fake_ecu — simple boat physics driven by keyboard.
 *
 * Throttle -> RPM -> speed (with lag), fuel burn, engine warm-up,
 * iBR, modes, DESS key and manual warning toggles.
 * All numbers are made-up but plausible; nothing here is real BRP data.
 */
#include "fake_ecu.h"

#include <math.h>
#include <time.h>

#include "lvgl.h"
#include LV_SDL_INCLUDE_PATH
#include "../src/dash_data/dash_data.h"

/* ---------- Tuning ---------- */

#define TICK_MS          20
#define RPM_IDLE         1700.0f
#define RPM_MAX          8000.0f
#define RPM_ECO_MAX      6000.0f
#define SPEED_MAX_KMH    80.0f
#define SPEED_SLOW_KMH   8.0f
#define SPEED_REV_KMH    8.0f
#define TEMP_RUN_C       82.0f
#define TEMP_OVERHEAT_C  100.0f
#define FUEL_LOW_PCT     15.0f
#define BATT_LOW_V       11.8f

/* ---------- State ---------- */

static dash_data_t s_d;
static float s_throttle;        /* 0..1 */
static uint32_t s_manual_warn;  /* warnings forced by keys */
static bool s_force_overheat;
static bool s_force_low_batt;
static bool s_demo;
static float s_demo_t;

/* ---------- Helpers ---------- */

static float approach(float v, float target, float rate, float dt)
{
    float k = 1.0f - expf(-rate * dt);
    return v + (target - v) * k;
}

static float clampf(float v, float lo, float hi)
{
    return v < lo ? lo : v > hi ? hi : v;
}

/* ---------- Simulation step ---------- */

static void step(float dt)
{
    if(s_demo) {
        s_demo_t += dt;
        s_throttle = 0.5f + 0.5f * sinf(s_demo_t * 0.35f);
        if(s_d.ibr != DASH_IBR_FORWARD) s_d.ibr = DASH_IBR_FORWARD;
    }

    bool running = s_d.dess_ok;
    float rpm_cap = s_d.mode == DASH_MODE_ECO ? RPM_ECO_MAX : RPM_MAX;
    float rpm_target = running ? RPM_IDLE + s_throttle * (rpm_cap - RPM_IDLE) : 0.0f;
    float rpm_rate = s_d.mode == DASH_MODE_SPORT ? 6.0f : 3.5f;
    s_d.rpm = (uint16_t)approach(s_d.rpm, rpm_target, rpm_rate, dt);

    /* Speed follows thrust with boat-like lag */
    float thrust = clampf((s_d.rpm - 2200.0f) / (RPM_MAX - 2200.0f), 0.0f, 1.0f);
    float speed_target = 0.0f;
    float speed_rate = 0.6f;
    switch(s_d.ibr) {
        case DASH_IBR_FORWARD: speed_target = thrust * SPEED_MAX_KMH; break;
        case DASH_IBR_REVERSE: speed_target = thrust * SPEED_REV_KMH; break;
        case DASH_IBR_BRAKE:   speed_target = 0.0f; speed_rate = 2.0f; break;
        default:               speed_target = 0.0f; speed_rate = 0.3f; break;
    }
    if(s_d.mode == DASH_MODE_SLOW) speed_target = fminf(speed_target, SPEED_SLOW_KMH);
    s_d.speed_kmh = approach(s_d.speed_kmh, speed_target, speed_rate, dt);
    if(s_d.speed_kmh < 0.05f) s_d.speed_kmh = 0.0f;

    /* Fuel burn (sped up so you can see it move) */
    s_d.fuel_pct = clampf(s_d.fuel_pct - (s_d.rpm / RPM_MAX) * 0.02f * dt, 0.0f, 100.0f);

    /* Temperature, battery, hours, trip */
    float temp_target = s_force_overheat ? 110.0f : running ? TEMP_RUN_C : 20.0f;
    s_d.engine_temp_c = approach(s_d.engine_temp_c, temp_target, 0.15f, dt);
    s_d.battery_v = s_force_low_batt ? 11.4f : running ? 13.8f : 12.6f;
    if(s_d.rpm > 0) s_d.engine_hours += dt / 3600.0f;
    s_d.trip_km += s_d.speed_kmh * dt / 3600.0f;

    /* Warnings = manual toggles + automatic thresholds */
    uint32_t w = s_manual_warn;
    if(s_d.fuel_pct <= FUEL_LOW_PCT) w |= DASH_WARN_LOW_FUEL;
    if(s_d.engine_temp_c >= TEMP_OVERHEAT_C) w |= DASH_WARN_OVERHEAT;
    if(s_d.battery_v <= BATT_LOW_V) w |= DASH_WARN_LOW_BATTERY;
    s_d.warnings = w;
    s_d.error_code = (w & DASH_WARN_CHECK_ENGINE) ? 0x0122 : 0;

    /* Clock (on the boat this comes from GPS) */
    time_t now = time(NULL);
    struct tm * tm = localtime(&now);
    s_d.clock_h = (int8_t)tm->tm_hour;
    s_d.clock_m = (int8_t)tm->tm_min;
}

static void tick_cb(lv_timer_t * t)
{
    LV_UNUSED(t);
    step(TICK_MS / 1000.0f);
    dash_data_set(&s_d);
}

/* ---------- Keyboard ---------- */

bool fake_ecu_key(int key)
{
    switch(key) {
        case SDLK_UP:    s_throttle = clampf(s_throttle + 0.1f, 0, 1); s_demo = false; break;
        case SDLK_DOWN:  s_throttle = clampf(s_throttle - 0.1f, 0, 1); s_demo = false; break;
        case SDLK_SPACE: s_throttle = 0; s_demo = false; break;
        case SDLK_RIGHT: s_d.fuel_pct = clampf(s_d.fuel_pct + 5, 0, 100); break;
        case SDLK_LEFT:  s_d.fuel_pct = clampf(s_d.fuel_pct - 5, 0, 100); break;
        case SDLK_f:     s_d.ibr = DASH_IBR_FORWARD; break;
        case SDLK_n:     s_d.ibr = DASH_IBR_NEUTRAL; break;
        case SDLK_r:     s_d.ibr = DASH_IBR_REVERSE; break;
        case SDLK_b:     s_d.ibr = DASH_IBR_BRAKE; break;
        case SDLK_m:     s_d.mode = (s_d.mode + 1) % DASH_MODE_COUNT; break;
        case SDLK_k:     s_d.dess_ok = !s_d.dess_ok; break;
        case SDLK_1:     s_manual_warn ^= DASH_WARN_CHECK_ENGINE; break;
        case SDLK_2:     s_manual_warn ^= DASH_WARN_OIL_PRESSURE; break;
        case SDLK_3:     s_force_overheat = !s_force_overheat; break;
        case SDLK_4:     s_d.fuel_pct = s_d.fuel_pct > FUEL_LOW_PCT ? 10 : 80; break;
        case SDLK_5:     s_force_low_batt = !s_force_low_batt; break;
        case SDLK_0:     s_manual_warn = 0; s_force_overheat = false; s_force_low_batt = false; break;
        case SDLK_a:     s_demo = !s_demo; break;
        default: return false;
    }
    return true;
}

/* ---------- API ---------- */

void fake_ecu_init(void)
{
    s_d = *dash_data_get();
    s_d.source = DASH_SRC_SIM;
    s_d.dess_ok = true;
    s_d.fuel_pct = 75.0f;
    s_d.engine_hours = 42.0f;
    lv_timer_create(tick_cb, TICK_MS, NULL);
}
