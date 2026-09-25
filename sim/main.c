/**
 * Simulator entry point — runs natively (SDL window) or in a browser
 * (Emscripten -> WebAssembly). Same dash code as the ESP32-P4 build.
 */
#include "lvgl.h"
#include LV_SDL_INCLUDE_PATH

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "../src/dash_data/dash_data.h"
#include "../src/ui/ui.h"
#include "fake_ecu.h"
#include "sim_overlay.h"

/* ---------- Keyboard ---------- */

/* Sees every SDL event before LVGL's driver consumes it. */
static int key_watch(void * user, SDL_Event * e)
{
    LV_UNUSED(user);
    if(e->type != SDL_KEYDOWN) return 0;
    int key = e->key.keysym.sym;
    if(!fake_ecu_key(key)) sim_overlay_key(key);
    return 0;
}

/* ---------- Main loop ---------- */

#ifdef __EMSCRIPTEN__
static void loop_once(void)
{
    lv_timer_handler();
}
#endif

int main(void)
{
    lv_init();

    lv_display_t * disp = lv_sdl_window_create(UI_HOR_RES, UI_VER_RES);
    lv_sdl_window_set_title(disp, "Mini Jet Boat Dash - Simulator");
    SDL_AddEventWatch(key_watch, NULL);

    dash_data_init();
    fake_ecu_init();
    ui_init();
    sim_overlay_init();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(loop_once, 0, 1);
#else
    while(1) {
        uint32_t wait_ms = lv_timer_handler();
        if(wait_ms > 10) wait_ms = 10;
        SDL_Delay(wait_ms);
    }
#endif
    return 0;
}
