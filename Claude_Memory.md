# Claude Memory — Mini Jet Boat Dash
_Updated 2026-09-25 · compact, indexed; detail lives in code/README._

## Index
1 Status · 2 Decisions · 3 UI · 4 Architecture · 5 Hardware · 6 Lessons · 7 Open · 8 Next · 9 Log

## 1. Status
- Phase 1–2 (sim) well advanced. Live sim: https://arlene122.github.io/mini-jet-boat-dash/ (deploys from `main`).
- Work branch `claude/compassionate-mendel-bp8lbr`; PR #4 merged; PR #5 = smoothness + settings exit.
- UI **not locked**; lock before reveal animation.

## 2. Decisions
- Spark 2015 900 HO: modes Touring (default) + Sport; ECO UNVERIFIED. Top ~80 km/h → scale 0–105.
- ESP32-P4 + LVGL v9.6 (C). Audio on separate ESP32 (BT). Sim = browser WASM (dev tool only; dash UI is C/LVGL).
- Speed from GPS; engine via CAN. Units km/h/kn/mph (distance follows), °C/°F, 12/24 h.
- Phone: auto-reconnect to last phone; pairing from Music page (not Settings).
- Settings = set-and-forget, **not a swipe page**: knob hold opens/closes (no dedicated button); CLOSE row + "hold knob to close" hint; 5-way left = back.
- Requirements.md changes only when owner says (Rule 6).

## 3. UI (current)
- Frame: glowing accent lines top/bottom with centre "pinch"; tilted gradient bars in the notch (top: trip|clock|hours; bottom: mode|warnings|DESS·source).
- Centre: glowing 270° speed ring + digital speed, alt unit, iBR pill. Mirrored **half-hexagon** gauges (top/bottom symmetric → middle = 50 %, ticks 25/50/75; long straight side 300 px / short ends — owner prefers; old even version = `BRK_OUT_X 600, BRK_MID_Y1 262`; numbers 34 px): RPM left (red ≥7000), FUEL right (amber ≤15%).
- Left zone (keep clean): status icons (log, Wi-Fi, GPS, BT) + engine temp, battery, fuel use; fading hairline edge.
- Right page zone (no box, fading hairline): swipe pages **NAV, MUSIC, LIGHTS, RIDE**.
  - NAV: push toggles **Map** (map fills panel; offline tiles later) ↔ **Marine** (compass, depth, water temp, tide chart w/ next low/high times, coords).
  - RIDE: distance hero, top speed/ride time/max RPM, fuel used, hours, error code.
- Accent = tone family per mode (deep/main/light/soft): Touring ocean→ice-cyan, Sport ember→orange, Eco forest→green; deep for glows, light for highlights, soft for captions; amber/red only for alerts.
- New warning → banner over page zone (plain words + action), knob push acks; bottom icon stays lit.
- NO ECU (no frames 1 s): engine values "--", amber NO ECU.
- Key-on arc sweep. Refs: old HTML = layout only; owner ref dribbble 26477944 (blocked — need screenshots).

## 4. Architecture
- `src/dash_data` (UI's only input; `dash_task` = CAN decode, ECU timeout, derived warnings, trip) · `src/can` (**placeholder protocol, UNVERIFIED**) · `src/settings` (versioned blob) · `src/ui` (+`pages/`, zones in `ui_layout.h`) · `sim/`.
- P4 must implement: `can_bus_receive` (TWAI), `settings_store_load/save` (NVS), `dash_cmd_*` (relays, audio/BT, brightness). **No CAN TX API** (safety).
- Controls abstracted in `ui_input.h` (NEXT/PREV/SELECT/BACK/SETTINGS/UP/DOWN).

## 5. Hardware
Wisecoco 12.3" 1920×720 HDMI (owned) · ESP32-P4 + LT8912B (to buy) · USB-CAN (to buy) · stock gauge kept for sniffing · engine not runnable yet.

## 6. Lessons
- HTML dash too laggy; Pi boot too slow; never assume BRP CAN IDs.
- LVGL 9.6: use `lv_obj_set_hidden/…` (flag APIs deprecated); read sizes via constants, not `lv_obj_get_width` at create time.
- Sim perf: stencil PNG re-decoded every frame when LV_CACHE_DEF_SIZE=0 → 70 ms/frame; 8 MB cache → 4 ms. Measure with LV_USE_SYSMON/PERF_MONITOR (temp build).
- Smooth gauges: arcs in 0.1 units (1 km/h steps look jumpy); number hysteresis ±0.8 stops flicker.
- Dark low-contrast gradients band → use solid fills; glow = opaque pre-mixed passes (no alpha overlap beads).
- Sandbox blocks Emscripten SDL2 zip → `EMCC_LOCAL_PORTS=sdl2=<SDL git clone>`; blocks dribbble/manualslib.

## 7. Open
5 light names · 4 button jobs (map mode?) · wake signal · fuel sender · dash opening size · P4 board model · stencil is eye-traced (SVG better) · owner msg "add om…" cut off · P4 perf of glows untested.

## 8. Next
Owner review → lock UI → reveal/splash animation → ride-log format → CAN replay (candump) → buy P4 + USB-CAN.

## 9. Log
- 09-25 Half-hex gauges + tone family.
- 09-25 Planning; sim + CI; UI v1→v2 (ring, brackets, boxless pages); settings + saving; CAN pipeline; Nav map/marine, Music BT, Settings overlay, 4 pages.
