#!/usr/bin/env python3
"""Validate the structural integrity of the campaign playlist and level files."""
from __future__ import annotations

import argparse
import json
from pathlib import Path

DEFAULT_CAMPAIGN = Path("Game/Assets/Levels/campaign.txt")
DEFAULT_LEVELS_ROOT = Path("Game/Assets/Levels")


def validate_campaign_catalogue(campaign_file: Path, levels_root: Path) -> list[str]:
    errors: list[str] = []
    entries: list[tuple[int, str]] = []

    if not campaign_file.is_file():
        return [f"campaign file does not exist: {campaign_file}"]
    if not levels_root.is_dir():
        return [f"levels root does not exist: {levels_root}"]

    try:
        lines = campaign_file.read_text(encoding="utf-8").splitlines()
    except UnicodeError as exc:
        return [f"campaign file is not valid UTF-8: {exc}"]

    for line_number, raw in enumerate(lines, 1):
        line = raw.rstrip("\r")
        if not line or line.startswith("#"):
            continue
        if line != line.strip():
            errors.append(
                f"line {line_number}: leading/trailing whitespace is not allowed: {line!r}"
            )
            continue
        entries.append((line_number, line))

    if not entries:
        errors.append("campaign contains no level entries")
        return errors

    seen: dict[str, int] = {}
    root = levels_root.resolve()

    for line_number, entry in entries:
        path = Path(entry)
        if (
            path.is_absolute()
            or any(part == ".." for part in path.parts)
            or "\\" in entry
        ):
            errors.append(
                f"line {line_number}: campaign path must be repository-relative and portable: {entry!r}"
            )
            continue
        if path.suffix.lower() != ".lvl":
            errors.append(
                f"line {line_number}: campaign entry must end in .lvl: {entry!r}"
            )
            continue

        key = path.as_posix().casefold()
        previous = seen.get(key)
        if previous is not None:
            errors.append(
                f"line {line_number}: duplicate campaign level {entry!r}; first declared on line {previous}"
            )
        else:
            seen[key] = line_number

        target = (root / path).resolve()
        try:
            target.relative_to(root)
        except ValueError:
            errors.append(
                f"line {line_number}: campaign level escapes the levels root: {entry!r}"
            )
            continue
        if not target.is_file():
            errors.append(
                f"line {line_number}: campaign level does not exist: {entry!r}"
            )
            continue

        try:
            level_lines = target.read_text(encoding="utf-8").splitlines()
        except UnicodeError as exc:
            errors.append(f"line {line_number}: level file is not valid UTF-8: {entry!r}: {exc}")
            continue

        for level_line_number, raw_level in enumerate(level_lines, 1):
            level_line = raw_level.strip()
            if level_line and not level_line.startswith("#") and level_line.split()[0] == "FLAG":
                errors.append(
                    f"{entry}:{level_line_number}: FLAG is not allowed in .lvl; campaign goal is derived automatically"
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
    else:
        if errors:
            for error in errors:
                print(f"ERROR: {error}")
        else:
            print("OK: campaign catalogue and level-file structure are valid")

    return 1 if errors else 0


if __name__ == "__main__":
    raise SystemExit(main())
