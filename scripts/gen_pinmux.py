#!/usr/bin/env python3
"""
Генерация pinmux из board + overlay проекта (D5/D9).

    gen_pinmux.py <board.yml> <app.yml>

  board.yml — onboard: { <alias>: {port, pin, mode, pull, state} }   (физика платы)
  app.yml   — pins:    { <logical>: {use: <alias>, ...} | {port, pin, ...} }  (overlay)

Выход: ./generated/hardware_pins.h, ./generated/generated_gpio_init.c

Проверки (ошибка конфигурации вместо «чуда» на железе):
  - use: ссылается на несуществующий onboard-ресурс;
  - у явного пина нет port/pin;
  - недопустимые mode/pull/state или некорректное имя;
  - два логических пина проекта на одном физическом пине.
"""

import os
import re
import sys
from pathlib import Path

import yaml
from jinja2 import Template

VALID_MODES = ("input", "output", "alt_function", "analog")
VALID_PULLS = ("none", "up", "down")
VALID_STATES = ("none", "high", "low")

MODE_MAP = {
    "input": "ABL_GPIO_MODE_INPUT",
    "output": "ABL_GPIO_MODE_OUTPUT",
    "alt_function": "ABL_GPIO_MODE_ALT_FUNCTION",
    "analog": "ABL_GPIO_MODE_ANALOG",
}
PULL_MAP = {
    "none": "ABL_GPIO_PULL_NONE",
    "up": "ABL_GPIO_PULL_UP",
    "down": "ABL_GPIO_PULL_DOWN",
}
STATE_MAP = {
    "none": "ABL_GPIO_STATE_NONE",
    "high": "ABL_GPIO_STATE_HIGH",
    "low": "ABL_GPIO_STATE_LOW",
}

IDENT_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")

PINS_TEMPLATE = """#ifndef HARDWARE_PINS_H
#define HARDWARE_PINS_H

#include "abl_gpio.h"
#include "soc_hal.h"   /* макросы портов (GPIOC и т.п.) — из SoC-дефиниции */

/* Сгенерировано из board + app overlay — не редактировать вручную. */

{% for name, p in pins.items() %}
#define {{ name|upper }}_PORT  {{ p.port_literal }}
#define {{ name|upper }}_PIN   {{ p.pin }}
#define {{ name|upper }}_MODE  {{ p.mode_macro }}
#define {{ name|upper }}_PULL  {{ p.pull_macro }}
#define {{ name|upper }}_STATE {{ p.state_macro }}
{% endfor %}

/* ─── Хэндлы пинов (во FLASH, без расхода RAM) ──────────────────────────── */
{% for name, p in pins.items() %}
static const abl_gpio_pin_t pin_{{ name }} = { {{ name|upper }}_PORT, {{ name|upper }}_PIN };
{% endfor %}

/* ─── Удобные макросы ───────────────────────────────────────────────────── */
/** Указатель на хэндл пина по имени */
#define PIN_GET(name) (&pin_##name)

#define abl_gpio_toggle_pin(name)        abl_gpio_toggle(PIN_GET(name))
#define abl_gpio_write_pin(name, state)  abl_gpio_write(PIN_GET(name), state)
#define abl_gpio_read_pin(name, out)     abl_gpio_read(PIN_GET(name), out)

#endif /* HARDWARE_PINS_H */
"""

GPIO_INIT_TEMPLATE = """#include "hardware_pins.h"    /* + abl_gpio.h, soc_hal.h */
#include "abl_target_init.h"
#include <stddef.h>

/* Сгенерировано из board + app overlay — не редактировать вручную. */

{% if pins %}
typedef struct {
    void*            port;
    uint16_t         pin;
    abl_gpio_mode_t  mode;
    abl_gpio_pull_t  pull;
    abl_gpio_state_t state;
} gpio_init_entry_t;

static const gpio_init_entry_t gpio_init_table[] = {
{% for name, p in pins.items() %}
    { {{ name|upper }}_PORT, {{ name|upper }}_PIN, {{ p.mode_macro }}, {{ p.pull_macro }}, {{ p.state_macro }} },
{% endfor %}
};

void generated_gpio_init(void)
{
    for (size_t i = 0; i < sizeof(gpio_init_table) / sizeof(gpio_init_table[0]); i++) {
        abl_gpio_pin_t pin = { gpio_init_table[i].port, gpio_init_table[i].pin };
        abl_gpio_init(&pin, gpio_init_table[i].mode, gpio_init_table[i].pull, gpio_init_table[i].state);
    }
}
{% else %}
void generated_gpio_init(void)
{
    /* Проект не использует пинов (секция pins в app-конфиге пуста). */
}
{% endif %}
"""


def load_yaml(path, what):
    if not os.path.exists(path):
        sys.exit(f"Error: {what} file does not exist: {path}")
    with open(path, encoding="utf-8") as f:
        data = yaml.safe_load(f) or {}
    if not isinstance(data, dict):
        sys.exit(f"Error: {what} file must be a YAML mapping: {path}")
    return data


