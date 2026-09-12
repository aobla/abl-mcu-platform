# CMake generated Testfile for 
# Source directory: /home/abl/projects/dsh/abl-mcu-platform/abl-mcu-platform/tests
# Build directory: /home/abl/projects/dsh/abl-mcu-platform/abl-mcu-platform/build-tests/tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[hal_gpio_test]=] "/home/abl/projects/dsh/abl-mcu-platform/abl-mcu-platform/build-tests/tests/hal_gpio_test")
set_tests_properties([=[hal_gpio_test]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/abl/projects/dsh/abl-mcu-platform/abl-mcu-platform/tests/CMakeLists.txt;14;add_test;/home/abl/projects/dsh/abl-mcu-platform/abl-mcu-platform/tests/CMakeLists.txt;0;")
add_test([=[runtime_test]=] "/home/abl/projects/dsh/abl-mcu-platform/abl-mcu-platform/build-tests/tests/runtime_test")
set_tests_properties([=[runtime_test]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/abl/projects/dsh/abl-mcu-platform/abl-mcu-platform/tests/CMakeLists.txt;14;add_test;/home/abl/projects/dsh/abl-mcu-platform/abl-mcu-platform/tests/CMakeLists.txt;0;")
add_test([=[codegen_test]=] "python3" "/home/abl/projects/dsh/abl-mcu-platform/abl-mcu-platform/tests/codegen_test.py" "/home/abl/projects/dsh/abl-mcu-platform/abl-mcu-platform/tests/../")
set_tests_properties([=[codegen_test]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/abl/projects/dsh/abl-mcu-platform/abl-mcu-platform/tests/CMakeLists.txt;18;add_test;/home/abl/projects/dsh/abl-mcu-platform/abl-mcu-platform/tests/CMakeLists.txt;0;")
