#!/usr/bin/env python3
"""
Deterministic movement/camera validation benchmark.

This is a small, dependency-free diagnostic benchmark. It mirrors the current
camera/viewport mathematics without requiring Vulkan or GLFW, so it can be run
on any development machine and used as a stable regression oracle.

It intentionally does not tune gameplay values or replace the C++ tests.
It only checks properties already established by the current contracts and
reports the numerical observations used to review future camera changes.

Usage:
    python3 Development/AI_Validation/movement_camera_benchmark.py
    python3 Development/AI_Validation/movement_camera_benchmark.py --json report.json
"""

from __future__ import annotations

import argparse
import json
import math
from dataclasses import dataclass
from typing import Callable

LOGICAL_WIDTH = 640.0
LOGICAL_HEIGHT = 360.0
TARGET_ASPECT = LOGICAL_WIDTH / LOGICAL_HEIGHT
FIXED_STEP = 1.0 / 60.0
CAMERA_SPEED = 5.0
VERTICAL_OFFSET = LOGICAL_HEIGHT * 0.35
EPSILON = 1e-5


@dataclass(frozen=True)
class LetterboxRect:
    offset_x: int
    offset_y: int
    width: int
    height: int


@dataclass(frozen=True)
class ScenarioResult:
    name: str
    passed: bool
    observations: dict[str, float | int | bool | str]


def compute_letterbox(window_width: int, window_height: int, target_aspect: float) -> LetterboxRect:
    if window_width <= 0 or window_height <= 0 or target_aspect <= 0.0:
        return LetterboxRect(0, 0, 0, 0)

    window_aspect = float(window_width) / float(window_height)
    view_width = window_width
    view_height = window_height

    if window_aspect > target_aspect:
        view_width = int(window_height * target_aspect)
    else:
        view_height = int(window_width / target_aspect)

    return LetterboxRect(
        (window_width - view_width) // 2,
        (window_height - view_height) // 2,
        view_width,
        view_height,
    )


def world_to_ndc(world_x: float, world_y: float, camera_x: float, camera_y: float) -> tuple[float, float]:
    screen_x = world_x - camera_x
    ndc_x = (screen_x / LOGICAL_WIDTH) * 2.0 - 1.0
    screen_y = world_y - camera_y
    ndc_y = -((screen_y / LOGICAL_HEIGHT) * 2.0 - 1.0)
    return ndc_x, ndc_y


def follow_y(position_y: float, target_y: float, dt: float, speed: float) -> float:
    alpha = max(0.0, min(1.0, speed * dt))
    return position_y + (target_y - position_y) * alpha


def run_scenario(name: str, check: Callable[[], tuple[bool, dict[str, float | int | bool | str]]]) -> ScenarioResult:
    passed, observations = check()
    return ScenarioResult(name=name, passed=passed, observations=observations)


def scenario_large_dt() -> tuple[bool, dict[str, float | int | bool | str]]:
    target = 400.0 - VERTICAL_OFFSET
    result = follow_y(0.0, target, 1.0, CAMERA_SPEED)
    passed = abs(result - target) <= EPSILON and result <= target + EPSILON
    return passed, {
        "start_y": 0.0,
        "target_y": target,
        "dt": 1.0,
        "speed": CAMERA_SPEED,
        "result_y": result,
        "overshoot": result > target + EPSILON,
    }


def scenario_fixed_step_tracking() -> tuple[bool, dict[str, float | int | bool | str]]:
    target = 400.0 - VERTICAL_OFFSET
    position = 0.0
    monotonic = True
    never_overshoots = True

    for _ in range(120):
        next_position = follow_y(position, target, FIXED_STEP, CAMERA_SPEED)
        monotonic &= next_position + EPSILON >= position
        never_overshoots &= next_position <= target + EPSILON
        position = next_position

    passed = monotonic and never_overshoots and position > target * 0.9
    return passed, {
        "ticks": 120,
        "dt": FIXED_STEP,
        "target_y": target,
        "final_y": position,
        "monotonic": monotonic,
        "never_overshoots": never_overshoots,
        "reached_90_percent": position > target * 0.9,
    }


def scenario_lower_bound() -> tuple[bool, dict[str, float | int | bool | str]]:
    target = max(0.0, 10.0 - VERTICAL_OFFSET)
    position = 50.0

    for _ in range(120):
        position = follow_y(position, target, FIXED_STEP, CAMERA_SPEED)
        position = max(0.0, position)

    passed = position >= 0.0 and position < 1.0
    return passed, {
        "start_y": 50.0,
        "target_y": target,
        "final_y": position,
        "at_or_above_zero": position >= 0.0,
        "converged_below_one": position < 1.0,
    }


