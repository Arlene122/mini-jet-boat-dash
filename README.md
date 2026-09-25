# Mini Jet Boat Dash

Custom 1920×720 digital dash for a mini jet boat with a 2015 Sea-Doo Spark engine.
ESP32-P4 + LVGL v9 (C). See `Requirements.md`, `Claude_Rules.md`, `Claude_Memory.md`.

## Try the simulator (no installs)

**https://arlene122.github.io/mini-jet-boat-dash/**

Click the dash, then use the keys listed under it. **← →** = knob (pages),
**Enter** = knob push, **↑ ↓** = 5-way, **W/S** = throttle, **M** = mode,
**1–5** = warnings. **H** = on-screen help, **T** = bezel stencil.

The browser page is only a host: the dash itself is the same C/LVGL code that
will run on the ESP32-P4, compiled to WebAssembly by GitHub Actions.

## Stencil image

`assets/stencil.png` is traced from the owner's outline drawing by
`tools/trace_stencil.py` (edit the corner points there to adjust).
Transparent = visible screen, dark = bezel.

## Layout

| Path | What |
|---|---|
| `src/dash_data/` | The only data the UI reads (sim / replay / real CAN fill it) |
| `src/dash_data/dash_cmd.h` | UI → boat actions (lights, music) |
| `src/ui/` | Screens and widgets (shared by PC and boat); zones in `ui_layout.h` |
| `src/ui/pages/` | Page card pages: Marine, Music, Lights, System, Trip |
| `src/ui/fonts/` | Large digit fonts (Montserrat, OFL licence) |
| `sim/` | Simulator: SDL main, fake Spark ECU, stencil/help overlay, `lv_conf.h` |
| `sim/web/shell.html` | Browser page around the WebAssembly build |
| `tools/stencil_to_c.py` | Embeds the stencil image at build time |
| `assets/` | Images |
| `.github/workflows/web-sim.yml` | Build to WebAssembly + publish to GitHub Pages |
