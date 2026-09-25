# Claude Memory — Mini Jet Boat Dash
_Last updated: 2026-09-25 · Keep compact. Update often._

## Index
1. Status · 2. Key Decisions · 3. Hardware · 4. Lessons / Avoid · 5. Open Items · 6. Next Step · 7. Change Log

## 1. Status
Phase 0 (planning) done. Requirements + Rules written. No code yet.

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

## 5. Open Items
Visual style (give 2–3 options) · 4 button functions · knob display · wake signal · fuel sender wiring · dash opening size · P4 board model.

## 6. Next Step
Phase 1: set up Windows dev env + LVGL PC simulator; propose 2–3 UI style concepts at 1920×720 within stencil.

## 7. Change Log
- 2026-09-25: Planning complete, docs created.
