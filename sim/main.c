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
#include "../src/dash_data/dash_task.h"
#include "../src/settings/settings.h"
#include "../src/ui/ui.h"
#include "../src/ui/ui_input.h"
#include "fake_ecu.h"
#include "sim_overlay.h"

/* ---------- Keyboard ---------- */

#define LONG_PRESS_MS 600

static uint32_t s_enter_down_ms;   /* 0 = not held */

/* Sees every SDL event before LVGL's driver consumes it.
 * Enter = knob push: short press = select, hold = settings. */
static int key_watch(void * user, SDL_Event * e)
{
    LV_UNUSED(user);
    if(e->type == SDL_KEYUP && e->key.keysym.sym == SDLK_RETURN && s_enter_down_ms) {
        bool longp = SDL_GetTicks() - s_enter_down_ms >= LONG_PRESS_MS;
        s_enter_down_ms = 0;
        ui_input(longp ? UI_IN_SETTINGS : UI_IN_SELECT);
        return 0;
    }
    if(e->type != SDL_KEYDOWN || e->key.repeat) return 0;
    int key = e->key.keysym.sym;
    if(fake_ecu_key(key) || sim_overlay_key(key)) return 0;
    switch(key) {   /* dash controls: knob + 5-way + Settings button */
        case SDLK_RIGHT:     ui_input(UI_IN_NEXT); break;
        case SDLK_LEFT:      ui_input(UI_IN_PREV); break;
        case SDLK_RETURN:    s_enter_down_ms = SDL_GetTicks(); break;
        case SDLK_BACKSPACE:
        case SDLK_ESCAPE:    ui_input(UI_IN_BACK); break;
        case SDLK_o:         ui_input(UI_IN_SETTINGS); break;
        case SDLK_UP:        ui_input(UI_IN_UP); break;
        case SDLK_DOWN:      ui_input(UI_IN_DOWN); break;
        default: break;
    }
    return 0;
}

/* ---------- Data loop (P4: its own task) ---------- */

static void dash_task_cb(lv_timer_t * t)
{
    LV_UNUSED(t);
    dash_task_step(lv_tick_get());
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
    settings_init();
    dash_task_init();
    fake_ecu_init();
    lv_timer_create(dash_task_cb, 10, NULL);
    sim_overlay_init();
    ui_init();

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
