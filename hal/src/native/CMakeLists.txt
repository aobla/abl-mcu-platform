# hal/src/native/CMakeLists.txt
# Native (host) HAL port (L1) — simulation backend for tests and development.

file(GLOB_RECURSE HAL_PORT_SOURCES *.c)

# The simulation hooks header is exported so that host tests can drive
# simulated inputs (see hal/src/native/include/abl_native_sim.h).
abl_component(hal_native
    SOURCES  ${HAL_PORT_SOURCES}
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/include
    DEPS     abl::hal
)
