# Mini Jet Boat Dash

Custom 1920×720 digital dash for a mini jet boat with a 2015 Sea-Doo Spark engine.
ESP32-P4 + LVGL v9 (C). See `Requirements.md`, `Claude_Rules.md`, `Claude_Memory.md`.

## Try the simulator (no installs)

**https://arlene122.github.io/mini-jet-boat-dash/**

Click the dash, then use the keys listed under it (throttle, fuel, iBR, modes,
DESS, warnings). Press **H** for on-screen help, **S** to cycle the bezel stencil.

The browser page is only a host: the dash itself is the same C/LVGL code that
will run on the ESP32-P4, compiled to WebAssembly by GitHub Actions.

## Stencil image

Put the real bezel stencil at `assets/stencil.png` (1920×720; transparent or
white = visible screen, dark = covered by bezel). Until then
`assets/stencil_placeholder.png` is used.

## Layout

| Path | What |
|---|---|
| `src/dash_data/` | The only data the UI reads (sim / replay / real CAN fill it) |
| `src/ui/` | Screens and widgets (shared by PC and boat) |
| `src/ui/fonts/` | Large digit fonts (Montserrat, OFL licence) |
| `sim/` | Simulator: SDL main, fake Spark ECU, stencil/help overlay, `lv_conf.h` |
| `sim/web/shell.html` | Browser page around the WebAssembly build |
| `tools/stencil_to_c.py` | Embeds the stencil image at build time |
| `assets/` | Images |
| `.github/workflows/web-sim.yml` | Build to WebAssembly + publish to GitHub Pages |
