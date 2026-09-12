#!/usr/bin/env python3
"""
Single code-generation entry point used by both build systems (the plain CMake
build and the ESP-IDF wrapper):

    generate.py <board.yml> <app.yml>

Runs every generator in this directory and writes into ./generated.
Templates live in ../templates and are the single source of truth for generated
code (ARCHITECTURE.md §9): add a generator function here and a template there.
"""

import sys

import gen_config
import gen_pinmux


def main():
    if len(sys.argv) != 3:
        sys.exit("Usage: generate.py <board.yml> <app.yml>")

    board_path, app_path = sys.argv[1], sys.argv[2]
    gen_config.generate(board_path, app_path)
    gen_pinmux.generate(board_path, app_path)


if __name__ == "__main__":
    main()
