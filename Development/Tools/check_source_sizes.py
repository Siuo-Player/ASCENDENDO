#!/usr/bin/env python3
"""Enforce source-file size limits for maintainability.

Only C/C++ source/header files are checked. Documentation, data files,
assets and scripts intentionally have no size limit here.

Policy:
  < 30 KiB  OK
  30–36 KiB WARNING
  > 36 KiB  ERROR (CI failure)
"""

from __future__ import annotations

import pathlib
import sys

WARNING_BYTES = 30 * 1024
HARD_LIMIT_BYTES = 36 * 1024
EXTENSIONS = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx"}
EXCLUDED_PARTS = {".git", "build", "dist"}
ROOTS = (pathlib.Path("Game"), pathlib.Path("Tests"))


def iter_sources() -> list[pathlib.Path]:
    files: list[pathlib.Path] = []
    for root in ROOTS:
        if not root.exists():
            continue
        for path in root.rglob("*"):
            if path.is_file() and path.suffix.lower() in EXTENSIONS:
                if not EXCLUDED_PARTS.intersection(path.parts):
                    files.append(path)
    return sorted(files)


def main() -> int:
    failed = False
    warned = False

    for path in iter_sources():
        size = path.stat().st_size
        kib = size / 1024.0
        if size > HARD_LIMIT_BYTES:
            failed = True
            print(f"ERROR   {kib:7.2f} KiB  {path} (> 36 KiB)")
        elif size >= WARNING_BYTES:
            warned = True
            print(f"WARNING {kib:7.2f} KiB  {path} (>= 30 KiB; plan a split)")

    if not failed and not warned:
        print("Source size check: OK")
    elif not failed:
        print("Source size check: OK (warnings only)")
    else:
        print("Source size check: FAILED — subdivide oversized code files.")

    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
