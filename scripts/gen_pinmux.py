#!/usr/bin/env python3
"""
Pinmux generation from the board + the project overlay (D5/D9).

    gen_pinmux.py <board.yml> <app.yml>

  board.yml — onboard: { <alias>: {port, pin, mode, pull, state, speed, otype, af} }
  app.yml   — pins:    { <logical>: {use: <alias>, ...} | {port, pin, ...} }

Output: ./generated/hardware_pins.h, ./generated/generated_gpio_init.c

Checks (configuration errors instead of surprises on hardware):
  - use: points to an unknown onboard resource;
  - an explicit pin has no port/pin;
  - invalid mode/pull/state/speed/otype/af or a non-identifier name;
  - two logical pins of the project resolve to the same physical pin.

Note: the generated code is platform-bound (port macros such as GPIOC), which is
why it includes the SoC-provided vendor header (soc_hal.h) itself.
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
VALID_SPEEDS = ("low", "medium", "high", "very_high")
VALID_OTYPES = ("push_pull", "open_drain")

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
SPEED_MAP = {
    "low": "ABL_GPIO_SPEED_LOW",
    "medium": "ABL_GPIO_SPEED_MEDIUM",
    "high": "ABL_GPIO_SPEED_HIGH",
    "very_high": "ABL_GPIO_SPEED_VERY_HIGH",
}
OTYPE_MAP = {
    "push_pull": "ABL_GPIO_OTYPE_PUSH_PULL",
    "open_drain": "ABL_GPIO_OTYPE_OPEN_DRAIN",
}

IDENT_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")

PINS_TEMPLATE = """#ifndef HARDWARE_PINS_H
#define HARDWARE_PINS_H

#include "abl_gpio.h"
#include "soc_hal.h"   /* port macros (GPIOC, ...) from the SoC definition */

/* Generated from board + app overlay — do not edit by hand. */

{% for name, p in pins.items() %}
#define {{ name|upper }}_PORT  {{ p.port_literal }}
#define {{ name|upper }}_PIN   {{ p.pin }}
#define {{ name|upper }}_MODE  {{ p.mode_macro }}
#define {{ name|upper }}_PULL  {{ p.pull_macro }}
#define {{ name|upper }}_STATE {{ p.state_macro }}
#define {{ name|upper }}_SPEED {{ p.speed_macro }}
#define {{ name|upper }}_OTYPE {{ p.otype_macro }}
#define {{ name|upper }}_AF    {{ p.af }}
{% endfor %}

/* ─── Pin handles (kept in FLASH, no RAM cost) ──────────────────────────── */
{% for name, p in pins.items() %}
static const abl_gpio_pin_t pin_{{ name }} = { {{ name|upper }}_PORT, {{ name|upper }}_PIN };
{% endfor %}

/* ─── Convenience macros ────────────────────────────────────────────────── */
/** Pointer to a pin handle by name */
#define PIN_GET(name) (&pin_##name)

#define abl_gpio_toggle_pin(name)        abl_gpio_toggle(PIN_GET(name))
#define abl_gpio_write_pin(name, state)  abl_gpio_write(PIN_GET(name), state)
#define abl_gpio_read_pin(name, out)     abl_gpio_read(PIN_GET(name), out)

#endif /* HARDWARE_PINS_H */
"""

GPIO_INIT_TEMPLATE = """#include "hardware_pins.h"    /* + abl_gpio.h, soc_hal.h */
#include "abl_target_init.h"
#include <stddef.h>

/* Generated from board + app overlay — do not edit by hand. */

{% if pins %}
typedef struct {
    abl_gpio_pin_t pin;
    abl_gpio_cfg_t cfg;
} gpio_init_entry_t;

static const gpio_init_entry_t gpio_init_table[] = {
{% for name, p in pins.items() %}
    { { {{ name|upper }}_PORT, {{ name|upper }}_PIN },
      { {{ name|upper }}_MODE, {{ name|upper }}_PULL, {{ name|upper }}_STATE,
        {{ name|upper }}_SPEED, {{ name|upper }}_OTYPE, {{ name|upper }}_AF } },
{% endfor %}
};

