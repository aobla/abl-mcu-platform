# hal/CMakeLists.txt
# HAL contracts (L2) + port selection (L1).

# Contracts are header-only. Drivers depend on this component, never on a
# concrete port or vendor HAL (R4).
abl_component(hal INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/include)

# ─── Port selection ──────────────────────────────────────────────────────────
if(PLATFORM MATCHES "^stm32.*")
    set(HAL_PLATFORM "stm32")
elseif(PLATFORM STREQUAL "esp32")
    set(HAL_PLATFORM "esp32")
elseif(PLATFORM STREQUAL "avr")
    set(HAL_PLATFORM "avr")
elseif(PLATFORM STREQUAL "native")
    set(HAL_PLATFORM "native")
else()
    message(FATAL_ERROR "Unsupported HAL platform: ${PLATFORM}. Supported: stm32*, esp32, avr, native")
endif()

add_subdirectory(src/${HAL_PLATFORM})

# Alias for the active port (linked by add_firmware_target)
add_library(abl::active ALIAS abl_hal_${HAL_PLATFORM})
