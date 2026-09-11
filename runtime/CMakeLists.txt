# runtime/CMakeLists.txt
# Runtime contract (D6) + backend selection.
#
# The backend is chosen by the application: `product.runtime: bare | freertos`
# in the app config; build.sh passes it as -DABL_RUNTIME=...

if(NOT DEFINED ABL_RUNTIME OR ABL_RUNTIME STREQUAL "")
    set(ABL_RUNTIME "bare")
endif()

if(ABL_RUNTIME STREQUAL "freertos")
    message(FATAL_ERROR
        "runtime: the 'freertos' backend is not wired yet (planned: ARCHITECTURE §18 step 9). "
        "Use `product.runtime: bare` for now.")
elseif(NOT ABL_RUNTIME STREQUAL "bare")
    message(FATAL_ERROR "runtime: unknown backend '${ABL_RUNTIME}' (supported: bare)")
endif()

# Include-wise the backend only needs the contracts (abl::hal), but it is built
# on top of the platform HAL primitives (abl_delay_*, abl_time_uptime_ms), which
# live in the active port. Declaring the port as a dependency also gives CMake
# the link order it needs for static libraries.
abl_component(runtime
    SOURCES  src/${ABL_RUNTIME}/runtime_${ABL_RUNTIME}.c
    INCLUDES include
    DEPS     abl::hal abl::active
)
