# Claude Memory — Mini Jet Boat Dash
_Last updated: 2026-09-25 · Keep compact. Update often._

## Index
1. Status · 2. Key Decisions · 3. Hardware · 4. Lessons / Avoid · 5. Open Items · 6. Next Step · 7. Change Log

## 1. Status
Phase 1 in progress. Repo structure + LVGL v9.6.0 simulator done; builds natively (SDL2) and to WebAssembly.
GitHub Actions (`web-sim.yml`) builds every push, deploys to Pages from `main` → https://arlene122.github.io/mini-jet-boat-dash/
Main screen = **placeholder** layout (speed/RPM/fuel/iBR/mode/DESS/warnings/trip/hours/clock, tilted bars).
Fake Spark ECU (keys): ↑↓ throttle, Space off, ←→ fuel, F/N/R/B iBR, M mode, K DESS, 1–5 warnings, 0 clear, A demo, S stencil, H help.

## 2. Key Decisions
- Mini jet boat w/ 2015 Spark engine+ECU; dash **replaces stock gauge**; iBR/modes/DESS must work.
- ESP32-P4 + LVGL v9 (C, ESP-IDF). Audio on separate classic ESP32.
- Screen v1: owned Wisecoco 12.3" 1920×720 HDMI 500 nit via LT8912B bridge. Upgrade later to 850–1000 nit.
- Boot ≤3 s (splash covers). Off instantly (hold-up cap).
- Speed from GPS; engine data from CAN. Units switchable.
- Controls: SF39BA 5-way + SLD encoder + 4 buttons (functions TBD). No touch.
- Weather/tide via phone hotspot. Updates USB + OTA.
- Warnings: icons + soft tone via speakers (duck music, forced min vol).
- Logging toggle, phone-app-ready format. Settings persisted.
- Keep old HTML's tilted top/bottom bars; restyle modern/minimal.
- Simulator runs in browser (LVGL C → WebAssembly, GitHub Pages) = dev tool only; dash UI stays C/LVGL (not an HTML UI). No installs for user.
- Pinned: LVGL v9.6.0 (CMake FetchContent), Emscripten 3.1.74. `sim/lv_conf.h` from v9.6.0 template.
- Structure: `src/dash_data` (only UI input) · `src/ui` (screens/widgets, shared w/ P4) · `sim/` (SDL main, fake_ecu, stencil/help overlay) · `tools/stencil_to_c.py`.
- Big digits = generated Montserrat SemiBold fonts (200/96 px, digits only) via lv_font_conv.
- Stencil = sim-only top-layer overlay; `assets/stencil.png` if present else `stencil_placeholder.png` (transparent/white = visible).
- Fake ECU writes dash_data directly for now; Phase 2 → emit CAN frames through the real decoder.

## 3. Hardware Status
| Item | Status |
|---|---|
| Wisecoco screen | Owned |
| ESP32-P4 + LT8912B | To buy (model TBD) |
| Stock Spark gauge | Disassembled — **keep, needed for CAN sniffing** |
| Engine/ECU | In boat build, not runnable yet |

## 4. Lessons / Avoid
- HTML dash = too laggy. Don't revisit.
- Pi/Linux boot too slow.
- BRP CAN undocumented — never assume IDs.
- LVGL v9.6: `lv_obj_add/remove_flag` deprecated → use `lv_obj_set_hidden/scrollable/clickable`. Public headers moved to `include/`.
- Claude sandbox blocks Emscripten's SDL2 zip download → local test: `EMCC_LOCAL_PORTS=sdl2=<git clone of SDL release-2.30.9>`. CI unaffected.

## 5. Open Items
**Real stencil image not in repo yet** (user to upload `assets/stencil.png`) · Pages source must be set to "GitHub Actions" + branch merged to main · Visual style (give 2–3 options) · 4 button functions · knob display · wake signal · fuel sender wiring · dash opening size · P4 board model.

## 6. Next Step
Phase 1 cont.: once real stencil uploaded, fit layout to it; propose 2–3 UI style concepts at 1920×720 (switchable in sim).

## 7. Change Log
- 2026-09-25: Planning complete, docs created.
- 2026-09-25: Rule 2 clarified: `Chat:` = planning mode, no code/file changes.
- 2026-09-25: Phase 1 start — repo structure, LVGL sim (native + WASM), fake ECU, placeholder screen, Pages CI.
