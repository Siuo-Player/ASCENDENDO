#!/usr/bin/env python3
"""Enforce the repository's line-based source-size policy.

Only C/C++ source/header files are checked. Documentation, data files,
assets and scripts intentionally have no size limit here.

Policy:
  < 300 lines   OK
  300-399       WARNING
  >= 400        ERROR (CI failure)

The check covers the game/test trees plus the root-level game entry point.
"""

from __future__ import annotations

import pathlib
import sys

WARNING_LINES = 300
HARD_LIMIT_LINES = 400
EXTENSIONS = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx"}
EXCLUDED_PARTS = {".git", "build", "dist"}
ROOTS = (pathlib.Path("Game"), pathlib.Path("Tests"))
ROOT_FILES = (pathlib.Path("main.cpp"),)


def iter_sources() -> list[pathlib.Path]:
    files: set[pathlib.Path] = set()

    for root in ROOTS:
        if not root.exists():
            continue
        for path in root.rglob("*"):
            if (
                path.is_file()
                and path.suffix.lower() in EXTENSIONS
                and not EXCLUDED_PARTS.intersection(path.parts)
            ):
                files.add(path)

    for path in ROOT_FILES:
        if path.is_file() and path.suffix.lower() in EXTENSIONS:
            files.add(path)

    return sorted(files)


def line_count(path: pathlib.Path) -> int:
    try:
        return len(path.read_text(encoding="utf-8").splitlines())
    except UnicodeDecodeError as exc:
        print(f"ERROR   unreadable UTF-8 source file: {path}: {exc}")
        raise


def main() -> int:
    failed = False
    warned = False

    for path in iter_sources():
        lines = line_count(path)
        if lines >= HARD_LIMIT_LINES:
            failed = True
            print(f"ERROR   {lines:4d} lines  {path} (>= {HARD_LIMIT_LINES})")
        elif lines >= WARNING_LINES:
            warned = True
            print(f"WARNING {lines:4d} lines  {path} (>= {WARNING_LINES}; plan a split)")

    if not failed and not warned:
        print("Source size check: OK")
    elif not failed:
        print("Source size check: OK (warnings only)")
    else:
        print("Source size check: FAILED — subdivide oversized code files.")

    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
