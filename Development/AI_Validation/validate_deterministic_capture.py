#!/usr/bin/env python3
"""Run one deterministic capture and validate the resulting binary PPM."""

from __future__ import annotations

import argparse
import hashlib
import os
from pathlib import Path
import subprocess
import sys


def read_ppm(path: Path) -> tuple[int, int, bytes]:
    data = path.read_bytes()
    if not data.startswith(b"P6\n"):
        raise ValueError("PPM header is not P6")

    pos = 3
    tokens: list[bytes] = []
    while len(tokens) < 3:
        while pos < len(data) and data[pos] in b" \t\r\n":
            pos += 1
        if pos >= len(data):
            raise ValueError("truncated PPM header")
        if data[pos] == ord("#"):
            newline = data.find(b"\n", pos)
            if newline < 0:
                raise ValueError("unterminated PPM comment")
            pos = newline + 1
            continue
        end = pos
        while end < len(data) and data[end] not in b" \t\r\n":
            end += 1
        tokens.append(data[pos:end])
        pos = end

    width, height = int(tokens[0]), int(tokens[1])
    max_value = int(tokens[2])
    if width <= 0 or height <= 0 or max_value != 255:
        raise ValueError("invalid PPM dimensions/max value")

    if pos >= len(data) or data[pos] not in b" \t\r\n":
        raise ValueError("missing PPM header/pixel separator")
    if data[pos : pos + 2] == b"\r\n":
        pos += 2
    else:
        pos += 1

    pixels = data[pos:]
    expected = width * height * 3
    if len(pixels) != expected:
        raise ValueError(f"PPM pixel payload has {len(pixels)} bytes; expected {expected}")
    return width, height, pixels


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--binary", type=Path, required=True)
    parser.add_argument("--level", type=int, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--width", type=int, required=True)
    parser.add_argument("--height", type=int, required=True)
    args = parser.parse_args()

    if args.level < 0:
        parser.error("--level must be non-negative")
    if args.width <= 0 or args.height <= 0:
        parser.error("--width and --height must be positive")
    if not args.binary.is_file():
        parser.error(f"game binary not found: {args.binary}")

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.unlink(missing_ok=True)

    env = os.environ.copy()
    env["ASCENDENDO_CAPTURE_LEVEL_INDEX"] = str(args.level)
    env["ASCENDENDO_CAPTURE_WINDOW_WIDTH"] = str(args.width)
    env["ASCENDENDO_CAPTURE_WINDOW_HEIGHT"] = str(args.height)
    env["ASCENDENDO_CAPTURE_PPM"] = str(args.output.resolve())
    env.setdefault("LIBGL_ALWAYS_SOFTWARE", "1")
    env.setdefault("GALLIUM_DRIVER", "llvmpipe")
    env.setdefault("VK_LOADER_DEBUG", "error")

    print(f"[capture] level={args.level} window={args.width}x{args.height} output={args.output}")
    completed = subprocess.run(
        [str(args.binary)],
        env=env,
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    )
    sys.stdout.write(completed.stdout)
    if completed.returncode != 0:
        print(f"[capture] game exited with {completed.returncode}", file=sys.stderr)
        return completed.returncode
    if not args.output.is_file():
        print("[capture] expected PPM was not created", file=sys.stderr)
        return 2

    try:
        width, height, pixels = read_ppm(args.output)
    except (OSError, ValueError) as exc:
        print(f"[capture] invalid PPM: {exc}", file=sys.stderr)
        return 3

    if (width, height) != (args.width, args.height):
        print(
            f"[capture] dimensions {width}x{height} != expected {args.width}x{args.height}",
            file=sys.stderr,
        )
        return 4

    first = pixels[:3]
    has_non_uniform_pixel = any(pixels[i : i + 3] != first for i in range(3, len(pixels), 3))
    if not has_non_uniform_pixel:
        print("[capture] image is fully uniform; no rendered content evidence", file=sys.stderr)
        return 5

    checksum = hashlib.sha256(pixels).hexdigest()
    print(f"[capture] OK {width}x{height}, sha256(pixel-data)={checksum}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
