cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(esp32_test)

# Set custom partition table for testing
set(PARTITION_TABLE_CSV_PATH ${CMAKE_CURRENT_SOURCE_DIR}/partition_table.csv)
