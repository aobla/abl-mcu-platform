# runtime/CMakeLists.txt
# Runtime contract (D6) + backend selection.
#
# The backend is chosen by the application: `product.runtime: bare | freertos`
# in the app config; build.sh passes it as -DABL_RUNTIME=...
#
# Note: the ESP-IDF build (Step 9) compiles the backend source directly from the
# project wrapper, because the IDF component model provides FreeRTOS itself.

if(NOT DEFINED ABL_RUNTIME OR ABL_RUNTIME STREQUAL "")
    set(ABL_RUNTIME "bare")
endif()

if(ABL_RUNTIME STREQUAL "bare")
    # Include-wise the backend only needs the contracts (abl::hal), but it is
    # built on top of the platform HAL primitives (abl_delay_*, abl_time_*),
    # which live in the active port. Declaring the port as a dependency also
    # gives CMake the link order it needs for static libraries.
    abl_component(runtime
        SOURCES  src/bare/runtime_bare.c
        INCLUDES include
        DEPS     abl::hal abl::active
    )
elseif(ABL_RUNTIME STREQUAL "freertos")
    # FreeRTOS sources/headers must come from the platform (ESP-IDF provides
    # them; STM32 + FreeRTOS is a later step).
    abl_component(runtime
        SOURCES  src/freertos/runtime_freertos.c
        INCLUDES include
        DEPS     abl::hal abl::active
    )
else()
    message(FATAL_ERROR "runtime: unknown backend '${ABL_RUNTIME}' (supported: bare, freertos)")
endif()
