#pragma once

#define LCD_PIXEL_CLOCK_HZ (2 * 1000 * 1000)

#define LCD_PIN_DATA0 39
#define LCD_PIN_DATA1 40
#define LCD_PIN_DATA2 41
#define LCD_PIN_DATA3 42
#define LCD_PIN_DATA4 45
#define LCD_PIN_DATA5 46
#define LCD_PIN_DATA6 47
#define LCD_PIN_DATA7 48
#define LCD_PIN_PCLK 8
#define LCD_PIN_RD 9
#define LCD_PIN_CS 6
#define LCD_PIN_DC 7
#define LCD_PIN_RST 5
#define LCD_PIN_BK_LIGHT 38
#define LCD_PIN_POWER 15

#define PIN_BUTTON_1 0
#define PIN_BUTTON_2 14

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 170

#define LCD_BUFFER_SIZE   SCREEN_WIDTH * SCREEN_HEIGHT / 3

#define LCD_CMD_BITS 8
#define LCD_PARAM_BITS 8

#define LVGL_TICK_PERIOD_MS 2

// Supported alignment: 16, 32, 64. A higher alignment can enables higher burst transfer size, thus a higher i80 bus throughput.
#define PSRAM_DATA_ALIGNMENT 32

