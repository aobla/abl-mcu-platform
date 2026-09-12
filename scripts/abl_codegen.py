"""
Shared helpers for the ABL code generators.

Templates are the single source of truth for generated code and live in
templates/ (ARCHITECTURE.md §9); generators only provide data and validation and
must not carry inline templates.

Output always goes to <cwd>/generated, because the build directory owns the
generated files (both the plain CMake build and the ESP-IDF wrapper run the
generators with their build directory as the working directory).
"""

import os
import sys
from pathlib import Path

import yaml
from jinja2 import Template

REPO_ROOT = Path(__file__).resolve().parent.parent
TEMPLATES_DIR = REPO_ROOT / "templates"
GENERATED_DIR_NAME = "generated"


def fail(message):
    """Abort with a configuration error."""
    print(f"Error: {message}", file=sys.stderr)
    sys.exit(1)


def report_errors(title, errors):
    """Print validation errors (if any) and abort."""
    if errors:
        print(f"{title}:", file=sys.stderr)
        for error in errors:
            print(f"  - {error}", file=sys.stderr)
        sys.exit(1)


def load_template(name):
    """Load a Jinja template from templates/<name>."""
    path = TEMPLATES_DIR / name
    if not path.is_file():
        fail(f"template not found: {path}")
    return Template(path.read_text(encoding="utf-8"))


def load_yaml(path, what):
    """Load a YAML mapping, failing with a clear message."""
    if not os.path.exists(path):
        fail(f"{what} file does not exist: {path}")
    with open(path, encoding="utf-8") as f:
        data = yaml.safe_load(f) or {}
    if not isinstance(data, dict):
        fail(f"{what} file must be a YAML mapping: {path}")
    return data


def output_dir():
    """<cwd>/generated, created on demand."""
    path = Path(os.getcwd()) / GENERATED_DIR_NAME
    path.mkdir(parents=True, exist_ok=True)
    return path


def emit(filename, content):
    """Write a generated file and report it."""
    path = output_dir() / filename
    path.write_text(content, encoding="utf-8")
    print(f"Generated {path}")
    return path


def macro(name):
    """Normalize a config key into a C macro name: blink_period_ms -> BLINK_PERIOD_MS"""
    return "".join(ch if ch.isalnum() else "_" for ch in str(name)).upper()
