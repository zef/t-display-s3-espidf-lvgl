#pragma once

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

// I've seen some different recommendations for buffer sizes.
// I keep getting artifacts (like elements from a previous run) when I try to do less than the full screen.
// But doing it in one go seems to be working ok, but I might experiment with other values in the future.
#define LCD_BUFFER_SIZE   SCREEN_WIDTH * SCREEN_HEIGHT

#define LVGL_TICK_PERIOD_MS 2

#define LCD_PIXEL_CLOCK_HZ (2 * 1000 * 1000)

#define LCD_CMD_BITS 8
#define LCD_PARAM_BITS 8

// Supported alignment: 16, 32, 64. A higher alignment can enables higher burst transfer size, thus a higher i80 bus throughput.
#define PSRAM_DATA_ALIGNMENT 32
