/**
 * sim_platform — simulator versions of the platform hooks:
 *   CAN bus (loopback queue fed by the fake ECU)
 *   settings storage (browser localStorage / local file)
 * The P4 firmware provides its own versions (TWAI driver, NVS flash).
 */
#include "sim_platform.h"

#include <stdio.h>

#include "../src/can/can_frame.h"
#include "../src/settings/settings.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

/* ---------- CAN loopback ---------- */

#define CAN_QUEUE 64

static can_frame_t s_q[CAN_QUEUE];
static int s_head, s_tail;
static bool s_connected = true;

void sim_can_inject(const can_frame_t * f)
{
    if(!s_connected) return;                 /* "unplugged" */
    int next = (s_head + 1) % CAN_QUEUE;
    if(next == s_tail) return;               /* full: drop, like a busy bus */
    s_q[s_head] = *f;
    s_head = next;
}

void sim_can_set_connected(bool on) { s_connected = on; }
bool sim_can_connected(void) { return s_connected; }

bool can_bus_receive(can_frame_t * f)
{
    if(s_tail == s_head) return false;
    *f = s_q[s_tail];
    s_tail = (s_tail + 1) % CAN_QUEUE;
    return true;
}

/* ---------- Settings storage ---------- */

#ifdef __EMSCRIPTEN__
EM_JS(int, js_settings_load, (void * buf, int len), {
    try {
        var s = localStorage.getItem('jbdash_settings');
        if (!s) return 0;
        var b = atob(s);
        if (b.length != len) return 0;
        for (var i = 0; i < len; i++) HEAPU8[buf + i] = b.charCodeAt(i);
        return 1;
    } catch (e) { return 0; }
});
EM_JS(void, js_settings_save, (const void * buf, int len), {
    try {
        var s = "";
        for (var i = 0; i < len; i++) s += String.fromCharCode(HEAPU8[buf + i]);
        localStorage.setItem('jbdash_settings', btoa(s));
    } catch (e) {}
});

bool settings_store_load(void * buf, uint32_t len) { return js_settings_load(buf, (int)len) == 1; }
bool settings_store_save(const void * buf, uint32_t len) { js_settings_save(buf, (int)len); return true; }
#else
#define SETTINGS_FILE "dash_settings.bin"

bool settings_store_load(void * buf, uint32_t len)
{
    FILE * f = fopen(SETTINGS_FILE, "rb");
    if(!f) return false;
    bool ok = fread(buf, 1, len, f) == len;
    fclose(f);
    return ok;
}

bool settings_store_save(const void * buf, uint32_t len)
{
    FILE * f = fopen(SETTINGS_FILE, "wb");
    if(!f) return false;
    bool ok = fwrite(buf, 1, len, f) == len;
    fclose(f);
    return ok;
}
#endif
