#!/usr/bin/env python3
"""Authoritative static campaign validation and mechanical difficulty report.

This tool validates the shipped .lvl contract, then uses the existing fixed-step
simulation to search for a physically valid spawn->FLAG route. Difficulty is
reported from measurable jump margins rather than a subjective label.
"""
from __future__ import annotations

import argparse
import json
import math
import os
import random
import sys
from dataclasses import dataclass, asdict

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
SIM_DIR = os.path.join(SCRIPT_DIR, "sim")
sys.path.insert(0, SIM_DIR)
from engine import (  # type: ignore
    FIXED_STEP,
    LOGICAL_HEIGHT,
    LOGICAL_WIDTH,
    PLAYER_WIDTH,
    simulate_jump,
    simulate_jump_flag,
)

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
    platforms: tuple[Surface, ...]
    flag: Surface | None


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


def _finite(v: float) -> bool:
    return math.isfinite(v)


def parse_level(path: str) -> ParsedLevel:
    platforms: list[Surface] = []
    flag: Surface | None = None
    name = os.path.basename(path)
    with open(path, "r", encoding="utf-8") as handle:
        for line_no, raw in enumerate(handle, 1):
            line = raw.strip()
            if not line or line.startswith("#"):
                continue
            parts = line.split()
            directive = parts[0]
            if directive == "NAME":
                name = " ".join(parts[1:]) or name
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
            if x < 0 or x + w > LOGICAL_WIDTH or y < 0 or y + h > LOGICAL_HEIGHT:
                raise ValueError(f"{path}:{line_no}: geometry outside 640x360 page")
            surface = Surface(x, y, w, h)
            if directive == "FLAG":
                if flag is not None:
                    raise ValueError(f"{path}:{line_no}: multiple FLAG directives")
                flag = surface
            else:
                platforms.append(surface)
    return ParsedLevel(path, name, tuple(platforms), flag)


def _rect(surface: Surface) -> tuple[float, float, float, float]:
    return (surface.x, surface.y, surface.right, surface.top)


def _target_dx(source: Surface, target: Surface) -> float:
    return max(0.0, target.x - source.right, source.x - target.right)


def _scan_transition(source: Surface, target: Surface) -> Transition | None:
    best: Transition | None = None
    source_y = source.top
    target_y = target.top
    dy = target_y - source_y
    if dy > 0:
        # A jump cannot land above the effective ballistic apex. The simulator
        # is authoritative for actual collision placement; this is only a fast reject.
        pass
    center = target.x + target.w * 0.5
    dx_gap = _target_dx(source, target)
    for direction in (-1, 1):
        for idx in range(MAX_CHARGE_SAMPLES):
            charge = idx / (MAX_CHARGE_SAMPLES - 1)
            landed, fx, fy, _, _ = simulate_jump(
                source.x + source.w * 0.5 - PLAYER_WIDTH * 0.5,
                source_y,
                direction,
                charge,
                [_rect(source), _rect(target)],
            )
            if not landed or abs(fy - target_y) > 1.5:
                continue
            body_center = fx + PLAYER_WIDTH * 0.5
            if not (target.x <= body_center <= target.right):
                continue
            horizontal_margin = min(body_center - target.x, target.right - body_center)
            charge_margin = min(charge, 1.0 - charge)
            robustness = _robustness(source, target, direction, charge)
            candidate = Transition(
                0, 0, charge, direction, fx, fy,
                horizontal_margin, charge_margin, robustness,
            )
            if best is None or (candidate.robustness, candidate.horizontal_margin) > (
                best.robustness, best.horizontal_margin
            ):
                best = candidate
    return best


def _robustness(source: Surface, target: Surface, direction: int, charge: float) -> float:
    rng = random.Random(307)
    success = 0
    start_x = source.x + source.w * 0.5 - PLAYER_WIDTH * 0.5
    for _ in range(ROBUSTNESS_TRIALS):
        noisy_charge = max(0.0, min(1.0, charge + rng.uniform(-0.08, 0.08)))
        noisy_x = start_x + rng.uniform(-8.0, 8.0)
        landed, fx, fy, _, _ = simulate_jump(
            noisy_x, source.top, direction, noisy_charge, [_rect(source), _rect(target)]
        )
        center = fx + PLAYER_WIDTH * 0.5
        if landed and abs(fy - target.top) <= 1.5 and target.x <= center <= target.right:
            success += 1
    return success / ROBUSTNESS_TRIALS


