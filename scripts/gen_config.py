#!/usr/bin/env python3
"""
hardware_config.h generator (D5/D9).

    gen_config.py <board.yml> <app.yml>

  board — cpu, frequencies, memory
  app   — features, params

Output: ./generated/hardware_config.h

The template lives in templates/hardware_config.h.jinja (single source, §9).
"""

import sys

from abl_codegen import emit, load_template, load_yaml, macro, report_errors


def validate(board, app):
    errors = []
    if "mcu" not in board:
        errors.append("board: missing 'mcu' (mcu.part is required)")
    elif not isinstance(board["mcu"], dict) or "part" not in board["mcu"]:
        errors.append("board: missing 'mcu.part'")
    for key in ("frequencies", "memory"):
        if key in board and not isinstance(board[key], dict):
            errors.append(f"board: '{key}' must be a mapping")
    for key in ("features", "params"):
        if key in app and not isinstance(app[key], dict):
            errors.append(f"app: '{key}' must be a mapping")
    return errors


def generate(board_path, app_path):
    """Generate hardware_config.h and return the written path."""
    board = load_yaml(board_path, "board")
    app = load_yaml(app_path, "app")

    report_errors("Config validation errors", validate(board, app))

    content = load_template("hardware_config.h.jinja").render(
        cpu=macro(board["cpu"]) if board.get("cpu") else "",
        frequencies={macro(k): v for k, v in (board.get("frequencies") or {}).items()},
        memory={macro(k): v for k, v in (board.get("memory") or {}).items()},
        features={macro(k): v for k, v in (app.get("features") or {}).items()},
        params={macro(k): v for k, v in (app.get("params") or {}).items()},
    )
    return emit("hardware_config.h", content)


def main():
    if len(sys.argv) != 3:
        sys.exit("Usage: gen_config.py <board.yml> <app.yml>")
    generate(sys.argv[1], sys.argv[2])


if __name__ == "__main__":
    main()
