#!/usr/bin/env python3
"""Deterministic visual stress-scene fixtures for ASCENDENDO.

This utility does not render images. It defines stable scene fixtures and viewport
metadata that a future renderer capture path can consume without embedding
subjective visual judgements in unit tests.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from dataclasses import asdict, dataclass
from pathlib import Path

LOGICAL_WIDTH = 640
LOGICAL_HEIGHT = 360
TARGET_ASPECT = LOGICAL_WIDTH / LOGICAL_HEIGHT

VIEWPORTS = (
    (1152, 648),   # 16:9
    (800, 600),    # 4:3
    (1680, 720),   # 21:9
)

LEVELS = {
    "inicio": "Game/Assets/Levels/inicio.lvl",
    "zigzag": "Game/Assets/Levels/zigzag.lvl",
    "precipicio": "Game/Assets/Levels/precipicio.lvl",
}


@dataclass(frozen=True)
class Rect:
    x: float
    y: float
    width: float
    height: float


@dataclass(frozen=True)
class Scene:
    scene_id: str
    purpose: str
    level: str
    camera_x: float
    camera_y: float
    player: Rect
    platform_indices: tuple[int, ...]
    flag: Rect | None = None


@dataclass(frozen=True)
class ViewportCapture:
    scene_id: str
    width: int
    height: int
    aspect_label: str
    logical_width: int
    logical_height: int
    viewport_x: int
    viewport_y: int
    viewport_width: int
    viewport_height: int
    camera_x: float
    camera_y: float
    player_bounds: Rect
    visible_platform_bounds: tuple[Rect, ...]
    flag_bounds: Rect | None


def viewport_geometry(width: int, height: int) -> tuple[int, int, int, int]:
    if width <= 0 or height <= 0:
        raise ValueError("viewport dimensions must be positive")

    aspect = width / height
    if aspect > TARGET_ASPECT:
        vp_h = height
        vp_w = int(height * TARGET_ASPECT)
    else:
        vp_w = width
        vp_h = int(width / TARGET_ASPECT)

    offset_x = (width - vp_w) // 2
    offset_y = (height - vp_h) // 2
    return offset_x, offset_y, vp_w, vp_h


def parse_level(path: Path) -> tuple[list[Rect], Rect | None]:
    platforms: list[Rect] = []
    flag: Rect | None = None
    for raw in path.read_text(encoding="utf-8").splitlines():
        fields = raw.split()
        if not fields or fields[0].startswith("#") or fields[0] == "NAME":
            continue
        if fields[0] == "PLATFORM" and len(fields) == 5:
            platforms.append(Rect(*(float(v) for v in fields[1:])))
        elif fields[0] == "FLAG" and len(fields) == 5:
            flag = Rect(*(float(v) for v in fields[1:]))
    return platforms, flag


def build_scenes(repo_root: Path) -> tuple[Scene, ...]:
    levels = {name: parse_level(repo_root / rel) for name, rel in LEVELS.items()}
    inicio, _ = levels["inicio"]
    zigzag, _ = levels["zigzag"]
    precipicio, precip_flag = levels["precipicio"]

    return (
        Scene(
            "A-baseline",
            "baseline player/platform contrast",
            "inicio",
            camera_x=0.0,
            camera_y=0.0,
            player=Rect(312.0, 40.0, 16.0, 16.0),
            platform_indices=(0, 1, 2, 3),
        ),
        Scene(
            "B-dense-vertical",
            "stacked reachable platforms during upward camera tracking",
            "zigzag",
            camera_x=0.0,
            camera_y=85.0,
            player=Rect(418.0, 105.0, 16.0, 16.0),
            platform_indices=(1, 2, 3),
        ),
        Scene(
            "C-viewport-edge",
            "player and next landing near logical horizontal boundary",
            "inicio",
            camera_x=0.0,
            camera_y=170.0,
            player=Rect(600.0, 175.0, 16.0, 16.0),
            platform_indices=(1, 2, 3),
        ),
        Scene(
            "D-goal-salience",
            "goal and nearby landing geometry",
            "precipicio",
            camera_x=0.0,
            camera_y=170.0,
            player=Rect(405.0, 205.0, 16.0, 16.0),
            platform_indices=(1, 2, 3),
            flag=precip_flag,
        ),
        Scene(
            "E-prop-density",
            "stable gameplay geometry reserved for future prop/background composition",
            "zigzag",
            camera_x=0.0,
            camera_y=85.0,
            player=Rect(418.0, 105.0, 16.0, 16.0),
            platform_indices=(1, 2, 3),
        ),
    )


def label_for(width: int, height: int) -> str:
    ratio = round(width / height, 5)
    if ratio == round(16 / 9, 5):
        return "16:9"
    if ratio == round(4 / 3, 5):
        return "4:3"
    if ratio == round(21 / 9, 5):
        return "21:9"
    return f"{width}:{height}"


def build_manifest(repo_root: Path, commit_sha: str) -> dict:
    scenes = build_scenes(repo_root)
    level_data = {name: parse_level(repo_root / rel) for name, rel in LEVELS.items()}
    captures: list[ViewportCapture] = []

    for scene in scenes:
        platforms, parsed_flag = level_data[scene.level]
        scene_flag = scene.flag if scene.flag is not None else parsed_flag
        for width, height in VIEWPORTS:
            vx, vy, vw, vh = viewport_geometry(width, height)
            selected = tuple(platforms[i] for i in scene.platform_indices)
            captures.append(
                ViewportCapture(
                    scene_id=scene.scene_id,
                    width=width,
                    height=height,
                    aspect_label=label_for(width, height),
                    logical_width=LOGICAL_WIDTH,
                    logical_height=LOGICAL_HEIGHT,
                    viewport_x=vx,
                    viewport_y=vy,
                    viewport_width=vw,
                    viewport_height=vh,
                    camera_x=scene.camera_x,
                    camera_y=scene.camera_y,
                    player_bounds=scene.player,
                    visible_platform_bounds=selected,
                    flag_bounds=scene_flag,
                )
            )

    return {
        "schema": "ascendendo.visual-stress.v1",
        "commit_sha": commit_sha,
        "logical_world": {"width": LOGICAL_WIDTH, "height": LOGICAL_HEIGHT},
        "scenes": [asdict(scene) for scene in scenes],
        "captures": [asdict(capture) for capture in captures],
        "properties_not_proven_by_this_manifest": [
            "subjective readability",
            "final art/style quality",
            "ideal camera feel",
            "player preference",
        ],
    }


def validate_manifest(manifest: dict) -> None:
    expected_captures = len(manifest["scenes"]) * len(VIEWPORTS)
    if len(manifest["captures"]) != expected_captures:
        raise AssertionError("unexpected capture count")

    seen = {(item["scene_id"], item["width"], item["height"]) for item in manifest["captures"]}
    if len(seen) != expected_captures:
        raise AssertionError("capture identifiers are not unique")

    for item in manifest["captures"]:
        left = item["viewport_x"]
        top = item["viewport_y"]
        right = left + item["viewport_width"]
        bottom = top + item["viewport_height"]
        if left < 0 or top < 0 or right > item["width"] or bottom > item["height"]:
            raise AssertionError("letterboxed viewport exceeds window bounds")
        if item["logical_width"] != LOGICAL_WIDTH or item["logical_height"] != LOGICAL_HEIGHT:
            raise AssertionError("logical world changed unexpectedly")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo-root", type=Path, default=Path(__file__).resolve().parents[2])
    parser.add_argument("--commit", default=os.environ.get("GITHUB_SHA", "UNKNOWN"))
    parser.add_argument("--json", type=Path, help="write deterministic manifest to this file")
    args = parser.parse_args()

    manifest = build_manifest(args.repo_root, args.commit)
    validate_manifest(manifest)

    text = json.dumps(manifest, indent=2, sort_keys=True) + "\n"
    if args.json:
        args.json.write_text(text, encoding="utf-8")
    else:
        print(text, end="")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
