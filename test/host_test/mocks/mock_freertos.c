#include <stdio.h>
#include <unistd.h>

// Mock FreeRTOS implementation for host testing
typedef void* TaskHandle_t;
typedef uint32_t TickType_t;

#define portTICK_PERIOD_MS 1
#define pdMS_TO_TICKS(ms) (ms)

void vTaskDelay(TickType_t xTicksToDelay) {
    usleep(xTicksToDelay * 1000); // Convert to microseconds
}

void vTaskDelete(TaskHandle_t xTaskToDelete) {
    // Mock implementation
}

// Mock task creation
typedef void (*TaskFunction_t)(void *);

int xTaskCreate(TaskFunction_t pxTaskCode,
                const char * const pcName,
                const uint16_t usStackDepth,
                void * const pvParameters,
                unsigned int uxPriority,
                TaskHandle_t * const pxCreatedTask) {
    // Mock implementation - don't actually create task for host tests
    return 1; // pdPASS
}
