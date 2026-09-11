# hal/src/avr/CMakeLists.txt
# AVR HAL port (L1). TODO (Step 8): the AVR port is not buildable yet.

file(GLOB_RECURSE HAL_PORT_SOURCES *.c)

abl_component(hal_avr
    SOURCES ${HAL_PORT_SOURCES}
    DEPS    abl::hal
)