void generated_gpio_init(void)
{
    for (size_t i = 0; i < sizeof(gpio_init_table) / sizeof(gpio_init_table[0]); i++) {
        (void)abl_gpio_configure(&gpio_init_table[i].pin, &gpio_init_table[i].cfg);
    }
}
{% else %}
void generated_gpio_init(void)
{
    /* The project does not use any pin (empty 'pins' section in the app config). */
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
    """Numeric port (AVR index, ESP32 GPIO) -> pointer; string (GPIOC) as is."""
    if isinstance(port, bool):
        raise ValueError("port must not be a boolean")
    if isinstance(port, int):
        return f"((void*)(uintptr_t){port})"
    return str(port)


def resolve_pins(board, app):
    onboard = board.get("onboard") or {}
    overlay = app.get("pins") or {}
    errors = []
    resolved = {}

    if not isinstance(onboard, dict):
        return {}, ["board: 'onboard' must be a mapping"]
    if not isinstance(overlay, dict):
        return {}, ["app: 'pins' must be a mapping"]

    available = ", ".join(sorted(onboard)) or "-"

    for logical, spec in overlay.items():
        if not IDENT_RE.match(str(logical)):
            errors.append(f"pins.{logical}: name must be a C identifier")
            continue
        if not isinstance(spec, dict):
            errors.append(f"pins.{logical}: must be a mapping")
            continue

        if "use" in spec:
            ref = spec["use"]
            if ref not in onboard:
                errors.append(
                    f"pins.{logical}: use: '{ref}' is not an onboard resource "
                    f"(available: {available})"
                )
                continue
            base = onboard[ref]
            if not isinstance(base, dict):
                errors.append(f"board.onboard.{ref}: must be a mapping")
                continue
            base = dict(base)
        else:
            if "port" not in spec or "pin" not in spec:
                errors.append(
                    f"pins.{logical}: needs 'use: <onboard alias>' "
                    f"or explicit 'port' and 'pin'"
                )
                continue
            base = {"port": spec["port"], "pin": spec["pin"]}

        # Optional overrides coming from the project overlay
        for key in ("mode", "pull", "state", "speed", "otype", "af"):
            if key in spec:
                base[key] = spec[key]

        base.setdefault("mode", "output")
        base.setdefault("pull", "none")
        base.setdefault("state", "none")
        base.setdefault("speed", "low")
        base.setdefault("otype", "push_pull")
        base.setdefault("af", 0)

        if base["mode"] not in VALID_MODES:
            errors.append(f"pins.{logical}: invalid mode '{base['mode']}' (valid: {', '.join(VALID_MODES)})")
            continue
        if base["pull"] not in VALID_PULLS:
            errors.append(f"pins.{logical}: invalid pull '{base['pull']}' (valid: {', '.join(VALID_PULLS)})")
            continue
        if base["state"] not in VALID_STATES:
            errors.append(f"pins.{logical}: invalid state '{base['state']}' (valid: {', '.join(VALID_STATES)})")
            continue
        if base["speed"] not in VALID_SPEEDS:
            errors.append(f"pins.{logical}: invalid speed '{base['speed']}' (valid: {', '.join(VALID_SPEEDS)})")
            continue
        if base["otype"] not in VALID_OTYPES:
            errors.append(f"pins.{logical}: invalid otype '{base['otype']}' (valid: {', '.join(VALID_OTYPES)})")
            continue
        if not isinstance(base["pin"], int) or isinstance(base["pin"], bool) or base["pin"] < 0:
            errors.append(f"pins.{logical}: 'pin' must be a non-negative integer")
            continue
        if not isinstance(base["af"], int) or isinstance(base["af"], bool) or not 0 <= base["af"] <= 15:
            errors.append(f"pins.{logical}: 'af' must be an integer in 0..15")
            continue

        try:
            base["port_literal"] = port_literal(base["port"])
        except ValueError as e:
            errors.append(f"pins.{logical}: {e}")
            continue

        base["mode_macro"] = MODE_MAP[base["mode"]]
        base["pull_macro"] = PULL_MAP[base["pull"]]
        base["state_macro"] = STATE_MAP[base["state"]]
        base["speed_macro"] = SPEED_MAP[base["speed"]]
        base["otype_macro"] = OTYPE_MAP[base["otype"]]
        resolved[str(logical)] = base

    # Conflicts inside the project: two logical pins on one physical pin
    seen = {}
    for logical, spec in resolved.items():
        key = (str(spec["port"]), spec["pin"])
        if key in seen:
            errors.append(
                f"pin conflict: '{logical}' and '{seen[key]}' both use "
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
        print("Warning: app.pins is empty — no pin will be initialised", file=sys.stderr)

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
