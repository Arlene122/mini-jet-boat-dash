/**
 * ui — dash user interface entry point (same code on PC and ESP32-P4).
 */
#ifndef UI_H
#define UI_H

/* Screen size (Wisecoco 12.3" bar panel) */
#define UI_HOR_RES 1920
#define UI_VER_RES 720

/* Create screens and start the refresh timer. Call after lv_init() and
 * display creation. */
void ui_init(void);

#endif /* UI_H */
