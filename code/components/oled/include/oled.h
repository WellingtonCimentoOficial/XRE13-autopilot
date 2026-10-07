#pragma once

typedef enum {
    OLED_FONT_SMALL,
    OLED_FONT_MEDIUM,
    OLED_FONT_LARGE
} oled_font_t;

typedef enum {
    OLED_ERROR_NONE,
    OLED_ERROR_NOT_READY,
    OLED_ERROR_BUTTON_NOT_CONFIRMED,
    OLED_ERROR_SPEED_TOO_LOW,
    OLED_ERROR_BRAKE_ACTIVE,
    OLED_ERROR_CLUTCH_ACTIVE,
} oled_error_t;

void oled_show_error(oled_error_t error);
void oled_show_motor_calibrating(bool show);
void oled_print(char *text, int line, oled_font_t font);
void oled_init(void);