#!/usr/bin/env python3
"""Authoritative mechanical campaign validation and deterministic difficulty report."""
from __future__ import annotations

import argparse
import json
import math
import os
import random
import sys
from dataclasses import dataclass

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(SCRIPT_DIR, "sim"))
from engine import LOGICAL_HEIGHT, LOGICAL_WIDTH, PLAYER_WIDTH, simulate_jump, simulate_jump_flag  # type: ignore

CHARGE_SAMPLES = 41
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
class Level:
    path: str
    name: str
    screens: int
    platforms: tuple[Surface, ...]
    flag: Surface | None
    spawn: tuple[float, float] | None


def rect(s: Surface) -> tuple[float, float, float, float]:
    return s.x, s.y, s.right, s.top


def parse_level(path: str) -> Level:
    platforms: list[Surface] = []
    flag = None
    spawn = None
    screens = 1
    name = os.path.basename(path)

    with open(path, encoding="utf-8") as stream:
        for number, raw in enumerate(stream, 1):
            line = raw.strip()
            if not line or line.startswith("#"):
                continue
            fields = line.split()
            directive = fields[0]
            if directive == "NAME":
                name = " ".join(fields[1:]) or name
                continue
            if directive == "SCREENS":
                if len(fields) != 2:
                    raise ValueError(f"{path}:{number}: invalid SCREENS")
                screens = int(fields[1])
                if screens < 1:
                    raise ValueError(f"{path}:{number}: screens must be positive")
                continue
            if directive == "SPAWN":
                if len(fields) != 3:
                    raise ValueError(f"{path}:{number}: invalid SPAWN")
                spawn = (float(fields[1]), float(fields[2]))
                if not all(math.isfinite(value) for value in spawn):
                    raise ValueError(f"{path}:{number}: non-finite spawn")
                continue
            if directive not in {"PLATFORM", "FLAG"} or len(fields) != 5:
                raise ValueError(f"{path}:{number}: invalid directive")
            values = tuple(float(value) for value in fields[1:])
            if not all(math.isfinite(value) for value in values):
                raise ValueError(f"{path}:{number}: non-finite geometry")
            x, y, width, height = values
            if width <= 0 or height <= 0:
                raise ValueError(f"{path}:{number}: non-positive geometry")
            level_height = screens * LOGICAL_HEIGHT
            if x < 0 or x + width > LOGICAL_WIDTH or y < 0 or y + height > level_height:
                raise ValueError(f"{path}:{number}: geometry outside {LOGICAL_WIDTH}x{level_height}")
            surface = Surface(x, y, width, height)
            if directive == "FLAG":
                if flag is not None:
                    raise ValueError(f"{path}:{number}: multiple FLAG directives")
                flag = surface
            else:
                platforms.append(surface)

    if spawn is not None:
        sx, sy = spawn
        if not (0 <= sx <= LOGICAL_WIDTH and 0 <= sy <= screens * LOGICAL_HEIGHT):
            raise ValueError(f"{path}: spawn outside level bounds")
    return Level(path, name, screens, tuple(platforms), flag, spawn)


def initial_surface(level: Level) -> Surface:
    x = level.spawn[0] if level.spawn is not None else 320.0
    y = level.spawn[1] if level.spawn is not None else 0.0
    return Surface(x, y - 1.0, PLAYER_WIDTH, 1.0)


def landing_transition(source: Surface, target: Surface):
    best = None
    for direction in (-1, 1):
        for index in range(CHARGE_SAMPLES):
            charge = index / (CHARGE_SAMPLES - 1)
            landed, fx, fy, _, _ = simulate_jump(
                source.x + source.w * 0.5 - PLAYER_WIDTH * 0.5,
                source.top,
                direction,
                charge,
                [rect(source), rect(target)],
            )
            if not landed or abs(fy - target.top) > 1.5:
                continue
            center = fx + PLAYER_WIDTH * 0.5
            if not target.x <= center <= target.right:
                continue
            rng = random.Random(307)
            successes = 0
            start_x = source.x + source.w * 0.5 - PLAYER_WIDTH * 0.5
            for _ in range(ROBUSTNESS_TRIALS):
                noisy_charge = max(0.0, min(1.0, charge + rng.uniform(-0.08, 0.08)))
                noisy_x = start_x + rng.uniform(-8.0, 8.0)
                ok, nx, ny, _, _ = simulate_jump(noisy_x, source.top, direction, noisy_charge, [rect(source), rect(target)])
                ncenter = nx + PLAYER_WIDTH * 0.5
                successes += int(ok and abs(ny - target.top) <= 1.5 and target.x <= ncenter <= target.right)
            candidate = (successes / ROBUSTNESS_TRIALS, direction, charge, min(center - target.x, target.right - center), min(charge, 1.0 - charge))
            if best is None or candidate > best:
                best = candidate
    return best


def flag_robustness(source: Surface, level: Level) -> float:
    assert level.flag is not None
    rng = random.Random(307)
    successes = 0
    start_x = source.x + source.w * 0.5 - PLAYER_WIDTH * 0.5
    for _ in range(ROBUSTNESS_TRIALS):
        direction = rng.choice((-1, 1))
        noisy_charge = max(0.0, min(1.0, rng.random() + rng.uniform(-0.08, 0.08)))
        noisy_x = start_x + rng.uniform(-8.0, 8.0)
        hit, _, _ = simulate_jump_flag(noisy_x, source.top, direction, noisy_charge, [rect(p) for p in level.platforms], rect(level.flag))
        successes += int(hit)
    return successes / ROBUSTNESS_TRIALS


