/**
 * settings — versioned blob with magic + checksum so a corrupt or old
 * save falls back to defaults instead of garbage.
 */
#include "settings.h"

#include <string.h>

/* ---------- Storage format ---------- */

#define SETTINGS_MAGIC    0x4A424431u   /* "JBD1" */
#define SETTINGS_VERSION  1

typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t sum;
    settings_t s;
} settings_blob_t;

static const settings_t DEFAULTS = {
    .speed_unit = UNIT_KMH, .temp_f = 0, .clock_12h = 0, .brightness = 100, .logging = 1,
};

/* ---------- State ---------- */

static settings_t s_cur;
static uint32_t s_seq;

/* ---------- Helpers ---------- */

static uint16_t checksum(const settings_t * s)
{
    const uint8_t * p = (const uint8_t *)s;
    uint16_t sum = 0x5A5A;
    for(uint32_t i = 0; i < sizeof(*s); i++) sum = (uint16_t)((sum << 1 | sum >> 15) ^ p[i]);
    return sum;
}

static bool valid(const settings_t * s)
{
    return s->speed_unit < UNIT_SPEED_COUNT && s->temp_f <= 1 && s->clock_12h <= 1 &&
           s->brightness >= 10 && s->brightness <= 100 && s->logging <= 1;
}

/* ---------- API ---------- */

void settings_init(void)
{
    settings_blob_t b;
    s_cur = DEFAULTS;
    if(settings_store_load(&b, sizeof(b)) && b.magic == SETTINGS_MAGIC &&
       b.version == SETTINGS_VERSION && b.sum == checksum(&b.s) && valid(&b.s)) {
        s_cur = b.s;
    }
    s_seq++;
}

const settings_t * settings_get(void)
{
    return &s_cur;
}

void settings_update(const settings_t * s)
{
    if(!valid(s) || memcmp(s, &s_cur, sizeof(*s)) == 0) return;
    s_cur = *s;
    s_seq++;
    settings_blob_t b = { SETTINGS_MAGIC, SETTINGS_VERSION, checksum(&s_cur), s_cur };
    settings_store_save(&b, sizeof(b));
}

uint32_t settings_seq(void)
{
    return s_seq;
}

/* ---------- Units ---------- */

float settings_speed(float kmh)
{
    switch(s_cur.speed_unit) {
        case UNIT_KNOTS: return kmh / 1.852f;
        case UNIT_MPH:   return kmh / 1.609344f;
        default:         return kmh;
    }
}

const char * settings_speed_unit(void)
{
    static const char * n[] = { "KM/H", "KNOTS", "MPH" };
    return n[s_cur.speed_unit];
}

float settings_speed_alt(float kmh)
{
    return s_cur.speed_unit == UNIT_KNOTS ? kmh : kmh / 1.852f;
}

const char * settings_speed_alt_unit(void)
{
    return s_cur.speed_unit == UNIT_KNOTS ? "km/h" : "kn";
}

float settings_dist(float km)
{
    switch(s_cur.speed_unit) {
        case UNIT_KNOTS: return km / 1.852f;
        case UNIT_MPH:   return km / 1.609344f;
        default:         return km;
    }
}

const char * settings_dist_unit(void)
{
    static const char * n[] = { "km", "nm", "mi" };
    return n[s_cur.speed_unit];
}

int settings_temp(float c)
{
    float v = s_cur.temp_f ? c * 9.0f / 5.0f + 32.0f : c;
    return (int)(v < 0 ? v - 0.5f : v + 0.5f);
}

const char * settings_temp_unit(void)
{
    return s_cur.temp_f ? "°F" : "°C";
}
