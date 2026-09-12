# tests/CMakeLists.txt
# Host tests (D8). Configured only when PLATFORM=native and ABL_BUILD_TESTS=ON.
#
# The tests link the HAL/runtime WITHOUT the soc component: native_main.c owns
# main() and every test binary provides its own.

add_library(abl_test_main INTERFACE)
target_include_directories(abl_test_main INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# ─── C tests ─────────────────────────────────────────────────────────────────
foreach(_test hal_gpio_test runtime_test)
    add_executable(${_test} ${_test}.c)
    target_link_libraries(${_test} PRIVATE abl_test_main abl::hal abl::active abl::runtime)
    add_test(NAME ${_test} COMMAND ${_test})
endforeach()

# ─── Code generation contract tests (Python) ─────────────────────────────────
add_test(
    NAME codegen_test
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/codegen_test.py ${CMAKE_CURRENT_SOURCE_DIR}/../
)
