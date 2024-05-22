#pragma once

#define CONFIG_LCD_RGB_ISR_IRAM_SAFE = true

/* LCD CONFIG */
// #define EXAMPLE_LCD_PIXEL_CLOCK_HZ   (6528000) //(10 * 1000 * 1000)
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ (2 * 1000 * 1000)

// The pixel number in horizontal and vertical
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 170
#define LCD_BUFFER_SIZE   SCREEN_WIDTH * 100
// #define EXAMPLE_PSRAM_DATA_ALIGNMENT 64
#define EXAMPLE_PSRAM_DATA_ALIGNMENT 32

/*ESP32S3*/
#define PIN_LCD_BL                   38

#define PIN_LCD_D0                   39
#define PIN_LCD_D1                   40
#define PIN_LCD_D2                   41
#define PIN_LCD_D3                   42
#define PIN_LCD_D4                   45
#define PIN_LCD_D5                   46
#define PIN_LCD_D6                   47
#define PIN_LCD_D7                   48

#define PIN_POWER_ON                 15

#define PIN_LCD_RES                  5
#define PIN_LCD_CS                   6
#define PIN_LCD_DC                   7
#define PIN_LCD_WR                   8
#define PIN_LCD_RD                   9

#define PIN_BUTTON_1                 0
#define PIN_BUTTON_2                 14
#define PIN_BAT_VOLT                 4

#define PIN_IIC_SCL                  17
#define PIN_IIC_SDA                  18

#define PIN_TOUCH_INT                16
#define PIN_TOUCH_RES                21