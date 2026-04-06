# hal/src/esp32/CMakeLists.txt
# Сборка HAL для ESP32

file(GLOB_RECURSE ESP32_HAL_SOURCES *.c)

add_library(hal_esp32 STATIC ${ESP32_HAL_SOURCES})

target_include_directories(hal_esp32 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

# ESP-IDF подключается через IDF_PATH
if(DEFINED ENV{IDF_PATH})
    target_include_directories(hal_esp32 PUBLIC
        $ENV{IDF_PATH/components/driver/include
        $ENV{IDF_PATH/components/freertos/include
        $ENV{IDF_PATH/components/esp_timer/include
    )
endif()