def scenario_ndc() -> tuple[bool, dict[str, float | int | bool | str]]:
    origin = world_to_ndc(0.0, 0.0, 0.0, 0.0)
    top_right = world_to_ndc(LOGICAL_WIDTH, LOGICAL_HEIGHT, 0.0, 0.0)
    center = world_to_ndc(LOGICAL_WIDTH / 2.0, LOGICAL_HEIGHT / 2.0, 0.0, 0.0)
    passed = (
        origin == (-1.0, 1.0)
        and top_right == (1.0, -1.0)
        and all(abs(value) <= EPSILON for value in center)
    )
    return passed, {
        "origin_x": origin[0],
        "origin_y": origin[1],
        "top_right_x": top_right[0],
        "top_right_y": top_right[1],
        "center_x": center[0],
        "center_y": center[1],
    }


def scenario_letterbox() -> tuple[bool, dict[str, float | int | bool | str]]:
    cases = {
        "1280x720": compute_letterbox(1280, 720, TARGET_ASPECT),
        "2560x1080": compute_letterbox(2560, 1080, TARGET_ASPECT),
        "1080x1920": compute_letterbox(1080, 1920, TARGET_ASPECT),
    }
    passed = (
        cases["1280x720"] == LetterboxRect(0, 0, 1280, 720)
        and cases["2560x1080"] == LetterboxRect(320, 0, 1920, 1080)
        and cases["1080x1920"] == LetterboxRect(0, 656, 1080, 607)
    )
    observations: dict[str, float | int | bool | str] = {"passed_reference_cases": passed}
    for label, rect in cases.items():
        observations[f"{label}_offset_x"] = rect.offset_x
        observations[f"{label}_offset_y"] = rect.offset_y
        observations[f"{label}_width"] = rect.width
        observations[f"{label}_height"] = rect.height
    return passed, observations


def scenario_invalid_viewport() -> tuple[bool, dict[str, float | int | bool | str]]:
    zero_width = compute_letterbox(0, 720, TARGET_ASPECT)
    zero_height = compute_letterbox(1280, 0, TARGET_ASPECT)
    passed = zero_width == LetterboxRect(0, 0, 0, 0) and zero_height == LetterboxRect(0, 0, 0, 0)
    return passed, {
        "zero_width_empty": zero_width == LetterboxRect(0, 0, 0, 0),
        "zero_height_empty": zero_height == LetterboxRect(0, 0, 0, 0),
    }


def scenario_finite_inputs() -> tuple[bool, dict[str, float | int | bool | str]]:
    values = [0.0, 1.0, -1.0, 640.0, 360.0]
    all_finite = True
    for world_x in values:
        for world_y in values:
            ndc_x, ndc_y = world_to_ndc(world_x, world_y, 0.0, 0.0)
            all_finite &= math.isfinite(ndc_x) and math.isfinite(ndc_y)
    passed = all_finite
    return passed, {"checked_points": len(values) * len(values), "all_finite": all_finite}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--json", metavar="PATH", help="write the deterministic report as JSON")
    args = parser.parse_args()

    scenarios = [
        run_scenario("camera.follow large dt", scenario_large_dt),
        run_scenario("camera.follow fixed-step tracking", scenario_fixed_step_tracking),
        run_scenario("camera.follow lower bound", scenario_lower_bound),
        run_scenario("camera worldToNDC mapping", scenario_ndc),
        run_scenario("viewport letterbox reference cases", scenario_letterbox),
        run_scenario("viewport invalid dimensions", scenario_invalid_viewport),
        run_scenario("camera NDC finite reference grid", scenario_finite_inputs),
    ]

    report = {
        "benchmark": "movement-camera-v1",
        "logical_width": LOGICAL_WIDTH,
        "logical_height": LOGICAL_HEIGHT,
        "fixed_step": FIXED_STEP,
        "camera_speed": CAMERA_SPEED,
        "vertical_offset": VERTICAL_OFFSET,
        "scenarios": [
            {"name": item.name, "passed": item.passed, "observations": item.observations}
            for item in scenarios
        ],
        "passed": all(item.passed for item in scenarios),
    }

    for item in scenarios:
        print(f"[{ 'PASS' if item.passed else 'FAIL' }] {item.name}")
        for key, value in item.observations.items():
            print(f"       {key}: {value}")

    print(f"\nBenchmark result: {'PASS' if report['passed'] else 'FAIL'}")

    if args.json:
        with open(args.json, "w", encoding="utf-8") as handle:
            json.dump(report, handle, indent=2, sort_keys=True)
            handle.write("\n")
        print(f"Report: {args.json}")

    return 0 if report["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
