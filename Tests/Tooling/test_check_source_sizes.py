#!/usr/bin/env python3
"""Small standard-library tests for the source-size gate."""

from __future__ import annotations

import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from Development.Tools import check_source_sizes


def write_bytes(path: Path, size: int) -> None:
    path.write_bytes(b"x" * size)


def main() -> int:
    with tempfile.TemporaryDirectory() as tmp:
        path = Path(tmp) / "sample.cpp"

        write_bytes(path, check_source_sizes.WARNING_BYTES - 1)
        assert check_source_sizes.file_size_bytes(path) < check_source_sizes.WARNING_BYTES

        write_bytes(path, check_source_sizes.WARNING_BYTES)
        assert check_source_sizes.WARNING_BYTES <= check_source_sizes.file_size_bytes(path) < check_source_sizes.HARD_LIMIT_BYTES

        write_bytes(path, check_source_sizes.HARD_LIMIT_BYTES)
        assert check_source_sizes.file_size_bytes(path) >= check_source_sizes.HARD_LIMIT_BYTES

    print("source-size KiB policy tests: OK")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
