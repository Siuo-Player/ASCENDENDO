#!/usr/bin/env python3
"""Enforce physical source-file size limits for maintainability.

Only C/C++ source/header files are checked. Documentation, data files,
assets and scripts intentionally have no size limit here.

Policy (normative; see docs/CODE_SIZE.md):
  < 40 KiB       OK
  40–47.99 KiB   WARNING
  >= 48 KiB      ERROR (CI failure)

Line count is intentionally diagnostic only; it is not a gate criterion.
The root ``main.cpp`` is explicitly included because it is an architectural
composition unit and is subject to the same responsibility/cohesion review.
"""

from __future__ import annotations

import pathlib

WARNING_BYTES = 40 * 1024
HARD_LIMIT_BYTES = 48 * 1024
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
            if path.is_file() and path.suffix.lower() in EXTENSIONS:
                if not EXCLUDED_PARTS.intersection(path.parts):
                    files.add(path)

    for path in ROOT_FILES:
        if path.is_file() and path.suffix.lower() in EXTENSIONS:
            files.add(path)

    return sorted(files)


def file_size_bytes(path: pathlib.Path) -> int:
    """Return the physical UTF-8 file size used by the normative gate."""
    return path.stat().st_size


def line_count(path: pathlib.Path) -> int:
    """Return physical source lines as diagnostic information only."""
    return len(path.read_text(encoding="utf-8").splitlines())


def main() -> int:
    failed = False
    warned = False

    for path in iter_sources():
        size_bytes = file_size_bytes(path)
        size_kib = size_bytes / 1024.0
        lines = line_count(path)

        if size_bytes >= HARD_LIMIT_BYTES:
            failed = True
            print(
                f"ERROR   {size_kib:6.1f} KiB  {path} "
                f"(>= {HARD_LIMIT_BYTES / 1024:.0f} KiB; {lines} lines diagnostic)"
            )
        elif size_bytes >= WARNING_BYTES:
            warned = True
            print(
                f"WARNING {size_kib:6.1f} KiB  {path} "
                f"(>= {WARNING_BYTES / 1024:.0f} KiB; {lines} lines diagnostic)"
            )

    if not failed and not warned:
        print("Source-size check: OK")
    elif not failed:
        print("Source-size check: OK (warnings only)")
    else:
        print("Source-size check: FAILED — subdivide oversized code files.")

    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
