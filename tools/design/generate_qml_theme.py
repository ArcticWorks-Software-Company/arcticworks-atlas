#!/usr/bin/env python3
"""Generate the QML theme data singleton from ArcticWorks design tokens.

Reads W3C Design Tokens from tools/design/vendor/tokens/ plus per-theme
semantic overlays in tools/design/vendor/themes/<theme>/tokens.json, resolves
all {group.name} references to concrete values, applies density tiers, and
emits qt/qml/aw/themes/generated/AWThemeData.qml.

Usage: python3 tools/design/generate_qml_theme.py
"""

import json
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
VENDOR_DIR = REPO_ROOT / "tools" / "design" / "vendor"
TOKENS_DIR = VENDOR_DIR / "tokens"
THEMES_DIR = VENDOR_DIR / "themes"
OUT_PATH = REPO_ROOT / "qt" / "qml" / "aw" / "themes" / "generated" / "AWThemeData.qml"

REF_RE = re.compile(r"\{([a-zA-Z0-9]+(?:-[a-zA-Z0-9]+)*(?:\.[a-zA-Z0-9-]+)*)\}")
FULL_REF_RE = re.compile(r"^\{([a-zA-Z0-9]+(?:-[a-zA-Z0-9]+)*(?:\.[a-zA-Z0-9-]+)*)\}$")


def is_shadow_object(v):
    return isinstance(v, dict) and "offsetX" in v


def load_tokens_into(target, directory):
    for path in sorted(directory.glob("*.json")):
        if path.name == "density.json":
            continue
        with open(path, encoding="utf-8") as f:
            data = json.load(f)
        for group, value in data.items():
            if group in target:
                raise RuntimeError(f'Duplicate top-level group "{group}" in {path.name}')
            target[group] = value


def flatten(node):
    """Flatten the merged token tree into a list of (path, value, type)."""
    leaves = []

    def walk(current, path, inherited_type):
        for key, raw in current.items():
            if key.startswith("$"):
                continue
            value = raw
            token_type = inherited_type
            if isinstance(raw, dict) and not is_shadow_object(raw):
                if "$type" in raw:
                    token_type = raw["$type"]
                if "$value" in raw:
                    value = raw["$value"]
                else:
                    walk(raw, path + [key], token_type)
                    continue
            if isinstance(value, dict) and not is_shadow_object(value):
                walk(value, path + [key], token_type)
            else:
                leaves.append((path + [key], value, token_type))

    walk(node, [], None)
    return leaves


def resolve_values(flat):
    """Resolve {group.name} references in flat {path: value} map to concrete values."""
    resolving = set()

    def resolve_path(name):
        if name in resolving:
            raise RuntimeError(f"Token reference cycle at {name}")
        resolving.add(name)
        try:
            return resolve_value(name, flat[name])
        finally:
            resolving.discard(name)

    def resolve_value(name, value):
        if isinstance(value, str):
            full = FULL_REF_RE.match(value)
            if full:
                target = full.group(1)
                if target not in flat:
                    raise RuntimeError(f"Unresolved token reference: {{{target}}} from {name}")
                return resolve_path(target)

            def sub(match):
                target = match.group(1)
                if target not in flat:
                    raise RuntimeError(f"Unresolved token reference: {{{target}}} from {name}")
                return str(resolve_path(target))

            return REF_RE.sub(sub, value)
        if isinstance(value, list):
            return [resolve_value(name, item) for item in value]
        if isinstance(value, dict):
            return {k: resolve_value(name, v) for k, v in value.items()}
        return value

    return {name: resolve_value(name, value) for name, value in flat.items()}


def to_qml_value(value, token_type):
    """Convert a resolved value to a QML/JS literal."""
    if isinstance(value, bool):
        return value
    if isinstance(value, (int, float)):
        return value
    if isinstance(value, list):
        if token_type == "cubicBezier":
            return [float(x) for x in value]
        return [to_qml_value(item, None) for item in value]
    if isinstance(value, dict):
        return to_shadow(value)
    assert isinstance(value, str), f"unexpected value type {type(value)}: {value!r}"

    if token_type in ("fontWeight", "lineHeight", "number"):
        try:
            return float(value)
        except ValueError:
            return value

    m = re.match(r"^-?\d+(\.\d+)?px$", value)
    if m:
        return float(value[:-2])
    m = re.match(r"^-?\d+(\.\d+)?ms$", value)
    if m:
        return float(value[:-2])
    m = re.match(r"^-?\d+$", value)
    if m:
        return int(value)
    return value


