/**
 * ui_input — physical controls as abstract events.
 * Boat: SLD knob + SF39BA 5-way + buttons. Simulator: keyboard.
 */
#ifndef UI_INPUT_H
#define UI_INPUT_H

typedef enum {
    UI_IN_NEXT = 0,   /* knob clockwise  -> next page */
    UI_IN_PREV,       /* knob anticlockwise -> previous page */
    UI_IN_SELECT,     /* knob push / 5-way centre */
    UI_IN_HOME,       /* knob long press */
    UI_IN_UP,         /* 5-way up */
    UI_IN_DOWN,       /* 5-way down */
} ui_input_t;

/* Feed one control event into the UI. */
void ui_input(ui_input_t in);

#endif /* UI_INPUT_H */