def flag_transition(source: Surface, level: Level):
    if level.flag is None:
        return None
    for direction in (-1, 1):
        for index in range(CHARGE_SAMPLES):
            charge = index / (CHARGE_SAMPLES - 1)
            hit, _, _ = simulate_jump_flag(
                source.x + source.w * 0.5 - PLAYER_WIDTH * 0.5,
                source.top,
                direction,
                charge,
                [rect(platform) for platform in level.platforms],
                rect(level.flag),
            )
            if hit:
                return charge, direction, flag_robustness(source, level)
    return None


def route(level: Level):
    surfaces = [initial_surface(level), *level.platforms]
    queue = [0]
    seen = {0}
    transitions = 0
    robustness = 1.0
    min_margin = float("inf")
    min_charge_margin = float("inf")

    while queue:
        source = surfaces[queue.pop(0)]
        if level.flag is not None:
            hit = flag_transition(source, level)
            if hit is not None:
                charge, _, flag_robust = hit
                return transitions + 1, min(robustness, flag_robust), min_margin if min_margin != float("inf") else 0.0, min_charge_margin if min_charge_margin != float("inf") else min(charge, 1.0 - charge), "spawn_to_flag"
        for target_index, target in enumerate(level.platforms, 1):
            if target_index in seen:
                continue
            transition = landing_transition(source, target)
            if transition is None:
                continue
            robustness_value, _, _, margin, charge_margin = transition
            seen.add(target_index)
            queue.append(target_index)
            transitions += 1
            robustness = min(robustness, robustness_value)
            min_margin = min(min_margin, margin)
            min_charge_margin = min(min_charge_margin, charge_margin)

    if not level.platforms:
        return 0, 0.0, 0.0, 0.0, "no_platforms"
    highest = max(range(1, len(surfaces)), key=lambda index: surfaces[index].top)
    if highest in seen:
        return transitions, robustness, min_margin if min_margin != float("inf") else 0.0, min_charge_margin if min_charge_margin != float("inf") else 0.0, "top_reached"
    return transitions, 0.0, 0.0, 0.0, "highest_platform_unreachable"


def difficulty(transitions: int, robustness: float, margin: float, charge_margin: float):
    if transitions == 0:
        return {"rating": "unreachable", "score": 100.0, "transitions": 0}
    safety = 0.45 * robustness + 0.35 * max(0.0, min(1.0, margin / 48.0)) + 0.20 * max(0.0, min(1.0, charge_margin / 0.25))
    score = round(100.0 * (1.0 - safety), 2)
    rating = "tutorial" if score < 20 else "easy" if score < 40 else "medium" if score < 60 else "hard" if score < 80 else "extreme"
    return {"rating": rating, "score": score, "transitions": transitions, "minimum_robustness": round(robustness, 3), "minimum_horizontal_margin_px": round(margin, 2), "minimum_charge_margin": round(charge_margin, 3)}


def validate_level(path: str, strict_flag_policy: bool = False, final: bool = False):
    level = parse_level(path)
    errors = []
    if not level.platforms:
        errors.append("no platforms")
    if strict_flag_policy and final and level.flag is None:
        errors.append("final campaign level must contain FLAG")
    if strict_flag_policy and not final and level.flag is not None:
        errors.append("non-final campaign level must not contain FLAG")
    transitions, robustness, margin, charge_margin, reason = route(level)
    if not transitions:
        errors.append(f"map is not mechanically reachable ({reason})")
    return {"path": os.path.normpath(path), "name": level.name, "screens": level.screens, "platforms": len(level.platforms), "has_flag": level.flag is not None, "valid": not errors, "errors": errors, "difficulty": difficulty(transitions, robustness, margin, charge_margin)}


def validate_campaign(campaign_path: str, strict_flag_policy: bool = False):
    base = os.path.dirname(campaign_path)
    with open(campaign_path, encoding="utf-8") as stream:
        names = [line.strip() for line in stream if line.strip() and not line.startswith("#")]
    if not names:
        raise ValueError("campaign contains no levels")
    normalized = [os.path.normpath(name) for name in names]
    if len(set(normalized)) != len(names):
        raise ValueError("campaign contains duplicate level entries")
    reports = []
    for index, name in enumerate(names):
        path = os.path.join(base, name)
        if not os.path.isfile(path):
            raise ValueError(f"campaign references missing level: {name}")
        reports.append(validate_level(path, strict_flag_policy=strict_flag_policy, final=index == len(names) - 1))
    return reports


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--campaign", action="store_true")
    parser.add_argument("--level")
    parser.add_argument("--strict-campaign-policy", action="store_true")
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()
    root = os.path.normpath(os.path.join(SCRIPT_DIR, "..", ".."))
    campaign = os.path.join(root, "Game", "Assets", "Levels", "campaign.txt")
    reports = validate_campaign(campaign, strict_flag_policy=args.strict_campaign_policy) if args.campaign else [validate_level(args.level, strict_flag_policy=args.strict_campaign_policy, final=True)]
    valid = all(report["valid"] for report in reports)
    if args.json:
        print(json.dumps(reports, indent=2, sort_keys=True))
    else:
        for report in reports:
            status = "OK" if report["valid"] else "FAIL"
            diff = report["difficulty"]
            print(f"[{status}] {report['name']}: {diff['rating']} ({diff['score']})")
            for error in report["errors"]:
                print(f"  - {error}")
    return 0 if valid else 1


if __name__ == "__main__":
    raise SystemExit(main())