def to_shadow(shadow):
    def num(v):
        m = re.match(r"^-?\d+(\.\d+)?px$", str(v))
        return float(str(v)[:-2]) if m else 0.0

    return {
        "offsetX": num(shadow.get("offsetX", "0")),
        "offsetY": num(shadow.get("offsetY", "0")),
        "blur": num(shadow.get("blur", "0")),
        "spread": num(shadow.get("spread", "0")),
        "color": shadow.get("color", "rgba(0,0,0,0)"),
    }


def build_base(overlay_path):
    """Merge base tokens + theme semantic overlay, return resolved {path: value}."""
    merged = {}
    load_tokens_into(merged, TOKENS_DIR)

    if overlay_path is not None and overlay_path.exists():
        with open(overlay_path, encoding="utf-8") as f:
            overlay = json.load(f)
        for group, value in overlay.items():
            if isinstance(value, dict) and not is_shadow_object(value):
                merged[group] = {**(merged.get(group) or {}), **value}
            else:
                merged[group] = value

    leaves = flatten(merged)
    flat = {".".join(path): value for path, value, _ in leaves}
    types = {".".join(path): token_type for path, _, token_type in leaves}
    resolved = resolve_values(flat)
    return {name: to_qml_value(resolved[name], types.get(name)) for name in resolved}


def qml_render(value, indent):
    pad = " " * indent
    if isinstance(value, bool):
        return "true" if value else "false"
    if isinstance(value, (int, float)):
        return repr(value)
    if isinstance(value, str):
        return json.dumps(value)
    if isinstance(value, list):
        if not value:
            return "[]"
        inner = ", ".join(qml_render(v, indent + 4) for v in value)
        return f"[\n{' ' * (indent + 4)}{inner}\n{pad}]"
    if isinstance(value, dict):
        items = []
        for key in sorted(value):
            items.append(f"{' ' * (indent + 4)}{json.dumps(key)}: {qml_render(value[key], indent + 4)}")
        return "{\n" + ",\n".join(items) + "\n" + pad + "}"
    raise RuntimeError(f"cannot render {value!r}")


def main():
    with open(TOKENS_DIR / "density.json", encoding="utf-8") as f:
        density_meta = json.load(f)

    themes = {}
    overlays = {"dark": None}
    for theme_dir in sorted(THEMES_DIR.iterdir()):
        if theme_dir.is_dir():
            overlays[theme_dir.name] = theme_dir / "tokens.json"

    for theme, overlay_path in overlays.items():
        base = build_base(overlay_path)

        variants = {"compact": base}
        for tier, overrides in density_meta.get("tiers", {}).items():
            if not isinstance(overrides, dict):
                continue
            tier_map = {path: raw for path, raw in overrides.items() if not path.startswith("$")}
            merged = dict(base)
            merged.update(tier_map)
            variants[tier] = resolve_values(merged)

        themes[theme] = variants

    theme_names = sorted(themes)
    tier_names = sorted({t for v in themes.values() for t in v})

    lines = [
        "pragma Singleton",
        "import QtQuick",
        "",
        "// GENERATED FILE - do not edit by hand.",
        "// Generated by tools/design/generate_qml_theme.py from ArcticWorks design tokens.",
        "// All values are fully resolved (no token references remain).",
        "QtObject {",
        f"    readonly property var themeNames: {json.dumps(theme_names)}",
        f"    readonly property var densityTiers: {json.dumps(tier_names)}",
        f"    readonly property string defaultDensity: {json.dumps(density_meta.get('default', 'compact'))}",
        "    readonly property var themes: " + qml_render(themes, 8).lstrip("\n") + "",
        "}",
    ]

    OUT_PATH.parent.mkdir(parents=True, exist_ok=True)
    with open(OUT_PATH, "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(lines) + "\n")

    print(f"Wrote {OUT_PATH} ({len(themes)} themes, {len(tier_names)} density tiers)")


if __name__ == "__main__":
    sys.exit(main())
