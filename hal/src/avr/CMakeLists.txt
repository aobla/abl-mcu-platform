# hal/src/avr/CMakeLists.txt
# Сборка HAL для AVR

file(GLOB_RECURSE AVR_HAL_SOURCES *.c)

add_library(hal_avr STATIC ${AVR_HAL_SOURCES})

target_include_directories(hal_avr PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

# AVR libc доступна через тулчейн автоматически