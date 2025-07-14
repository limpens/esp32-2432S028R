#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include <esp_log.h>
#include <esp_err.h>
#include <esp_timer.h>
#include <esp_lcd_touch.h>
#include <esp_lcd_touch_xpt2046.h>

#include <driver/spi_master.h>
#include <driver/gpio.h>

#include "hardware.h"
#include "touch.h"

static const char *TAG = "touch";

esp_err_t touch_init(esp_lcd_touch_handle_t *tp)
{
    ESP_LOGI(TAG, "Initializing touch controller...");
    
    ESP_LOGI(TAG, "Touch config: X_MIN=%d, X_MAX=%d, Y_MIN=%d, Y_MAX=%d", 
             TOUCH_X_RES_MIN, TOUCH_X_RES_MAX, TOUCH_Y_RES_MIN, TOUCH_Y_RES_MAX);
    ESP_LOGI(TAG, "LCD config: %dx%d (landscape: %dx%d)", LCD_H_RES, LCD_V_RES, LCD_V_RES, LCD_H_RES);
    
    esp_lcd_panel_io_handle_t tp_io_handle = NULL;

    const esp_lcd_panel_io_spi_config_t tp_io_config = { 
        .cs_gpio_num = TOUCH_CS,
        .dc_gpio_num = TOUCH_DC,
        .spi_mode = 0,
        .pclk_hz = TOUCH_CLOCK_HZ,
        .trans_queue_depth = 3,
        .on_color_trans_done = NULL,
        .user_ctx = NULL,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .flags = { 
            .dc_low_on_data = 0, 
            .octal_mode = 0, 
            .sio_mode = 0, 
            .lsb_first = 0, 
            .cs_high_active = 0 
        } 
    };

    static const int SPI_MAX_TRANSFER_SIZE = 32768;
    const spi_bus_config_t buscfg_touch = { 
        .mosi_io_num = TOUCH_SPI_MOSI,
        .miso_io_num = TOUCH_SPI_MISO,
        .sclk_io_num = TOUCH_SPI_CLK,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .data4_io_num = GPIO_NUM_NC,
        .data5_io_num = GPIO_NUM_NC,
        .data6_io_num = GPIO_NUM_NC,
        .data7_io_num = GPIO_NUM_NC,
        .max_transfer_sz = SPI_MAX_TRANSFER_SIZE,
        .flags = SPICOMMON_BUSFLAG_SCLK | SPICOMMON_BUSFLAG_MISO | SPICOMMON_BUSFLAG_MOSI | SPICOMMON_BUSFLAG_MASTER | SPICOMMON_BUSFLAG_GPIO_PINS,
        .isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO,
        .intr_flags = ESP_INTR_FLAG_LOWMED | ESP_INTR_FLAG_IRAM 
    };

    esp_lcd_touch_config_t tp_cfg = {
        .x_max = LCD_H_RES,  // For landscape mode: touch X maps to display height (240)
        .y_max = LCD_V_RES,  // For landscape mode: touch Y maps to display width (320)
        .rst_gpio_num = TOUCH_RST,
        .int_gpio_num = TOUCH_IRQ,
        .levels = {.reset = 0, .interrupt = 0},
        .flags = {
            .swap_xy = false,       // SWAP X/Y for landscape mode (90° rotation)
            .mirror_x = TOUCH_MIRROR_X,  // Mirror X axis
            .mirror_y = TOUCH_MIRROR_Y   // Mirror Y axis
        },
        .process_coordinates = NULL,  // Use default coordinate processing
        .interrupt_callback = NULL
    };

    ESP_LOGI(TAG, "Touch configuration for landscape mode (90° rotation):");
    ESP_LOGI(TAG, "  - Display size: %dx%d (width x height)", LCD_V_RES, LCD_H_RES);
    ESP_LOGI(TAG, "  - Touch mapping: x_max=%d, y_max=%d", LCD_H_RES, LCD_V_RES);
    ESP_LOGI(TAG, "  - Touch range: X=%d-%d, Y=%d-%d", 
             TOUCH_X_RES_MIN, TOUCH_X_RES_MAX, TOUCH_Y_RES_MIN, TOUCH_Y_RES_MAX);
    ESP_LOGI(TAG, "  - Flags: swap_xy=%s, mirror_x=%s, mirror_y=%s", 
             tp_cfg.flags.swap_xy ? "true" : "false",
             tp_cfg.flags.mirror_x ? "true" : "false", 
             tp_cfg.flags.mirror_y ? "true" : "false");

    ESP_LOGI(TAG, "Touch GPIO config: CLK=%d, MOSI=%d, MISO=%d, CS=%d", 
             TOUCH_SPI_CLK, TOUCH_SPI_MOSI, TOUCH_SPI_MISO, TOUCH_CS);

    ESP_ERROR_CHECK(spi_bus_initialize(TOUCH_SPI, &buscfg_touch, SPI_DMA_CH_AUTO));
    ESP_LOGI(TAG, "Touch SPI bus initialized");

    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)TOUCH_SPI, &tp_io_config, &tp_io_handle));
    ESP_LOGI(TAG, "Touch panel IO created");

    ESP_ERROR_CHECK(esp_lcd_touch_new_spi_xpt2046(tp_io_handle, &tp_cfg, tp));
    ESP_LOGI(TAG, "XPT2046 touch controller initialized successfully");

    return ESP_OK;
}
