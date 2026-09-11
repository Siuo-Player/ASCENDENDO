#!/usr/bin/env python3
"""Validate the structural integrity of the campaign playlist and level files."""
from __future__ import annotations

import argparse
import json
from pathlib import Path

DEFAULT_CAMPAIGN = Path(__file__).resolve().parents[2] / "Game" / "Assets" / "Levels" / "campaign.txt"
DEFAULT_LEVELS_ROOT = DEFAULT_CAMPAIGN.parent


def validate_campaign_catalogue(campaign_file: Path, levels_root: Path) -> list[str]:
    errors: list[str] = []
    try:
        lines = campaign_file.read_text(encoding="utf-8").splitlines()
    except FileNotFoundError:
        return [f"campaign file does not exist: {campaign_file}"]
    except UnicodeError as exc:
        return [f"campaign file is not valid UTF-8: {exc}"]

    seen: set[str] = set()
    for line_number, raw in enumerate(lines, 1):
        entry = raw.strip()
        if not entry or entry.startswith("#"):
            continue
        if entry in seen:
            errors.append(f"line {line_number}: duplicate campaign entry: {entry!r}")
        seen.add(entry)
        if Path(entry).is_absolute() or "\\" in entry:
            errors.append(f"line {line_number}: invalid campaign path: {entry!r}")
            continue

        target = (levels_root / entry).resolve()
        try:
            target.relative_to(levels_root.resolve())
        except ValueError:
            errors.append(f"line {line_number}: campaign level escapes the levels root: {entry!r}")
            continue
        if target.suffix.lower() != ".lvl":
            errors.append(f"line {line_number}: campaign entry is not a .lvl file: {entry!r}")
            continue
        if not target.is_file():
            errors.append(f"line {line_number}: campaign level does not exist: {entry!r}")
            continue

        try:
            level_lines = target.read_text(encoding="utf-8").splitlines()
        except UnicodeError as exc:
            errors.append(f"line {line_number}: level file is not valid UTF-8: {entry!r}: {exc}")
            continue

        for level_line_number, raw_level in enumerate(level_lines, 1):
            level_line = raw_level.strip()
            if not level_line or level_line.startswith("#"):
                continue
            directive = level_line.split()[0]
            if directive in {"FLAG", "SPAWN"}:
                errors.append(
                    f"{entry}:{level_line_number}: {directive} is not allowed in .lvl; it is derived automatically"
                )

    return errors


def main() -> int:
    parser = argparse.ArgumentParser(description="Validate campaign playlist references")
    parser.add_argument("--campaign", default=str(DEFAULT_CAMPAIGN))
    parser.add_argument("--levels-root", default=str(DEFAULT_LEVELS_ROOT))
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()

    errors = validate_campaign_catalogue(Path(args.campaign), Path(args.levels_root))
    if args.json:
        print(json.dumps({"valid": not errors, "errors": errors}, indent=2))
    elif errors:
        for error in errors:
            print(f"ERROR: {error}")
    else:
        print("OK: campaign catalogue and level-file structure are valid")

    return 1 if errors else 0


if __name__ == "__main__":
    raise SystemExit(main())
