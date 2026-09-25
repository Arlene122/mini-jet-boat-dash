/**
 * ui_layout — screen zones fitted to the bezel stencil (assets/stencil.png).
 * Change these if the stencil changes; widgets position from them.
 *
 *        /---------- top bar: trip | clock | hours ----------\
 *   < engine panel |   speed gauge + RPM inner arc   | page card |
 *        \------- bottom bar: mode | warnings | DESS -------/
 */
#ifndef UI_LAYOUT_H
#define UI_LAYOUT_H

#include "ui.h"

/* Tilted bars (follow the stencil's slanted edges) */
#define TOP_BAR_Y      50
#define TOP_BAR_H      62
#define TOP_BAR_X      205          /* wide (bottom) edge */
#define TOP_BAR_W      1620
#define TOP_BAR_IN_L   57           /* slant insets at the narrow edge */
#define TOP_BAR_IN_R   47

#define BOT_BAR_Y      608
#define BOT_BAR_H      62
#define BOT_BAR_X      197          /* wide (top) edge */
#define BOT_BAR_W      1533
#define BOT_BAR_IN_L   50
#define BOT_BAR_IN_R   37

/* Centre gauge */
#define GAUGE_CX       (UI_HOR_RES / 2)
#define GAUGE_CY       ((TOP_BAR_Y + TOP_BAR_H + BOT_BAR_Y) / 2)
#define GAUGE_R        246

/* Side zones */
#define SIDE_Y1        (TOP_BAR_Y + TOP_BAR_H + 30)
#define SIDE_Y2        (BOT_BAR_Y - 24)
#define SIDE_H         (SIDE_Y2 - SIDE_Y1)
#define ENGINE_X       225
#define ENGINE_W       450
#define PAGE_X         1250
#define PAGE_W         480

#endif /* UI_LAYOUT_H */
