#!/usr/bin/env python3
"""Authoritative static campaign validation and mechanical difficulty report."""
from __future__ import annotations

import argparse
import json
import math
import os
import random
import sys
from dataclasses import asdict, dataclass

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
SIM_DIR = os.path.join(SCRIPT_DIR, "sim")
sys.path.insert(0, SIM_DIR)
from engine import LOGICAL_HEIGHT, LOGICAL_WIDTH, PLAYER_WIDTH, simulate_jump, simulate_jump_flag  # type: ignore

MAX_CHARGE_SAMPLES = 41
ROBUSTNESS_TRIALS = 80


@dataclass(frozen=True)
class Surface:
    x: float
    y: float
    w: float
    h: float

    @property
    def right(self) -> float:
        return self.x + self.w

    @property
    def top(self) -> float:
        return self.y + self.h


@dataclass(frozen=True)
class ParsedLevel:
    path: str
    name: str
    screen_count: int
    platforms: tuple[Surface, ...]
    flag: Surface | None
    spawn: tuple[float, float] | None


@dataclass(frozen=True)
class Transition:
    source: int
    target: int
    charge: float
    direction: int
    landing_x: float
    landing_y: float
    horizontal_margin: float
    charge_margin: float
    robustness: float


def _finite(value: float) -> bool:
    return math.isfinite(value)


def parse_level(path: str) -> ParsedLevel:
    platforms: list[Surface] = []
    flag: Surface | None = None
    spawn: tuple[float, float] | None = None
    screen_count = 1
    name = os.path.basename(path)

    with open(path, "r", encoding="utf-8") as handle:
        for line_no, raw in enumerate(handle, 1):
            line = raw.rstrip("\r\n").strip()
            if not line or line.startswith("#"):
                continue
            parts = line.split()
            directive = parts[0]

            if directive == "NAME":
                name = " ".join(parts[1:]) or name
                continue
            if directive == "SCREENS":
                if len(parts) != 2:
                    raise ValueError(f"{path}:{line_no}: invalid SCREENS directive")
                try:
                    screen_count = int(parts[1])
                except ValueError as exc:
                    raise ValueError(f"{path}:{line_no}: invalid screen count") from exc
                if screen_count < 1:
                    raise ValueError(f"{path}:{line_no}: screen count must be positive")
                continue
            if directive == "SPAWN":
                if len(parts) != 3:
                    raise ValueError(f"{path}:{line_no}: invalid SPAWN directive")
                try:
                    x, y = (float(v) for v in parts[1:])
                except ValueError as exc:
                    raise ValueError(f"{path}:{line_no}: non-numeric spawn") from exc
                if not all(_finite(v) for v in (x, y)):
                    raise ValueError(f"{path}:{line_no}: non-finite spawn")
                spawn = (x, y)
                continue
            if directive not in {"PLATFORM", "FLAG"} or len(parts) != 5:
                raise ValueError(f"{path}:{line_no}: invalid directive or arity")

            try:
                x, y, w, h = (float(v) for v in parts[1:])
            except ValueError as exc:
                raise ValueError(f"{path}:{line_no}: non-numeric geometry") from exc
            if not all(_finite(v) for v in (x, y, w, h)):
                raise ValueError(f"{path}:{line_no}: non-finite geometry")
            if w <= 0 or h <= 0:
                raise ValueError(f"{path}:{line_no}: non-positive geometry")
            level_height = screen_count * LOGICAL_HEIGHT
            if x < 0 or x + w > LOGICAL_WIDTH or y < 0 or y + h > level_height:
                raise ValueError(f"{path}:{line_no}: geometry outside {LOGICAL_WIDTH}x{level_height} level")

            surface = Surface(x, y, w, h)
            if directive == "FLAG":
                if flag is not None:
                    raise ValueError(f"{path}:{line_no}: multiple FLAG directives")
                flag = surface
            else:
                platforms.append(surface)

    level_height = screen_count * LOGICAL_HEIGHT
    if spawn is not None:
        x, y = spawn
        if x < 0 or x > LOGICAL_WIDTH or y < 0 or y > level_height:
            raise ValueError(f"{path}: SPAWN outside {LOGICAL_WIDTH}x{level_height} level")

    return ParsedLevel(path, name, screen_count, tuple(platforms), flag, spawn)


def _rect(surface: Surface) -> tuple[float, float, float, float]:
    return (surface.x, surface.y, surface.right, surface.top)


