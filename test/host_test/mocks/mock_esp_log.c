#include <stdio.h>
#include <stdarg.h>
#include <time.h>

// Mock ESP_LOG implementation for host testing
typedef enum {
    ESP_LOG_NONE,       
    ESP_LOG_ERROR,      
    ESP_LOG_WARN,       
    ESP_LOG_INFO,       
    ESP_LOG_DEBUG,      
    ESP_LOG_VERBOSE     
} esp_log_level_t;

int esp_log_level_get(const char* tag) {
    return ESP_LOG_INFO;
}

void esp_log_level_set(const char* tag, esp_log_level_t level) {
    // Mock implementation
}

void esp_log_write(esp_log_level_t level, const char* tag, const char* format, ...) {
    const char* level_str[] = {"NONE", "ERROR", "WARN", "INFO", "DEBUG", "VERBOSE"};
    
    printf("[%s] %s: ", level_str[level], tag);
    
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    
    printf("\n");
}

// Mock heap capabilities
size_t heap_caps_get_free_size(uint32_t caps) {
    return 100000; // Mock free heap size
}

// Mock system time
int64_t esp_timer_get_time(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * 1000000LL + ts.tv_nsec / 1000;
}
