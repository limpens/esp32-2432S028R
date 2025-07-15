#include <stdio.h>

// Mock GPIO implementation for host testing
typedef enum {
    ESP_OK = 0,
} esp_err_t;

typedef enum {
    GPIO_NUM_0 = 0,
    GPIO_NUM_2 = 2,
    GPIO_NUM_13 = 13,
    GPIO_NUM_14 = 14,
    GPIO_NUM_33 = 33,
    GPIO_NUM_36 = 36,
} gpio_num_t;

typedef enum {
    GPIO_MODE_DISABLE = 0,
    GPIO_MODE_INPUT,
    GPIO_MODE_OUTPUT,
} gpio_mode_t;

typedef enum {
    GPIO_PULLUP_DISABLE = 0,
    GPIO_PULLUP_ENABLE,
} gpio_pullup_t;

typedef enum {
    GPIO_PULLDOWN_DISABLE = 0,
    GPIO_PULLDOWN_ENABLE,
} gpio_pulldown_t;

typedef enum {
    GPIO_INTR_DISABLE = 0,
    GPIO_INTR_NEGEDGE,
} gpio_int_type_t;

typedef struct {
    uint64_t pin_bit_mask;
    gpio_mode_t mode;
    gpio_pullup_t pull_up_en;
    gpio_pulldown_t pull_down_en;
    gpio_int_type_t intr_type;
} gpio_config_t;

// Mock GPIO state
static int gpio_levels[40] = {0};

esp_err_t gpio_config(const gpio_config_t *pGPIOConfig) {
    // Mock implementation - just return success
    return ESP_OK;
}

esp_err_t gpio_set_level(gpio_num_t gpio_num, uint32_t level) {
    if (gpio_num < 40) {
        gpio_levels[gpio_num] = level;
    }
    return ESP_OK;
}

int gpio_get_level(gpio_num_t gpio_num) {
    if (gpio_num < 40) {
        return gpio_levels[gpio_num];
    }
    return 0;
}
