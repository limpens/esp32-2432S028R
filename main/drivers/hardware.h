#pragma once

#define LCD_H_RES          240
#define LCD_V_RES          320
#define LCD_BITS_PIXEL     16
// Optimized buffer configuration based on LVGL 9.3 recommendations
// Use 1/4 screen size for better performance and memory usage
#define LCD_BUF_LINES      80  // Increased from 30 to 80 lines (1/4 screen)
#define LCD_DOUBLE_BUFFER  1   // Keep double buffering for smooth rendering
#define LCD_DRAWBUF_SIZE   (LCD_H_RES * LCD_BUF_LINES)  // 240 × 80 = 19,200 pixels

#ifdef CYD_ILI9341
#define LCD_MIRROR_X       (true)
#define LCD_MIRROR_Y       (false)
#else
#define LCD_MIRROR_X       (false)
#define LCD_MIRROR_Y       (false)
#endif

#define LCD_PIXEL_CLOCK_HZ (40 * 1000 * 1000)
#define LCD_CMD_BITS       (8)
#define LCD_PARAM_BITS     (8)
#define LCD_SPI_HOST       SPI2_HOST
#define LCD_SPI_CLK        (gpio_num_t) GPIO_NUM_14
#define LCD_SPI_MOSI       (gpio_num_t) GPIO_NUM_13
#define LCD_SPI_MISO       (gpio_num_t) GPIO_NUM_12
#define LCD_DC             (gpio_num_t) GPIO_NUM_2
#define LCD_CS             (gpio_num_t) GPIO_NUM_15
#define LCD_RESET          (gpio_num_t) GPIO_NUM_4
#define LCD_BUSY           (gpio_num_t) GPIO_NUM_NC

#define LCD_BACKLIGHT      (gpio_num_t) GPIO_NUM_21
#define LCD_BACKLIGHT_LEDC_CH  (1)

#define TOUCH_X_RES_MIN 200   // Reduced minimum for better edge coverage
#define TOUCH_X_RES_MAX 3900  // Increased maximum for better edge coverage  
#define TOUCH_Y_RES_MIN 280   // Reduced minimum for better edge coverage
#define TOUCH_Y_RES_MAX 3900  // Increased maximum for better edge coverage

#define TOUCH_CLOCK_HZ (2500000)  // 2.5MHz as recommended
#define TOUCH_SPI      SPI3_HOST
#define TOUCH_SPI_CLK  (gpio_num_t) GPIO_NUM_25
#define TOUCH_SPI_MOSI (gpio_num_t) GPIO_NUM_32
#define TOUCH_SPI_MISO (gpio_num_t) GPIO_NUM_39
#define TOUCH_CS       (gpio_num_t) GPIO_NUM_33
#define TOUCH_DC       (gpio_num_t) GPIO_NUM_NC
#define TOUCH_RST      (gpio_num_t) GPIO_NUM_NC
#define TOUCH_IRQ      (gpio_num_t) GPIO_NUM_NC /* GPIO_NUM_36, XPT driver is working better (for me) without IRQ */

#define TOUCH_MIRROR_X (true)  // Try without X mirroring for better corner coverage
#define TOUCH_MIRROR_Y (false)   // Keep Y mirroring