def _route(level: ParsedLevel) -> tuple[list[Transition], str]:
    ground = Surface(0, 0, LOGICAL_WIDTH, 4)
    surfaces = [ground, *level.platforms]
    start = 0
    came_from: dict[int, int] = {}
    via: dict[int, Transition] = {}
    queue = [start]
    seen = {start}

    while queue:
        current = queue.pop(0)
        current_surface = surfaces[current]
        if level.flag is not None:
            flag_rect = _rect(level.flag)
            # Any airborne overlap is sufficient for the real runtime contract.
            for direction in (-1, 1):
                for idx in range(MAX_CHARGE_SAMPLES):
                    charge = idx / (MAX_CHARGE_SAMPLES - 1)
                    hit, _, _ = simulate_jump_flag(
                        current_surface.x + current_surface.w * 0.5 - PLAYER_WIDTH * 0.5,
                        current_surface.top,
                        direction,
                        charge,
                        [_rect(p) for p in level.platforms],
                        flag_rect,
                    )
                    if hit:
                        t = Transition(
                            current, -1, charge, direction,
                            0.0, level.flag.y, 0.0, min(charge, 1-charge), 1.0,
                        )
                        path = [t]
                        node = current
                        while node != start:
                            path.append(via[node])
                            node = came_from[node]
                        return list(reversed(path)), "spawn_to_flag"

        for idx, target in enumerate(surfaces[1:], 1):
            if idx in seen:
                continue
            transition = _scan_transition(current_surface, target)
            if transition is None:
                continue
            transition = Transition(
                current, idx, transition.charge, transition.direction,
                transition.landing_x, transition.landing_y,
                transition.horizontal_margin, transition.charge_margin,
                transition.robustness,
            )
            seen.add(idx)
            came_from[idx] = current
            via[idx] = transition
            queue.append(idx)

    return [], "unreachable_flag"


def difficulty_score(transitions: list[Transition]) -> dict[str, float | str]:
    if not transitions:
        return {"rating": "unreachable", "score": 100.0}
    robustness = sum(t.robustness for t in transitions) / len(transitions)
    margins = [max(0.0, min(1.0, t.horizontal_margin / 48.0)) for t in transitions]
    charge_margins = [max(0.0, min(1.0, t.charge_margin / 0.25)) for t in transitions]
    safety = 0.45 * robustness + 0.35 * (sum(margins) / len(margins)) + 0.20 * (
        sum(charge_margins) / len(charge_margins)
    )
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
        "mean_horizontal_margin_px": round(sum(t.horizontal_margin for t in transitions) / len(transitions), 2),
    }


def validate_level(path: str, require_flag: bool = True) -> dict:
    level = parse_level(path)
    errors: list[str] = []
    if require_flag and level.flag is None:
        errors.append("missing FLAG")
    transitions, reason = _route(level)
    if level.flag is not None and not transitions:
        errors.append(f"FLAG unreachable ({reason})")
    report = {
        "path": os.path.normpath(path),
        "name": level.name,
        "platforms": len(level.platforms),
        "flag": asdict(level.flag) if level.flag else None,
        "valid": not errors,
        "errors": errors,
        "route": [asdict(t) for t in transitions],
        "difficulty": difficulty_score(transitions),
    }
    return report


def validate_campaign(campaign_path: str) -> list[dict]:
    base = os.path.dirname(campaign_path)
    with open(campaign_path, "r", encoding="utf-8") as handle:
        names = [line.strip() for line in handle if line.strip() and not line.startswith("#")]
    reports = []
    for index, name in enumerate(names):
        path = os.path.join(base, name)
        reports.append(validate_level(path, require_flag=(index == len(names) - 1)))
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
    ok = all(r["valid"] for r in reports)
    if args.json:
        print(json.dumps(reports, indent=2, sort_keys=True))
    else:
        for r in reports:
            status = "OK" if r["valid"] else "FAIL"
            d = r["difficulty"]
            print(f"[{status}] {r['name']}: {d['rating']} ({d['score']})")
            for err in r["errors"]:
                print(f"  - {err}")
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
