/**
 * settings — user preferences, saved across power cycles.
 * Storage goes through settings_store.h (simulator: browser / file,
 * P4: NVS flash — written when the board arrives).
 */
#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdbool.h>
#include <stdint.h>

typedef enum { UNIT_KMH = 0, UNIT_KNOTS, UNIT_MPH, UNIT_SPEED_COUNT } speed_unit_t;

typedef struct {
    uint8_t speed_unit;     /* speed_unit_t (distance follows: km / nm / mi) */
    uint8_t temp_f;         /* 0 = °C, 1 = °F */
    uint8_t clock_12h;      /* 0 = 24 h, 1 = 12 h */
    uint8_t brightness;     /* 10..100 % */
    uint8_t logging;        /* ride logging on/off */
} settings_t;

void settings_init(void);                 /* load, or defaults if none/corrupt */
const settings_t * settings_get(void);
void settings_update(const settings_t * s);   /* apply + save */
uint32_t settings_seq(void);              /* bumps on every change */

/* ---------- Unit helpers ---------- */
float settings_speed(float kmh);              /* in chosen unit */
const char * settings_speed_unit(void);       /* "KM/H", "KNOTS", "MPH" */
float settings_speed_alt(float kmh);          /* secondary unit value */
const char * settings_speed_alt_unit(void);   /* "kn" or "km/h" */
float settings_dist(float km);
const char * settings_dist_unit(void);        /* "km", "nm", "mi" */
int settings_temp(float c);
const char * settings_temp_unit(void);        /* "°C" / "°F" */

/* Persistence backend (platform) */
bool settings_store_load(void * buf, uint32_t len);
bool settings_store_save(const void * buf, uint32_t len);

#endif /* SETTINGS_H */
