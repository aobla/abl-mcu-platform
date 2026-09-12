#!/bin/bash
# Host tests (native + codegen) — ARCHITECTURE §16.
#
#   ./scripts/run_host_tests.sh
#
# Configures the platform standalone for the native platform, builds the test
# binaries and runs ctest. No hardware and no cross toolchain required.

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PLATFORM_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${PLATFORM_DIR}/build-tests"

cmake -S "${PLATFORM_DIR}" -B "${BUILD_DIR}" \
    -DPLATFORM=native \
    -DABL_BUILD_TESTS=ON \
    -DCMAKE_BUILD_TYPE=Debug \
    -G Ninja

cmake --build "${BUILD_DIR}"
ctest --test-dir "${BUILD_DIR}" --output-on-failure
