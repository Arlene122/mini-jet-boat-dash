# Claude Rules — Mini Jet Boat Dash

## Workflow
1. **Read `Claude_Memory.md` first** every new chat, then `Requirements.md` as needed.
2. Message starts with **`Chat:`** (any capitalisation) → **planning mode**: discuss/plan only. **Do not write, change, commit or push any code or files.**
3. Before starting any task: ask questions until **≥97% sure** of what to do.
4. **Be token-efficient**: short answers, no repeated content, edit only what changes.
5. Update `Claude_Memory.md` after every meaningful decision, fix or lesson. Keep it compact + indexed.
6. **Only update `Requirements.md` when the owner explicitly says so.** Record proposals/decisions in `Claude_Memory.md` instead.

## Coding
- User has **no coding experience**: give exact, step-by-step Windows instructions (what to click/type, where files go).
- Language: **C, LVGL v9, ESP-IDF**. PC build via LVGL SDL simulator.
- UI never reads hardware directly — only the `dash_data` layer (sim / replay / real CAN).
- Every feature must run in the **PC simulator first**, then hardware.
- Keep files small and modular (one screen/widget per file). Clear section headers.
- Only redraw what changes (P4 performance). Avoid full-screen animations during riding.
- Design for 1920×720; keep content inside the stencil visible area.

## Safety
- **Never transmit on the real CAN bus** until the message is decoded and verified in the simulator/replay.
- Mark any CAN ID/byte meaning as `UNVERIFIED` until confirmed from real logs.
- Never guess BRP protocol details as fact.
- Warnings must always show on screen even if audio fails.

## Don'ts
- No HTML/web UI (too laggy — tried and abandoned).
- No Raspberry Pi/Linux (boot too slow).
- No touch-based UI.
