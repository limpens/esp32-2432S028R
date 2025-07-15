cmake_minimum_required(VERSION 3.16)

project(esp32_host_tests)

# Find required packages
find_package(PkgConfig REQUIRED)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../main
    ${CMAKE_CURRENT_SOURCE_DIR}/../../main/drivers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../main/frontend
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks
)

# Mock implementations
set(MOCK_SOURCES
    mocks/mock_esp_log.c
    mocks/mock_nvs.c
    mocks/mock_gpio.c
    mocks/mock_freertos.c
)

# Test sources
set(TEST_SOURCES
    test_main.c
    test_utilities.c
    test_validation.c
)

# Create test executable
add_executable(host_tests ${TEST_SOURCES} ${MOCK_SOURCES})

# Compiler flags
target_compile_options(host_tests PRIVATE -Wall -Wextra -g)

# Define macros to simulate ESP-IDF environment
target_compile_definitions(host_tests PRIVATE
    HOST_TEST=1
    ESP_PLATFORM=1
    CONFIG_LOG_DEFAULT_LEVEL=3
)

# Enable testing
enable_testing()
add_test(NAME host_tests COMMAND host_tests)
