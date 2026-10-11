#pragma once

typedef enum {
    OLED_FONT_SMALL,
    OLED_FONT_MEDIUM,
    OLED_FONT_LARGE
} oled_font_t;

typedef enum {
    OLED_MESSAGE_NONE,
    OLED_MESSAGE_AUTOPILOT_BUSY,
    OLED_MESSAGE_BUTTON_NOT_CONFIRMED,
    OLED_MESSAGE_SPEED_TOO_LOW,
    OLED_MESSAGE_BRAKE_ACTIVE,
    OLED_MESSAGE_CLUTCH_ACTIVE,
} oled_message_t;

void oled_show_message(oled_message_t message);
void oled_show_motor_calibrating(bool show);
void oled_show_motor_emergency_releasing(bool show);
void oled_init(void);