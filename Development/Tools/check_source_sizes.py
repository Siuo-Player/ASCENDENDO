#!/usr/bin/env python3
"""Enforce source-size and source-length limits for maintainability.

The limits are engineering guardrails, not claims that one LOC threshold
universally defines good design. Large files are treated as refactoring
signals and are evaluated together with cohesion, coupling and responsibility.

Policy:
  < 30 KiB and < 300 lines  OK
  30–36 KiB or 300–399 lines WARNING
  > 36 KiB or >= 400 lines ERROR (CI failure)

The goal is to prevent application entry points and domain/presentation
components from becoming oversized, multi-responsibility units. When a limit
is reached, split by cohesive responsibility instead of merely moving random
functions between files.
"""

from __future__ import annotations

import pathlib

WARNING_BYTES = 30 * 1024
HARD_LIMIT_BYTES = 36 * 1024
WARNING_LINES = 300
HARD_LIMIT_LINES = 400
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
        line_count = sum(1 for _ in path.open("r", encoding="utf-8", errors="replace"))
        kib = size / 1024.0

        if size > HARD_LIMIT_BYTES or line_count >= HARD_LIMIT_LINES:
            failed = True
            reasons = []
            if size > HARD_LIMIT_BYTES:
                reasons.append(f"> 36 KiB")
            if line_count >= HARD_LIMIT_LINES:
                reasons.append(f">= {HARD_LIMIT_LINES} lines")
            print(f"ERROR   {kib:7.2f} KiB  {line_count:4d} lines  {path} ({'; '.join(reasons)})")
        elif size >= WARNING_BYTES or line_count >= WARNING_LINES:
            warned = True
            reasons = []
            if size >= WARNING_BYTES:
                reasons.append(">= 30 KiB")
            if line_count >= WARNING_LINES:
                reasons.append(f">= {WARNING_LINES} lines")
            print(f"WARNING {kib:7.2f} KiB  {line_count:4d} lines  {path} ({'; '.join(reasons)}; plan a split)")

    if not failed and not warned:
        print("Source size check: OK")
    elif not failed:
        print("Source size check: OK (warnings only)")
    else:
        print("Source size check: FAILED — subdivide oversized code files by cohesive responsibility.")

    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
