#pragma once

#define LCD_PIXEL_CLOCK_HZ (2 * 1000 * 1000)

#define LCD_BK_LIGHT_ON_LEVEL 1
#define LCD_BK_LIGHT_OFF_LEVEL !LCD_BK_LIGHT_ON_LEVEL
#define LCD_PIN_DATA0 39    // 6
#define LCD_PIN_DATA1 40    // 7
#define LCD_PIN_DATA2 41    //O 8
#define LCD_PIN_DATA3 42    // 9
#define LCD_PIN_DATA4 45    // 10
#define LCD_PIN_DATA5 46    // 11
#define LCD_PIN_DATA6 47    // 12
#define LCD_PIN_DATA7 48    // 13
#define LCD_PIN_PCLK 8      // 5
#define LCD_PIN_RD 9      
#define LCD_PIN_CS 6        // 3
#define LCD_PIN_DC 7        // 4
#define LCD_PIN_RST 5       // 2
#define LCD_PIN_BK_LIGHT 38 // 1
#define LCD_PIN_POWER 15

#define PIN_BUTTON_1                 0
#define PIN_BUTTON_2                 14

// The pixel number in horizontal and vertical
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 170

#define LCD_BUFFER_SIZE   SCREEN_WIDTH * (SCREEN_HEIGHT/2)

// Bit number used to represent command and parameter
#define LCD_CMD_BITS 8
#define LCD_PARAM_BITS 8

// #define EXAMPLE_LVGL_TICK_PERIOD_MS 2

// Supported alignment: 16, 32, 64. A higher alignment can enables higher burst transfer size, thus a higher i80 bus throughput.
#define PSRAM_DATA_ALIGNMENT 32

