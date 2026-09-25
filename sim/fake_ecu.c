/**
 * fake_ecu — pretend Spark ECU + fake GPS / audio board / light relays.
 *
 * Engine values go out as CAN frames (placeholder protocol, spark_can.h)
 * through the simulated bus, so the dash decodes them exactly like it
 * will on the boat. GPS, music and lights are written straight into
 * dash_data (they come from other boards, not the ECU).
 * All numbers are made-up but plausible; nothing here is real BRP data.
 */
#include "fake_ecu.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "lvgl.h"
#include LV_SDL_INCLUDE_PATH
#include "sim_platform.h"
#include "../src/can/spark_can.h"
#include "../src/dash_data/dash_cmd.h"
#include "../src/dash_data/dash_data.h"

/* ---------- Tuning ---------- */

#define TICK_MS          20
#define RPM_IDLE         1700.0f
#define RPM_MAX          8000.0f
#define RPM_ECO_MAX      6000.0f
#define SPEED_MAX_KMH    80.0f        /* 2015 Spark 900 HO ~48-50 mph */
#define SPEED_REV_KMH    8.0f
#define TEMP_RUN_C       82.0f
#define TANK_L           30.0f
#define FUEL_BURN_SPEEDUP 20.0f       /* so the gauge visibly moves */

/* Frame periods (ms) */
#define PERIOD_ENGINE    20
#define PERIOD_STATUS    100
#define PERIOD_FUEL      250
#define PERIOD_HOURS     1000

/* ---------- Engine state (the "ECU") ---------- */

static struct {
    float rpm, temp_c, batt_v, fuel_pct, rate_lph, hours;
    dash_ibr_t ibr;
    dash_mode_t mode;
    bool dess;
    uint32_t faults;
} s_ecu;

static float s_speed_kmh;       /* boat physics -> fake GPS */
static float s_throttle;        /* 0..1 */
static bool s_force_overheat, s_force_low_batt;
static bool s_demo;
static float s_demo_t, s_music_t;
static uint32_t s_ms;
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
    dash_music_t * m = &dash_data_edit()->music;
    s_track = (i + TRACK_COUNT) % TRACK_COUNT;
    snprintf(m->title, DASH_TEXT_LEN, "%s", TRACKS[s_track].title);
    snprintf(m->artist, DASH_TEXT_LEN, "%s", TRACKS[s_track].artist);
    m->len_s = TRACKS[s_track].len;
    s_music_t = 0;
}

/* ---------- Engine + boat physics ---------- */

static void step_engine(float dt)
{
    if(s_demo) {
        s_demo_t += dt;
        s_throttle = 0.5f + 0.5f * sinf(s_demo_t * 0.35f);
        s_ecu.ibr = DASH_IBR_FORWARD;
    }

    bool running = s_ecu.dess;
    float cap = s_ecu.mode == DASH_MODE_ECO ? RPM_ECO_MAX : RPM_MAX;
    float target = running ? RPM_IDLE + s_throttle * (cap - RPM_IDLE) : 0.0f;
    s_ecu.rpm = approach(s_ecu.rpm, target, s_ecu.mode == DASH_MODE_SPORT ? 6.0f : 3.5f, dt);

    float thrust = clampf((s_ecu.rpm - 2200.0f) / (RPM_MAX - 2200.0f), 0.0f, 1.0f);
    float vt = 0.0f, rate = 0.6f;
    switch(s_ecu.ibr) {
        case DASH_IBR_FORWARD: vt = thrust * SPEED_MAX_KMH; break;
        case DASH_IBR_REVERSE: vt = thrust * SPEED_REV_KMH; break;
        case DASH_IBR_BRAKE:   rate = 2.0f; break;
        default:               rate = 0.3f; break;
    }
    s_speed_kmh = approach(s_speed_kmh, vt, rate, dt);
    if(s_speed_kmh < 0.05f) s_speed_kmh = 0.0f;

    s_ecu.rate_lph = s_ecu.rpm > 0 ? 1.5f + 23.0f * powf(s_ecu.rpm / RPM_MAX, 2.0f) : 0.0f;
    s_ecu.fuel_pct = clampf(s_ecu.fuel_pct - s_ecu.rate_lph * dt / 3600.0f * FUEL_BURN_SPEEDUP / TANK_L * 100.0f,
                            0.0f, 100.0f);
    s_ecu.temp_c = approach(s_ecu.temp_c, s_force_overheat ? 110.0f : running ? TEMP_RUN_C : 20.0f, 0.15f, dt);
    s_ecu.batt_v = s_force_low_batt ? 11.4f : running ? 13.8f : 12.6f;
    if(s_ecu.rpm > 1) s_ecu.hours += dt / 3600.0f;

    uint32_t f = s_ecu.faults & ~DASH_WARN_OVERHEAT;
    if(s_ecu.temp_c >= 100.0f) f |= DASH_WARN_OVERHEAT;
    s_ecu.faults = f;
}