def _initial_surface(level: ParsedLevel) -> Surface:
    if level.spawn is not None:
        x, y = level.spawn
        return Surface(x, y - 1.0, PLAYER_WIDTH, 1.0)
    return Surface(320.0, -1.0, PLAYER_WIDTH, 1.0)


def _robustness(source: Surface, target: Surface, direction: int, charge: float) -> float:
    rng = random.Random(307)
    successes = 0
    start_x = source.x + source.w * 0.5 - PLAYER_WIDTH * 0.5
    for _ in range(ROBUSTNESS_TRIALS):
        noisy_charge = max(0.0, min(1.0, charge + rng.uniform(-0.08, 0.08)))
        noisy_x = start_x + rng.uniform(-8.0, 8.0)
        landed, fx, fy, _, _ = simulate_jump(
            noisy_x, source.top, direction, noisy_charge, [_rect(source), _rect(target)]
        )
        body_center = fx + PLAYER_WIDTH * 0.5
        if landed and abs(fy - target.top) <= 1.5 and target.x <= body_center <= target.right:
            successes += 1
    return successes / ROBUSTNESS_TRIALS


def _scan_transition(source: Surface, target: Surface) -> Transition | None:
    best: Transition | None = None
    for direction in (-1, 1):
        for index in range(MAX_CHARGE_SAMPLES):
            charge = index / (MAX_CHARGE_SAMPLES - 1)
            landed, fx, fy, _, _ = simulate_jump(
                source.x + source.w * 0.5 - PLAYER_WIDTH * 0.5,
                source.top,
                direction,
                charge,
                [_rect(source), _rect(target)],
            )
            if not landed or abs(fy - target.top) > 1.5:
                continue
            body_center = fx + PLAYER_WIDTH * 0.5
            if not (target.x <= body_center <= target.right):
                continue
            candidate = Transition(
                0,
                0,
                charge,
                direction,
                fx,
                fy,
                min(body_center - target.x, target.right - body_center),
                min(charge, 1.0 - charge),
                _robustness(source, target, direction, charge),
            )
            if best is None or (candidate.robustness, candidate.horizontal_margin) > (
                best.robustness,
                best.horizontal_margin,
            ):
                best = candidate
    return best


def _flag_robustness(
    source: Surface,
    flag: Surface,
    direction: int,
    charge: float,
    platforms: list[Surface],
) -> float:
    rng = random.Random(307)
    successes = 0
    start_x = source.x + source.w * 0.5 - PLAYER_WIDTH * 0.5
    for _ in range(ROBUSTNESS_TRIALS):
        noisy_charge = max(0.0, min(1.0, charge + rng.uniform(-0.08, 0.08)))
        noisy_x = start_x + rng.uniform(-8.0, 8.0)
        hit, _, _ = simulate_jump_flag(
            noisy_x,
            source.top,
            direction,
            noisy_charge,
            [_rect(platform) for platform in platforms],
            _rect(flag),
        )
        successes += int(hit)
    return successes / ROBUSTNESS_TRIALS


def _trace_path(
    start: int,
    target: int,
    came_from: dict[int, int],
    via: dict[int, Transition],
) -> list[Transition]:
    path: list[Transition] = []
    node = target
    while node != start:
        path.append(via[node])
        node = came_from[node]
    return list(reversed(path))


def _route(level: ParsedLevel) -> tuple[list[Transition], str]:
    initial = _initial_surface(level)
    surfaces = [initial, *level.platforms]
    start = 0
    came_from: dict[int, int] = {}
    via: dict[int, Transition] = {}
    queue = [start]
    seen = {start}

    while queue:
        current = queue.pop(0)
        source = surfaces[current]

        if level.flag is not None:
            for direction in (-1, 1):
                for index in range(MAX_CHARGE_SAMPLES):
                    charge = index / (MAX_CHARGE_SAMPLES - 1)
                    hit, _, _ = simulate_jump_flag(
                        source.x + source.w * 0.5 - PLAYER_WIDTH * 0.5,
                        source.top,
                        direction,
                        charge,
                        [_rect(platform) for platform in level.platforms],
                        _rect(level.flag),
                    )
                    if hit:
                        transition = Transition(
                            current,
                            -1,
                            charge,
                            direction,
                            0.0,
                            level.flag.y,
                            0.0,
                            min(charge, 1.0 - charge),
                            _flag_robustness(source, level.flag, direction, charge, list(level.platforms)),
                        )
                        return (
                            _trace_path(start, current, came_from, via) + [transition],
                            "spawn_to_flag",
                        )

        for target_index, target in enumerate(level.platforms, 1):
            if target_index in seen:
                continue
            transition = _scan_transition(source, target)
            if transition is None:
                continue
            transition = Transition(
                current,
                target_index,
                transition.charge,
                transition.direction,
                transition.landing_x,
                transition.landing_y,
                transition.horizontal_margin,
                transition.charge_margin,
                transition.robustness,
            )
            seen.add(target_index)
            came_from[target_index] = current
            via[target_index] = transition
            queue.append(target_index)

    if not level.platforms:
        return [], "no_platforms"

    highest = max(range(1, len(surfaces)), key=lambda index: surfaces[index].top)
    if highest in seen:
        return _trace_path(start, highest, came_from, via), "top_reached"
    return [], "highest_platform_unreachable"


