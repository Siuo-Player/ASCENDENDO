#!/usr/bin/env python3
"""Validate campaign playlists and the optional multi-campaign catalogue."""
from __future__ import annotations

import argparse
import json
from pathlib import Path

DEFAULT_ASSETS_ROOT = Path("Game/Assets")
DEFAULT_CAMPAIGN = DEFAULT_ASSETS_ROOT / "Levels/campaign.txt"
DEFAULT_CAMPAIGN_CATALOGUE = DEFAULT_ASSETS_ROOT / "Campaigns/catalogue.txt"


def _portable_relative_path(value: str) -> bool:
    path = Path(value)
    if path.is_absolute() or "\\" in value:
        return False
    return all(part not in {"", ".."} for part in path.parts)


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
            errors.append(
                f"line {line_number}: level file is not valid UTF-8: {entry!r}: {exc}"
            )
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


def validate_campaign_catalogue_file(
    catalogue_file: Path,
    assets_root: Path,
) -> list[str]:
    """Validate every playlist referenced by the optional campaign catalogue."""
    errors: list[str] = []
    if not catalogue_file.is_file():
        return []
    if not assets_root.is_dir():
        return [f"assets root does not exist: {assets_root}"]

    try:
        lines = catalogue_file.read_text(encoding="utf-8").splitlines()
    except UnicodeError as exc:
        return [f"campaign catalogue is not valid UTF-8: {exc}"]

    catalogue_root = catalogue_file.parent.resolve()
    assets_root_resolved = assets_root.resolve()
    seen_ids: dict[str, int] = {}
    seen_playlists: dict[str, int] = {}
    campaign_entries = 0

    for line_number, raw in enumerate(lines, 1):
        line = raw.rstrip("\r")
        if not line or line.startswith("#"):
            continue
        if line != line.strip():
            errors.append(
                f"line {line_number}: leading/trailing whitespace is not allowed: {line!r}"
            )
            continue

        fields = [field.strip() for field in line.split("|")]
        if len(fields) != 3:
            errors.append(
                f"line {line_number}: expected id|name|playlist"
            )
            continue

        campaign_id, name, playlist = fields
        campaign_entries += 1

        if not campaign_id or not campaign_id[0].isalnum() or any(
            not (char.isalnum() or char in "_-") for char in campaign_id
        ):
            errors.append(f"line {line_number}: invalid campaign id")
        elif campaign_id in seen_ids:
            errors.append(
                f"line {line_number}: duplicate campaign id: {campaign_id!r}; first declared on line {seen_ids[campaign_id]}"
            )
        else:
            seen_ids[campaign_id] = line_number

        if not name:
            errors.append(f"line {line_number}: campaign name is empty")

        playlist_path = Path(playlist)
        if (
            not _portable_relative_path(playlist)
            or playlist_path.name != "campaign.txt"
        ):
            errors.append(
                f"line {line_number}: playlist must be a portable relative path ending in campaign.txt: {playlist!r}"
            )
            continue

        normalized_key = playlist_path.as_posix().casefold()
        if normalized_key in seen_playlists:
            errors.append(
                f"line {line_number}: duplicate campaign playlist: {playlist!r}; first declared on line {seen_playlists[normalized_key]}"
            )
        else:
            seen_playlists[normalized_key] = line_number

        playlist_file = (catalogue_root / playlist_path).resolve()
        try:
            playlist_file.relative_to(assets_root_resolved)
        except ValueError:
            errors.append(f"line {line_number}: playlist escapes the assets root: {playlist!r}")
            continue
        if not playlist_file.is_file():
            errors.append(f"line {line_number}: playlist does not exist: {playlist!r}")
            continue

        level_root = playlist_file.parent
        for error in validate_campaign_catalogue(playlist_file, level_root):
            errors.append(f"{playlist}: {error}")

    if campaign_entries == 0:
        errors.append("campaign catalogue contains no campaign entries")

    return errors


def main() -> int:
    parser = argparse.ArgumentParser(description="Validate campaign playlists and catalogue references")
    parser.add_argument("--campaign", default=None, help="legacy/single campaign.txt path")
    parser.add_argument("--levels-root", default=None)
    parser.add_argument("--catalogue", default=str(DEFAULT_CAMPAIGN_CATALOGUE))
    parser.add_argument("--assets-root", default=str(DEFAULT_ASSETS_ROOT))
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()

    errors: list[str] = []
    catalogue = Path(args.catalogue)
    assets_root = Path(args.assets_root)

    if catalogue.is_file() and args.campaign is None:
        errors.extend(validate_campaign_catalogue_file(catalogue, assets_root))
    else:
        campaign = Path(args.campaign) if args.campaign else DEFAULT_CAMPAIGN
        levels_root = Path(args.levels_root) if args.levels_root else campaign.parent
        errors.extend(validate_campaign_catalogue(campaign, levels_root))

    if args.json:
        print(json.dumps({"valid": not errors, "errors": errors}, indent=2))
    else:
        if errors:
            for error in errors:
                print(f"ERROR: {error}")
        else:
            target = catalogue if catalogue.is_file() and args.campaign is None else Path(args.campaign) if args.campaign else DEFAULT_CAMPAIGN
            print(f"OK: campaign catalogue and level-file structure are valid ({target})")

    return 1 if errors else 0


if __name__ == "__main__":
    raise SystemExit(main())
