/**
 * ui_layout — screen zones fitted to the bezel stencil (assets/stencil.png).
 * Change these if the stencil changes; widgets position from them.
 *
 *   ____________        [ trip | clock | hours ]        ____________
 *               \______________________________________/     <- frame line
 *   engine panel |   speed gauge + RPM inner arc   | page card
 *    ___________________________________________________
 *   ____________/  [ mode | warnings | DESS ]  \_____________ <- frame line
 */
#ifndef UI_LAYOUT_H
#define UI_LAYOUT_H

#include "ui.h"

/* Frame lines: run across at Y_OUT, dip ("pinch") to Y_IN between the
 * notch x range; the bars sit inside the notch. */
#define FRAME_TOP_OUT  58
#define FRAME_TOP_IN   116
#define FRAME_BOT_OUT  662
#define FRAME_BOT_IN   604
#define NOTCH_X1       450
#define NOTCH_X2       1470
#define NOTCH_CURVE    100          /* width of each S-curve */

/* Bars inside the notch, narrowing slightly toward the screen centre */
#define BAR_X          (NOTCH_X1 + 20)
#define BAR_W          (NOTCH_X2 - NOTCH_X1 - 40)
#define BAR_H          52
#define BAR_TILT       18           /* inset of the inner (narrow) edge */
#define TOP_BAR_Y      (FRAME_TOP_IN - BAR_H - 8)
#define BOT_BAR_Y      (FRAME_BOT_IN + 8)

/* Centre gauge (between the dipped lines) */
#define GAUGE_CX       (UI_HOR_RES / 2)
#define GAUGE_CY       ((FRAME_TOP_IN + FRAME_BOT_IN) / 2)
#define GAUGE_R        232

/* Side zones */
#define SIDE_Y1        (FRAME_TOP_IN + 16)
#define SIDE_Y2        (FRAME_BOT_IN - 16)
#define SIDE_H         (SIDE_Y2 - SIDE_Y1)
#define ENGINE_X       225
#define ENGINE_W       450
#define PAGE_X         1250
#define PAGE_W         480

#endif /* UI_LAYOUT_H */