static void send_frames(void)
{
    can_frame_t f;
    if(s_ms % PERIOD_ENGINE == 0) {
        spark_can_encode_engine(&f, (uint16_t)s_ecu.rpm, s_ecu.temp_c, s_ecu.batt_v);
        sim_can_inject(&f);
    }
    if(s_ms % PERIOD_STATUS == 0) {
        uint16_t code = (s_ecu.faults & DASH_WARN_CHECK_ENGINE) ? 0x0122 : 0;
        spark_can_encode_status(&f, s_ecu.ibr, s_ecu.mode, s_ecu.faults, s_ecu.dess, code);
        sim_can_inject(&f);
    }
    if(s_ms % PERIOD_FUEL == 0) {
        spark_can_encode_fuel(&f, s_ecu.fuel_pct, s_ecu.rate_lph);
        sim_can_inject(&f);
    }
    if(s_ms % PERIOD_HOURS == 0) {
        spark_can_encode_hours(&f, s_ecu.hours);
        sim_can_inject(&f);
    }
}

/* ---------- Other boards: GPS, audio, clock ---------- */

static void step_other_boards(float dt)
{
    dash_data_t * d = dash_data_edit();

    /* Fake GPS (Sydney Harbour), heading drifts while moving */
    d->speed_kmh = s_speed_kmh;
    dash_marine_t * m = &d->marine;
    if(s_speed_kmh > 1.0f) m->heading_deg = fmodf(m->heading_deg + 4.0f * dt, 360.0f);
    float rad = m->heading_deg * 3.14159f / 180.0f;
    float km = s_speed_kmh * dt / 3600.0f;
    m->lat += cosf(rad) * km / 111.0f;
    m->lon += sinf(rad) * km / 92.0f;

    /* Fake audio board */
    if(d->music.connected && d->music.playing) {
        s_music_t += dt;
        if(s_music_t >= d->music.len_s) load_track(s_track + 1);
        d->music.pos_s = (uint16_t)s_music_t;
    }

    time_t now = time(NULL);
    struct tm * tm = localtime(&now);
    d->clock_h = (int8_t)tm->tm_hour;
    d->clock_m = (int8_t)tm->tm_min;
}

static void tick_cb(lv_timer_t * t)
{
    LV_UNUSED(t);
    float dt = TICK_MS / 1000.0f;
    s_ms += TICK_MS;
    step_engine(dt);
    send_frames();
    step_other_boards(dt);
}

/* ---------- dash_cmd (UI -> "boat") ---------- */

void dash_cmd_light_toggle(uint8_t index)
{
    if(index < DASH_LIGHT_COUNT) dash_data_edit()->lights ^= (uint8_t)(1u << index);
}

void dash_cmd_music(dash_music_cmd_t cmd)
{
    dash_music_t * m = &dash_data_edit()->music;
    if(!m->connected) return;
    switch(cmd) {
        case DASH_MUSIC_PLAY_PAUSE: m->playing = !m->playing; break;
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
        case SDLK_EQUALS: s_ecu.fuel_pct = clampf(s_ecu.fuel_pct + 5, 0, 100); break;
        case SDLK_MINUS:  s_ecu.fuel_pct = clampf(s_ecu.fuel_pct - 5, 0, 100); break;
        case SDLK_f:      s_ecu.ibr = DASH_IBR_FORWARD; break;
        case SDLK_n:      s_ecu.ibr = DASH_IBR_NEUTRAL; break;
        case SDLK_r:      s_ecu.ibr = DASH_IBR_REVERSE; break;
        case SDLK_b:      s_ecu.ibr = DASH_IBR_BRAKE; break;
        case SDLK_m:      s_ecu.mode = (s_ecu.mode + 1) % DASH_MODE_COUNT; break;
        case SDLK_k:      s_ecu.dess = !s_ecu.dess; break;
        case SDLK_e:      sim_can_set_connected(!sim_can_connected()); break;
        case SDLK_p:      dash_data_edit()->music.connected = !dash_data_get()->music.connected; break;
        case SDLK_1:      s_ecu.faults ^= DASH_WARN_CHECK_ENGINE; break;
        case SDLK_2:      s_ecu.faults ^= DASH_WARN_OIL_PRESSURE; break;
        case SDLK_3:      s_force_overheat = !s_force_overheat; break;
        case SDLK_4:      s_ecu.fuel_pct = s_ecu.fuel_pct > 15 ? 10 : 80; break;
        case SDLK_5:      s_force_low_batt = !s_force_low_batt; break;
        case SDLK_0:      s_ecu.faults = 0; s_force_overheat = false; s_force_low_batt = false; break;
        case SDLK_a:      s_demo = !s_demo; break;
        default: return false;
    }
    return true;
}

/* ---------- API ---------- */

void fake_ecu_init(void)
{
    s_ecu.dess = true;
    s_ecu.fuel_pct = 75.0f;
    s_ecu.hours = 42.0f;
    s_ecu.temp_c = 20.0f;
    s_ecu.batt_v = 12.6f;
    s_ecu.mode = DASH_MODE_TOURING;

    dash_data_t * d = dash_data_edit();
    d->source = DASH_SRC_SIM;
    d->lights = 0x01;
    d->marine = (dash_marine_t){
        .gps_fix = true, .lat = -33.8568f, .lon = 151.2153f, .heading_deg = 45.0f,
        .water_temp_ok = true, .water_temp_c = 21.5f,
        .tide_ok = true, .tide_m = 0.9f, .tide_min_m = 0.3f, .tide_max_m = 1.8f,
    };
    d->music.connected = true;
    d->music.playing = true;
    snprintf(d->music.phone, DASH_TEXT_LEN, "IPHONE");
    load_track(0);

    lv_timer_create(tick_cb, TICK_MS, NULL);
}
