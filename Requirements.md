# Mini Jet Boat Dash — Requirements
_v1.0 · 2026-09-25_

## 1. Project
Custom digital dash for a home-built **mini jet boat** powered by a **2015 Sea-Doo Spark** engine + ECU (Rotax 900 ACE). Dash **fully replaces the stock Spark gauge**. Owner has no coding experience → Claude writes all code; instructions must be beginner-friendly (Windows).

## 2. Hard Requirements
| # | Requirement |
|---|---|
| R1 | Key-on → live gauges **≤3 s** (target <1 s for MCU; HDMI monitor sync adds ~1–3 s). Splash animation covers boot. |
| R2 | Instant readings — no visible lag on needles/values. |
| R3 | Stunning, modern, minimal, sleek UI. 1920×720. All content inside the visible **stencil cut-out** (bezel covers rest). |
| R4 | Keep stock functions working: **iBR** (fwd/neutral/reverse/brake), **modes**, **DESS key**. |
| R5 | Full PC simulation (dash + fake Spark ECU) on Windows before touching real hardware. Tested constantly. |
| R6 | Physical controls only (wet hands/gloves). No touch. |
| R7 | Off instantly with key; settings/logs saved safely (hold-up capacitor). |

## 3. Hardware (v1)
| Part | Choice | Notes |
|---|---|---|
| Display | Wisecoco 12.3" 1920×720, HDMI, 500 nit, glossy (owned) | Weak in sun → anti-glare film. **Upgrade path:** 850–1000 nit 12.3" bar panel |
| Main MCU | **ESP32-P4** dev board + **LT8912B** MIPI-DSI→HDMI bridge | Runs LVGL, CAN, GPS, inputs, relays, Wi-Fi (via on-board C6) |
| Audio | Classic **ESP32** (BT Classic) → I2S DAC → amp | A2DP + AVRCP (iPhone + Android). Sends song/artist to P4 |
| CAN | 3.3 V CAN transceiver (e.g. SN65HVD230) on P4 TWAI | |
| GPS | UART GPS module | Speed, distance, trip, time. Future maps |
| Inputs | Shanpu **SF39BA** 5-way nav + **SLD** rotary encoder/push + 4 extra buttons | IP65 → add boot/cover or use IP67. Knob mini-display & 4-button functions **TBD** |
| Outputs | 4–6 ch relay/MOSFET board | Boat lights |
| Fuel | Read stock Spark fuel sender (if gauge-wired) | Calibration table in settings (tank may change) |
| Power | 12 V→5 V buck, ignition/wake sense, hold-up cap | Wake signal TBD (DESS vs start button) |
| Dev tools | USB-CAN adapter (~$30) | For sniffing ECU↔gauge traffic |
Budget: display+board target ≤ $150 (screen already owned).

## 4. Data Sources
- **GPS:** speed (km/h, knots, mph — switchable), distance, trip, clock.
- **ECU (CAN, proprietary BRP — must reverse-engineer):** RPM, engine temp, faults/check engine, oil pressure, mode, iBR state, engine hours, battery V, fuel (TBD).
- **Phone hotspot (Wi-Fi):** weather, tide.
- **Audio ESP32:** track info, playback state.

## 5. Features v1
- Main: speedometer (+ secondary unit), RPM, fuel %, clock, trip, engine hours.
- Top bar + bottom status bar with slight 3D tilt (kept from old HTML).
- Status/warnings: check engine, oil pressure, overheat, low fuel, error code, iBR, mode — icons + **soft low-pitch tone** through speakers (music ducks, forced min volume).
- Pages: System info · Music · Lights (relay control) · Weather/Tide · Settings.
- Settings persisted: units, 12/24 h, manual time, brightness, logging on/off, fuel calibration.
- Ride logging (toggle): top speed, max RPM, trips — stored in a phone-app-ready format.
- Startup splash: user-supplied short logo animation, only for boot duration.
- Updates: USB **and** OTA via phone hotspot.

## 6. Future (design for, don't build)
Water temp sensor · maps/navigation (GPS) · phone companion app for logs · sunlight screen upgrade · knob display · more sensors.

## 7. Software
- **C + LVGL v9** on **ESP-IDF** (P4) · ESP-IDF/Arduino for audio ESP32.
- **Hardware abstraction layer**: UI reads a single `dash_data` struct; source = simulator, CAN replay, or real CAN. Same UI code on PC and device.
- **PC:** LVGL SDL simulator (Windows, VS Code) + **fake Spark ECU** (generates/replays CAN frames, keyboard/slider control of values & warnings).
- Only redraw changed regions (P4 ≈ 15–20 fps full-screen at this res).

## 8. Phases
0. Docs & setup (this) → 1. PC simulator + UI design (2–3 style options) → 2. Fake ECU + data layer → 3. Hardware bring-up (P4 + HDMI + inputs) → 4. CAN sniffing (bench ECU+stock gauge, then running engine) → 5. Real CAN integration + gauge emulation (iBR/modes/DESS) → 6. Audio board → 7. GPS, lights, weather, logging → 8. Boat install & polish.

## 9. Risks
- BRP CAN protocol undocumented; stock gauge must be temporarily reconnected to record. **Keep stock gauge + connector.**
- Removing gauge may affect DESS/iBR/start — must emulate gauge messages.
- 500-nit glossy screen hard to read in sun.
- HDMI monitor sync delay may push boot toward 3 s.
- IP65 controls not submersible.

## 10. Open Decisions
Visual style · 4 extra button functions · knob display use · wake signal · fuel sender wiring · dash opening size (boat in progress) · exact P4 board model.
