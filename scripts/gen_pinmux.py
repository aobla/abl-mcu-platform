#!/usr/bin/env python3
"""
Pinmux generator (D5/D9).

    gen_pinmux.py <board.yml> <app.yml>

  board — onboard: { <alias>: {port, pin, mode, pull, state, speed, otype, af} }
  app   — pins:    { <logical>: {use: <alias>, ...} | {port, pin, ...} }

Output: ./generated/hardware_pins.h, ./generated/generated_gpio_init.c

Templates live in templates/ (single source, §9).

Checks (configuration errors instead of surprises on hardware):
  - use: points to an unknown onboard resource;
  - an explicit pin has no port/pin;
  - invalid mode/pull/state/speed/otype/af or a non-identifier name;
  - two logical pins of the project resolve to the same physical pin.
"""

import re
import sys

from abl_codegen import emit, load_template, load_yaml, report_errors

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


def generate(board_path, app_path):
    """Generate hardware_pins.h + generated_gpio_init.c; returns written paths."""
    board = load_yaml(board_path, "board")
    app = load_yaml(app_path, "app")

    pins, errors = resolve_pins(board, app)
    report_errors("Pinmux configuration errors", errors)

    if not pins:
        print("Warning: app.pins is empty — no pin will be initialised", file=sys.stderr)

    header = load_template("hardware_pins.h.jinja").render(pins=pins)
    source = load_template("generated_gpio_init.c.jinja").render(pins=pins)

    return emit("hardware_pins.h", header), emit("generated_gpio_init.c", source)


def main():
    if len(sys.argv) != 3:
        sys.exit("Usage: gen_pinmux.py <board.yml> <app.yml>")
    generate(sys.argv[1], sys.argv[2])


if __name__ == "__main__":
    main()
