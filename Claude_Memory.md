# Claude Memory — Mini Jet Boat Dash
_Last updated: 2026-09-25 · Keep compact. Update often._

## Index
1. Status · 2. Key Decisions · 3. Hardware · 4. Lessons / Avoid · 5. Open Items · 6. Next Step · 7. Change Log

## 1. Status
Phase 1 in progress. Repo structure + LVGL v9.6.0 simulator done; builds natively (SDL2) and to WebAssembly.
GitHub Actions (`web-sim.yml`) builds every push, deploys to Pages from `main` → https://arlene122.github.io/mini-jet-boat-dash/
UI v2: ring speedo (ref image), mirrored bracket gauges (RPM left, fuel right; same reach above/below ring centre), left stat list right-aligned; page zone has **no box** (fading hairlines, mirrors engine side); pages: one hero value each, hairline lists (Marine compass, Music art tile, Lights, Trip, System, **Settings**). **UI not locked yet** — lock before reveal animation. Owner ref to adapt: dribbble 26477944 (EV cluster; blocked here — ask for screenshots). Stencil traced → `assets/stencil.png`.
Data: fake ECU → CAN frames (**placeholder protocol**, `src/can/spark_can.*`, all UNVERIFIED) → sim bus → `dash_task` decode + ECU timeout (1 s → "NO ECU", engine values "--") + dash-side low fuel/batt + trip stats.
Settings (units km/h/kn/mph + distance, °C/°F, 12/24 h, brightness, logging) saved via `settings_store_*` (sim: browser localStorage / file; P4: NVS later).
Sim keys: ←→ knob pages, Enter knob push/ack, ↑↓ 5-way, Esc home · W/S throttle, Space off, -/= fuel, F/N/R/B iBR, M mode, K DESS, P phone, E unplug ECU, 1–5 warnings, 0 clear, A demo · T stencil, H help.

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
- **UI design (agreed):** modern-minimal + subtle futuristic. Layout: top tilted bar (trip | clock | hours), left engine panel (RPM, fuel, engine temp, battery), centre gauge = big digital speed + slim 240° speed arc w/ ticks + **RPM arc inside**, knots, iBR R/N/F (BRAKE) in arc gap; right **page card** (Marine, Music, Lights, System, Trip; knob changes page); bottom tilted bar (mode | warnings | DESS·source). Bars follow stencil slants.
- Accent = **ice-cyan**, tints by mode (Touring cyan, Sport orange, Eco green); amber/red only for alerts. Near-black bg.
- Frame: glowing accent lines across top & bottom that dip ("pinch") in the middle; top/bottom bars sit in the notch, tilted toward centre, horizontal gradient (dark ends, accent-tinted middle) — from owner's old HTML look.
- New warning → banner over page card (plain words + action), knob push acks; bottom-bar icon stays lit.
- Platform hooks the P4 must implement: `can_bus_receive` (TWAI), `settings_store_load/save` (NVS), `dash_cmd_*` (relays, audio, brightness). No CAN TX API on purpose (safety).
- Key-on sweep of arcs covers boot. UI→boat actions via `dash_cmd.h`; controls via `ui_input.h`; zones in `ui_layout.h`.
- Spark 2015 900 HO: modes Touring (default) + Sport (manual); ECO UNVERIFIED. Top ~48–50 mph ≈ 80 km/h → speed scale 0–105 km/h.
- Design refs (owner, later): glowing segmented blue arcs, bottom icon dock, perspective grid floor, clean thin digits; refs too dark → keep ours a bit brighter. Wants a "reveal" load-in animation when dash appears (after splash); splash could be jet-boat motion video (owner/AI-made). Speedo stays centre unless steering blocks view.
- Old HTML dash (uploaded) = layout reference only; never reuse its code/styling. Kept ideas: toolbar/status strip, 4 swipe pages, 5 lights on boat top view, music unconnected state.

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
Stencil is eye-traced from image (exact SVG would be better) · 5 light names (all-round white, LED strips, basic lights — owner to confirm) · end of owner msg "eventually add om…" cut off · day/night theme · ride summary on key-off · warning tone · 4 button functions · knob display · wake signal · fuel sender wiring · dash opening size · P4 board model.

## 6. Next Step
Owner reviews UI v2 → tweak. Then: splash + reveal anim, day/night, ride summary, logging file format, CAN replay (candump). Hardware when bought: P4 board + USB-CAN.

## 7. Change Log
- 2026-09-25: Planning complete, docs created.
- 2026-09-25: Rule 2 clarified: `Chat:` = planning mode, no code/file changes.
- 2026-09-25: Page zone boxless + page redesign; brackets balanced.
- 2026-09-25: UI v2 (ring speedo, brackets, restyled pages), Settings page + saving, CAN placeholder pipeline + ECU timeout.
- 2026-09-25: UI v1 design: centre gauge + RPM inner arc, engine panel, 5-page card, warning banner, mode tint, sweep; stencil traced.
- 2026-09-25: Phase 1 start — repo structure, LVGL sim (native + WASM), fake ECU, placeholder screen, Pages CI.
