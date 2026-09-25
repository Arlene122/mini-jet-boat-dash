/**
 * dash_cmd — actions requested by the UI (lights, music).
 *
 * The UI only asks; the platform decides how: the simulator flips fake
 * state, the boat drives relays / talks to the audio board. Results come
 * back through dash_data like any other value.
 * Implemented by the simulator (sim/) and later by the P4 firmware.
 */
#ifndef DASH_CMD_H
#define DASH_CMD_H

#include <stdint.h>

typedef enum {
    DASH_MUSIC_PLAY_PAUSE = 0,
    DASH_MUSIC_NEXT,
    DASH_MUSIC_PREV,
} dash_music_cmd_t;

/* Toggle boat light 0..DASH_LIGHT_COUNT-1 */
void dash_cmd_light_toggle(uint8_t index);

void dash_cmd_music(dash_music_cmd_t cmd);

/* Put the audio board in pairing mode for a new phone */
void dash_cmd_bt_pair(void);

/* Screen brightness 10..100 % (P4: backlight / panel control) */
void dash_cmd_brightness(uint8_t pct);

#endif /* DASH_CMD_H */
