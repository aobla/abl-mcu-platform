#!/usr/bin/env python3
"""
Генерация hardware_config.h из двух входов (D5/D9).

    gen_config.py <board.yml> <app.yml>

  board.yml — физика платы: cpu, frequencies, memory
  app.yml   — продукт: features, params

Выход: ./generated/hardware_config.h (относительно текущего каталога).
"""

import os
import sys
from pathlib import Path

import yaml
from jinja2 import Template

TEMPLATE = """#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

/* Сгенерировано из board + app конфигов — не редактировать вручную. */

/* ─── Плата (board) ─────────────────────────────────────────────────────── */
{% if cpu %}
#ifndef CPU_{{ cpu }}
#define CPU_{{ cpu }}
#endif
{% endif %}
{% for name, value in frequencies.items() %}
#ifndef {{ name }}_FREQ
#define {{ name }}_FREQ {{ value }}
#endif
{% endfor %}
{% for name, value in memory.items() %}
#ifndef {{ name }}_SIZE
#define {{ name }}_SIZE {{ value }}
#endif
{% endfor %}

/* ─── Продукт (app): фичи (0/1) ─────────────────────────────────────────── */
{% for name, enabled in features.items() %}
#ifndef CONFIG_{{ name }}
#define CONFIG_{{ name }} {% if enabled %}1{% else %}0{% endif %}
#endif
{% endfor %}

/* ─── Продукт (app): параметры ──────────────────────────────────────────── */
{% for name, value in params.items() %}
#ifndef CONFIG_{{ name }}
#define CONFIG_{{ name }} {{ value }}
#endif
{% endfor %}

#endif /* HARDWARE_CONFIG_H */
"""


def macro(name):
    """Нормализует имя в C-макрос: blink_period_ms → BLINK_PERIOD_MS"""
    return "".join(ch if ch.isalnum() else "_" for ch in str(name)).upper()


def load_yaml(path, what):
    if not os.path.exists(path):
        sys.exit(f"Error: {what} file does not exist: {path}")
    with open(path, encoding="utf-8") as f:
        data = yaml.safe_load(f) or {}
    if not isinstance(data, dict):
        sys.exit(f"Error: {what} file must be a YAML mapping: {path}")
    return data


def validate(board, app):
    errors = []
    if "mcu" not in board:
        errors.append("board: missing 'mcu' (нужен mcu.part)")
    elif not isinstance(board["mcu"], dict) or "part" not in board["mcu"]:
        errors.append("board: missing 'mcu.part'")
    for key in ("frequencies", "memory"):
        if key in board and not isinstance(board[key], dict):
            errors.append(f"board: '{key}' should be a mapping")
    for key in ("features", "params"):
        if key in app and not isinstance(app[key], dict):
            errors.append(f"app: '{key}' should be a mapping")
    return errors


def main():
    if len(sys.argv) != 3:
        sys.exit("Usage: gen_config.py <board.yml> <app.yml>")

    board = load_yaml(sys.argv[1], "board")
    app = load_yaml(sys.argv[2], "app")

    errors = validate(board, app)
    if errors:
        print("Config validation errors:", file=sys.stderr)
        for e in errors:
            print(f"  - {e}", file=sys.stderr)
        sys.exit(1)

    out_dir = Path(os.getcwd()) / "generated"
    out_dir.mkdir(parents=True, exist_ok=True)

    content = Template(TEMPLATE).render(
        cpu=macro(board["cpu"]) if board.get("cpu") else "",
        frequencies={macro(k): v for k, v in (board.get("frequencies") or {}).items()},
        memory={macro(k): v for k, v in (board.get("memory") or {}).items()},
        features={macro(k): v for k, v in (app.get("features") or {}).items()},
        params={macro(k): v for k, v in (app.get("params") or {}).items()},
    )

    path = out_dir / "hardware_config.h"
    path.write_text(content, encoding="utf-8")
    print(f"Generated {path}")


if __name__ == "__main__":
    main()
