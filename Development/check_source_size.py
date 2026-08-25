#!/usr/bin/env python3
"""Enforce source-file size limits for ASCENDENDO.

Hard limit: 48 KiB.
Warning threshold: 40 KiB.

The check intentionally covers source and test code only. Build artefacts,
assets, documentation and generated files are not subject to this rule.
"""

from __future__ import annotations

from pathlib import Path

WARNING_BYTES = 40 * 1024
HARD_LIMIT_BYTES = 48 * 1024

ROOTS = (Path("Game"), Path("Tests"))
ROOT_FILES = (Path("main.cpp"),)
EXTENSIONS = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx"}
EXCLUDED_PARTS = {"build", ".git"}


def iter_sources() -> list[Path]:
    files: list[Path] = []
    for root in ROOTS:
        if not root.exists():
            continue
        files.extend(
            p for p in root.rglob("*")
            if p.is_file()
            and p.suffix.lower() in EXTENSIONS
            and not any(part in EXCLUDED_PARTS for part in p.parts)
        )
    files.extend(p for p in ROOT_FILES if p.exists())
    return sorted(set(files))


def main() -> int:
    warnings = 0
    failures = 0

    for path in iter_sources():
        size = path.stat().st_size
        size_kib = size / 1024.0

        if size > HARD_LIMIT_BYTES:
            failures += 1
            print(f"[ERROR] {path}: {size_kib:.1f} KiB > {HARD_LIMIT_BYTES / 1024:.0f} KiB hard limit")
        elif size >= WARNING_BYTES:
            warnings += 1
            print(f"[WARN]  {path}: {size_kib:.1f} KiB >= {WARNING_BYTES / 1024:.0f} KiB warning threshold")

    if failures:
        print(f"\nSource-size gate FAILED: {failures} file(s) exceed the hard limit.")
        return 1

    if warnings:
        print(f"\nSource-size gate passed with {warnings} warning(s). Subdivision should be planned before the hard limit.")
    else:
        print("Source-size gate passed: no source file reached the warning threshold.")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