def port_literal(port):
    """Числовой порт (AVR-индекс, ESP32 GPIO) → указатель; строковый (GPIOC) — как есть."""
    if isinstance(port, bool):
        raise ValueError("port не может быть bool")
    if isinstance(port, int):
        return f"((void*)(uintptr_t){port})"
    return str(port)


def resolve_pins(board, app):
    onboard = board.get("onboard") or {}
    overlay = app.get("pins") or {}
    errors = []
    resolved = {}

    if not isinstance(onboard, dict):
        return {}, ["board: 'onboard' должен быть словарём"]
    if not isinstance(overlay, dict):
        return {}, ["app: 'pins' должен быть словарём"]

    available = ", ".join(sorted(onboard)) or "—"

    for logical, spec in overlay.items():
        if not IDENT_RE.match(str(logical)):
            errors.append(f"pins.{logical}: имя должно быть C-идентификатором")
            continue
        if not isinstance(spec, dict):
            errors.append(f"pins.{logical}: должен быть словарём")
            continue

        if "use" in spec:
            ref = spec["use"]
            if ref not in onboard:
                errors.append(
                    f"pins.{logical}: use: '{ref}' не найден в onboard платы "
                    f"(доступно: {available})"
                )
                continue
            base = onboard[ref]
            if not isinstance(base, dict):
                errors.append(f"board.onboard.{ref}: должен быть словарём")
                continue
            base = dict(base)
            source = f"use: {ref}"
        else:
            if "port" not in spec or "pin" not in spec:
                errors.append(
                    f"pins.{logical}: нужен 'use: <onboard-алиас>' "
                    f"либо явные 'port' и 'pin'"
                )
                continue
            base = {"port": spec["port"], "pin": spec["pin"]}
            source = "явный пин"

        # Необязательные переопределения из overlay
        for key in ("mode", "pull", "state"):
            if key in spec:
                base[key] = spec[key]

        base.setdefault("mode", "output")
        base.setdefault("pull", "none")
        base.setdefault("state", "none")

        if base["mode"] not in VALID_MODES:
            errors.append(f"pins.{logical}: недопустимый mode '{base['mode']}' (допустимо: {', '.join(VALID_MODES)})")
            continue
        if base["pull"] not in VALID_PULLS:
            errors.append(f"pins.{logical}: недопустимый pull '{base['pull']}' (допустимо: {', '.join(VALID_PULLS)})")
            continue
        if base["state"] not in VALID_STATES:
            errors.append(f"pins.{logical}: недопустимый state '{base['state']}' (допустимо: {', '.join(VALID_STATES)})")
            continue
        if not isinstance(base["pin"], int) or isinstance(base["pin"], bool) or base["pin"] < 0:
            errors.append(f"pins.{logical}: 'pin' должен быть неотрицательным целым")
            continue

        try:
            base["port_literal"] = port_literal(base["port"])
        except ValueError as e:
            errors.append(f"pins.{logical}: {e}")
            continue

        base["mode_macro"] = MODE_MAP[base["mode"]]
        base["pull_macro"] = PULL_MAP[base["pull"]]
        base["state_macro"] = STATE_MAP[base["state"]]
        base["source"] = source
        resolved[str(logical)] = base

    # Конфликты внутри проекта: два логических пина на одном физическом
    seen = {}
    for logical, spec in resolved.items():
        key = (str(spec["port"]), spec["pin"])
        if key in seen:
            errors.append(
                f"конфликт пинов: '{logical}' и '{seen[key]}' используют "
                f"port={spec['port']}, pin={spec['pin']}"
            )
        else:
            seen[key] = logical

    return resolved, errors


def main():
    if len(sys.argv) != 3:
        sys.exit("Usage: gen_pinmux.py <board.yml> <app.yml>")

    board = load_yaml(sys.argv[1], "board")
    app = load_yaml(sys.argv[2], "app")

    pins, errors = resolve_pins(board, app)
    if errors:
        print("Pinmux configuration errors:", file=sys.stderr)
        for e in errors:
            print(f"  - {e}", file=sys.stderr)
        sys.exit(1)

    if not pins:
        print("Warning: app.pins пуст — ни один пин не будет инициализирован", file=sys.stderr)

    out_dir = Path(os.getcwd()) / "generated"
    out_dir.mkdir(parents=True, exist_ok=True)

    pins_h = out_dir / "hardware_pins.h"
    pins_h.write_text(Template(PINS_TEMPLATE).render(pins=pins), encoding="utf-8")
    print(f"Generated {pins_h}")

    init_c = out_dir / "generated_gpio_init.c"
    init_c.write_text(Template(GPIO_INIT_TEMPLATE).render(pins=pins), encoding="utf-8")
    print(f"Generated {init_c}")


if __name__ == "__main__":
    main()
