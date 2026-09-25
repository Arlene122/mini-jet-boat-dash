/**
 * fake_ecu — simple boat physics driven by keyboard, plus fake GPS, audio
 * board and light relays so every page has data.
 * All numbers are made-up but plausible; nothing here is real BRP data.
 */
#include "fake_ecu.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "lvgl.h"
#include LV_SDL_INCLUDE_PATH
#include "../src/dash_data/dash_cmd.h"
#include "../src/dash_data/dash_data.h"

/* ---------- Tuning ---------- */

#define TICK_MS          20
#define RPM_IDLE         1700.0f
#define RPM_MAX          8000.0f
#define RPM_ECO_MAX      6000.0f
#define SPEED_MAX_KMH    80.0f
#define SPEED_REV_KMH    8.0f
#define TEMP_RUN_C       82.0f
#define TEMP_OVERHEAT_C  100.0f
#define FUEL_LOW_PCT     15.0f
#define BATT_LOW_V       11.8f
#define TANK_L           30.0f
#define FUEL_BURN_SPEEDUP 20.0f   /* so the gauge visibly moves */

/* ---------- State ---------- */

static dash_data_t s_d;
static float s_throttle;        /* 0..1 */
static uint32_t s_manual_warn;  /* warnings forced by keys */
static bool s_force_overheat;
static bool s_force_low_batt;
static bool s_demo;
static float s_demo_t;
static float s_ride_t;
static float s_music_t;
static int s_track;

typedef struct { const char * title; const char * artist; uint16_t len; } track_t;
static const track_t TRACKS[] = {
    { "Ocean Drive", "Duke Dumont", 206 },
    { "Midnight City", "M83", 244 },
    { "Sunset Lover", "Petit Biscuit", 238 },
};
#define TRACK_COUNT (int)(sizeof(TRACKS) / sizeof(TRACKS[0]))

/* ---------- Helpers ---------- */

static float approach(float v, float target, float rate, float dt)
{
    return v + (target - v) * (1.0f - expf(-rate * dt));
}

static float clampf(float v, float lo, float hi)
{
    return v < lo ? lo : v > hi ? hi : v;
}

static void load_track(int i)
{
    s_track = (i + TRACK_COUNT) % TRACK_COUNT;
    snprintf(s_d.music.title, DASH_TEXT_LEN, "%s", TRACKS[s_track].title);
    snprintf(s_d.music.artist, DASH_TEXT_LEN, "%s", TRACKS[s_track].artist);
    s_d.music.len_s = TRACKS[s_track].len;
    s_music_t = 0;
}

/* ---------- Simulation step ---------- */

static void step_engine(float dt)
{
    if(s_demo) {
        s_demo_t += dt;
        s_throttle = 0.5f + 0.5f * sinf(s_demo_t * 0.35f);
        s_d.ibr = DASH_IBR_FORWARD;
    }

    bool running = s_d.dess_ok;
    float rpm_cap = s_d.mode == DASH_MODE_ECO ? RPM_ECO_MAX : RPM_MAX;
    float rpm_target = running ? RPM_IDLE + s_throttle * (rpm_cap - RPM_IDLE) : 0.0f;
    float rpm_rate = s_d.mode == DASH_MODE_SPORT ? 6.0f : 3.5f;
    s_d.rpm = (uint16_t)approach(s_d.rpm, rpm_target, rpm_rate, dt);

    /* Speed follows thrust with boat-like lag */
    float thrust = clampf((s_d.rpm - 2200.0f) / (RPM_MAX - 2200.0f), 0.0f, 1.0f);
    float target = 0.0f, rate = 0.6f;
    switch(s_d.ibr) {
        case DASH_IBR_FORWARD: target = thrust * SPEED_MAX_KMH; break;
        case DASH_IBR_REVERSE: target = thrust * SPEED_REV_KMH; break;
        case DASH_IBR_BRAKE:   rate = 2.0f; break;
        default:               rate = 0.3f; break;
    }
    s_d.speed_kmh = approach(s_d.speed_kmh, target, rate, dt);
    if(s_d.speed_kmh < 0.05f) s_d.speed_kmh = 0.0f;

    /* Fuel */
    s_d.fuel_rate_lph = s_d.rpm > 0 ? 1.5f + 23.0f * powf(s_d.rpm / RPM_MAX, 2.0f) : 0.0f;
    float used = s_d.fuel_rate_lph * dt / 3600.0f * FUEL_BURN_SPEEDUP;
    s_d.fuel_pct = clampf(s_d.fuel_pct - used / TANK_L * 100.0f, 0.0f, 100.0f);
    s_d.trip.fuel_used_l += used;

    /* Temperature, battery, hours, trip */
    float temp_target = s_force_overheat ? 110.0f : running ? TEMP_RUN_C : 20.0f;
    s_d.engine_temp_c = approach(s_d.engine_temp_c, temp_target, 0.15f, dt);
    s_d.battery_v = s_force_low_batt ? 11.4f : running ? 13.8f : 12.6f;
    if(s_d.rpm > 0) s_d.engine_hours += dt / 3600.0f;
    s_d.trip_km += s_d.speed_kmh * dt / 3600.0f;
    if(s_d.speed_kmh > 1.0f) s_ride_t += dt;
    s_d.trip.ride_time_s = (uint32_t)s_ride_t;
    s_d.trip.top_speed_kmh = fmaxf(s_d.trip.top_speed_kmh, s_d.speed_kmh);
    if(s_d.rpm > s_d.trip.max_rpm) s_d.trip.max_rpm = s_d.rpm;

    /* Warnings = manual toggles + automatic thresholds */
    uint32_t w = s_manual_warn;
    if(s_d.fuel_pct <= FUEL_LOW_PCT) w |= DASH_WARN_LOW_FUEL;
    if(s_d.engine_temp_c >= TEMP_OVERHEAT_C) w |= DASH_WARN_OVERHEAT;
    if(s_d.battery_v <= BATT_LOW_V) w |= DASH_WARN_LOW_BATTERY;
    s_d.warnings = w;
    s_d.error_code = (w & DASH_WARN_CHECK_ENGINE) ? 0x0122 : 0;
}

