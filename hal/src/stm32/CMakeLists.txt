# hal/src/stm32/CMakeLists.txt
# STM32 HAL port (L1): abl_gpio/abl_delay implementations + the vendor HAL sources.
# Vendor context (device define, hal_conf, SDK includes) comes from abl::soc.

file(GLOB_RECURSE HAL_PORT_SOURCES *.c)

if(NOT DEFINED SOC_HAL_PREFIX OR SOC_HAL_PREFIX STREQUAL "")
    message(FATAL_ERROR "hal_stm32: SOC_HAL_PREFIX is not set — no SoC definition for mcu.part=${MCU_PART}")
endif()

set(HAL_VENDOR_SOURCES "")
if(DEFINED STM32_SDK_ROOT AND IS_DIRECTORY "${STM32_SDK_ROOT}/Src")
    # Minimal set: core + GPIO + EXTI + RCC + Cortex
    foreach(_name
            ${SOC_HAL_PREFIX}_hal
            ${SOC_HAL_PREFIX}_hal_gpio
            ${SOC_HAL_PREFIX}_hal_exti
            ${SOC_HAL_PREFIX}_hal_rcc
            ${SOC_HAL_PREFIX}_hal_rcc_ex
            ${SOC_HAL_PREFIX}_hal_cortex)
        if(EXISTS "${STM32_SDK_ROOT}/Src/${_name}.c")
            list(APPEND HAL_VENDOR_SOURCES "${STM32_SDK_ROOT}/Src/${_name}.c")
        endif()
    endforeach()
endif()

abl_component(hal_stm32
    SOURCES ${HAL_PORT_SOURCES} ${HAL_VENDOR_SOURCES}
    DEPS    abl::hal abl::soc
)