def difficulty_score(transitions: list[Transition]) -> dict[str, float | str | int]:
    if not transitions:
        return {"rating": "unreachable", "score": 100.0, "transitions": 0}
    robustness = sum(t.robustness for t in transitions) / len(transitions)
    margin = sum(max(0.0, min(1.0, t.horizontal_margin / 48.0)) for t in transitions) / len(transitions)
    charge = sum(max(0.0, min(1.0, t.charge_margin / 0.25)) for t in transitions) / len(transitions)
    safety = 0.45 * robustness + 0.35 * margin + 0.20 * charge
    score = 100.0 * (1.0 - safety)
    if score < 20:
        rating = "tutorial"
    elif score < 40:
        rating = "easy"
    elif score < 60:
        rating = "medium"
    elif score < 80:
        rating = "hard"
    else:
        rating = "extreme"
    return {
        "rating": rating,
        "score": round(score, 2),
        "transitions": len(transitions),
        "mean_robustness": round(robustness, 3),
        "minimum_robustness": round(min(t.robustness for t in transitions), 3),
        "mean_horizontal_margin_px": round(
            sum(t.horizontal_margin for t in transitions) / len(transitions), 2
        ),
    }


def validate_level(path: str, require_flag: bool = False, forbid_flag: bool = False) -> dict:
    level = parse_level(path)
    errors: list[str] = []
    if not level.platforms:
        errors.append("no platforms")
    if require_flag and level.flag is None:
        errors.append("missing final FLAG")
    if forbid_flag and level.flag is not None:
        errors.append("non-final level must not contain FLAG")

    transitions, reason = _route(level)
    if not transitions:
        errors.append(f"map is not mechanically reachable ({reason})")

    report = {
        "path": os.path.normpath(path),
        "name": level.name,
        "screens": level.screen_count,
        "spawn": level.spawn,
        "platforms": len(level.platforms),
        "flag": asdict(level.flag) if level.flag else None,
        "valid": not errors,
        "errors": errors,
        "route": [asdict(transition) for transition in transitions],
        "difficulty": difficulty_score(transitions),
    }
    return report


def validate_campaign(campaign_path: str) -> list[dict]:
    base = os.path.dirname(campaign_path)
    with open(campaign_path, "r", encoding="utf-8") as handle:
        names = [line.strip() for line in handle if line.strip() and not line.startswith("#")]
    if not names:
        raise ValueError("campaign contains no levels")

    reports: list[dict] = []
    seen: set[str] = set()
    for index, name in enumerate(names):
        normalized = os.path.normpath(name)
        if normalized in seen:
            raise ValueError(f"campaign contains duplicate level: {name}")
        seen.add(normalized)
        path = os.path.join(base, name)
        if not os.path.isfile(path):
            raise ValueError(f"campaign references missing level: {name}")
        reports.append(
            validate_level(
                path,
                require_flag=(index == len(names) - 1),
                forbid_flag=(index != len(names) - 1),
            )
        )
    return reports


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--campaign", action="store_true")
    parser.add_argument("--level")
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()
    root = os.path.normpath(os.path.join(SCRIPT_DIR, "..", ".."))
    campaign = os.path.join(root, "Game", "Assets", "Levels", "campaign.txt")
    reports = validate_campaign(campaign) if args.campaign else [validate_level(args.level)]
    ok = all(report["valid"] for report in reports)
    if args.json:
        print(json.dumps(reports, indent=2, sort_keys=True))
    else:
        for report in reports:
            difficulty = report["difficulty"]
            status = "OK" if report["valid"] else "FAIL"
            print(f"[{status}] {report['name']}: {difficulty['rating']} ({difficulty['score']})")
            for error in report["errors"]:
                print(f"  - {error}")
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