static void step_extras(float dt)
{
    /* GPS: drift the heading while moving (fake position: Sydney Harbour) */
    if(s_d.speed_kmh > 1.0f) s_d.marine.heading_deg = fmodf(s_d.marine.heading_deg + 4.0f * dt, 360.0f);
    float rad = s_d.marine.heading_deg * 3.14159f / 180.0f;
    float km = s_d.speed_kmh * dt / 3600.0f;
    s_d.marine.lat += cosf(rad) * km / 111.0f;
    s_d.marine.lon += sinf(rad) * km / 92.0f;

    /* Music */
    if(s_d.music.connected && s_d.music.playing) {
        s_music_t += dt;
        if(s_music_t >= s_d.music.len_s) load_track(s_track + 1);
        s_d.music.pos_s = (uint16_t)s_music_t;
    }

    time_t now = time(NULL);
    struct tm * tm = localtime(&now);
    s_d.clock_h = (int8_t)tm->tm_hour;
    s_d.clock_m = (int8_t)tm->tm_min;
}

static void tick_cb(lv_timer_t * t)
{
    LV_UNUSED(t);
    float dt = TICK_MS / 1000.0f;
    step_engine(dt);
    step_extras(dt);
    dash_data_set(&s_d);
}

/* ---------- dash_cmd (UI -> "boat") ---------- */

void dash_cmd_light_toggle(uint8_t index)
{
    if(index < DASH_LIGHT_COUNT) s_d.lights ^= (uint8_t)(1u << index);
}

void dash_cmd_music(dash_music_cmd_t cmd)
{
    if(!s_d.music.connected) return;
    switch(cmd) {
        case DASH_MUSIC_PLAY_PAUSE: s_d.music.playing = !s_d.music.playing; break;
        case DASH_MUSIC_NEXT:       load_track(s_track + 1); break;
        case DASH_MUSIC_PREV:       load_track(s_music_t > 3 ? s_track : s_track - 1); break;
    }
}

/* ---------- Keyboard ---------- */

bool fake_ecu_key(int key)
{
    switch(key) {
        case SDLK_w:      s_throttle = clampf(s_throttle + 0.1f, 0, 1); s_demo = false; break;
        case SDLK_s:      s_throttle = clampf(s_throttle - 0.1f, 0, 1); s_demo = false; break;
        case SDLK_SPACE:  s_throttle = 0; s_demo = false; break;
        case SDLK_EQUALS: s_d.fuel_pct = clampf(s_d.fuel_pct + 5, 0, 100); break;
        case SDLK_MINUS:  s_d.fuel_pct = clampf(s_d.fuel_pct - 5, 0, 100); break;
        case SDLK_f:      s_d.ibr = DASH_IBR_FORWARD; break;
        case SDLK_n:      s_d.ibr = DASH_IBR_NEUTRAL; break;
        case SDLK_r:      s_d.ibr = DASH_IBR_REVERSE; break;
        case SDLK_b:      s_d.ibr = DASH_IBR_BRAKE; break;
        case SDLK_m:      s_d.mode = (s_d.mode + 1) % DASH_MODE_COUNT; break;
        case SDLK_k:      s_d.dess_ok = !s_d.dess_ok; break;
        case SDLK_p:      s_d.music.connected = !s_d.music.connected; break;
        case SDLK_1:      s_manual_warn ^= DASH_WARN_CHECK_ENGINE; break;
        case SDLK_2:      s_manual_warn ^= DASH_WARN_OIL_PRESSURE; break;
        case SDLK_3:      s_force_overheat = !s_force_overheat; break;
        case SDLK_4:      s_d.fuel_pct = s_d.fuel_pct > FUEL_LOW_PCT ? 10 : 80; break;
        case SDLK_5:      s_force_low_batt = !s_force_low_batt; break;
        case SDLK_0:      s_manual_warn = 0; s_force_overheat = false; s_force_low_batt = false; break;
        case SDLK_a:      s_demo = !s_demo; break;
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
    s_d.lights = 0x01;

    s_d.marine.gps_fix = true;
    s_d.marine.lat = -33.8568f;
    s_d.marine.lon = 151.2153f;
    s_d.marine.heading_deg = 45.0f;
    s_d.marine.water_temp_ok = true;
    s_d.marine.water_temp_c = 21.5f;
    s_d.marine.tide_ok = true;
    s_d.marine.tide_m = 0.9f;
    s_d.marine.tide_min_m = 0.3f;
    s_d.marine.tide_max_m = 1.8f;

    s_d.music.connected = true;
    s_d.music.playing = true;
    snprintf(s_d.music.phone, DASH_TEXT_LEN, "IPHONE");
    load_track(0);

    lv_timer_create(tick_cb, TICK_MS, NULL);
}
