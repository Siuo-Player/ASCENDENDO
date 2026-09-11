#!/usr/bin/env python3
"""Check that small, user-facing state claims agree with repository data."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
CAMPAIGN = ROOT / "Game" / "Assets" / "Levels" / "campaign.txt"
README = ROOT / "README.md"
ROADMAP = ROOT / "docs" / "ROADMAP.md"


def campaign_entries() -> list[str]:
    return [
        line.strip()
        for line in CAMPAIGN.read_text(encoding="utf-8").splitlines()
        if line.strip() and not line.lstrip().startswith("#")
    ]


def documented_campaign_count(path: Path) -> int:
    text = path.read_text(encoding="utf-8")
    match = re.search(r"campanha[^\n]*?\*\*(\d+) níveis\*\*", text, re.IGNORECASE)
    if match is None:
        raise ValueError(f"{path}: could not find canonical campaign count")
    return int(match.group(1))


def validate() -> list[str]:
    errors: list[str] = []
    entries = campaign_entries()
    if not entries:
        errors.append("campaign.txt contains no playable entries")
        return errors

    actual = len(entries)
    for document in (README, ROADMAP):
        try:
            documented = documented_campaign_count(document)
        except ValueError as error:
            errors.append(str(error))
            continue
        if documented != actual:
            errors.append(
                f"{document}: documents {documented} levels, campaign.txt contains {actual}"
            )

    duplicates = sorted({entry for entry in entries if entries.count(entry) > 1})
    if duplicates:
        errors.append(f"campaign.txt contains duplicate entries: {', '.join(duplicates)}")
    return errors


def main() -> int:
    errors = validate()
    if errors:
        for error in errors:
            print(f"[ERROR] {error}")
        return 1
    count = len(campaign_entries())
    print(f"[OK] campaign state is consistent across catalogue, README and ROADMAP ({count} levels)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
