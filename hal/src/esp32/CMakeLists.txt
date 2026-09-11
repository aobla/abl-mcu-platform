# hal/src/esp32/CMakeLists.txt
# ESP32 HAL port (L1). TODO (Step 9): moves to the ESP-IDF wrapper.

file(GLOB_RECURSE HAL_PORT_SOURCES *.c)

set(HAL_IDF_INCLUDES "")
if(DEFINED ENV{IDF_PATH})
    list(APPEND HAL_IDF_INCLUDES
        $ENV{IDF_PATH}/components/driver/include
        $ENV{IDF_PATH}/components/freertos/include
        $ENV{IDF_PATH}/components/esp_timer/include
    )
endif()

abl_component(hal_esp32
    SOURCES  ${HAL_PORT_SOURCES}
    INCLUDES ${HAL_IDF_INCLUDES}
    DEPS     abl::hal
)
