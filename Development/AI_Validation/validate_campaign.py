#!/usr/bin/env python3
"""Authoritative mechanical campaign validation and control-space difficulty report."""
from __future__ import annotations

import argparse
import heapq
import json
import math
import os
import sys
from dataclasses import dataclass

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(SCRIPT_DIR, "sim"))
from engine import LOGICAL_HEIGHT, LOGICAL_WIDTH, PLAYER_WIDTH, simulate_jump, simulate_jump_flag  # type: ignore

CHARGE_SAMPLES = 41
LAUNCH_X_SAMPLES = 17
AUTO_GROUND_HEIGHT = 16.0
AUTO_GROUND_X = 0.0
AUTO_FLAG_HEIGHT = 40.0


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


def rect(s: Surface) -> tuple[float, float, float, float]:
    return s.x, s.y, s.right, s.top


def parse_level(path: str) -> Level:
    platforms: list[Surface] = []
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
            if directive in {"SPAWN", "FLAG"}:
                raise ValueError(f"{path}:{number}: {directive} is derived and must not be authored")
            if directive != "PLATFORM" or len(fields) != 5:
                raise ValueError(f"{path}:{number}: invalid directive")
            values = tuple(float(value) for value in fields[1:])
            if not all(math.isfinite(value) for value in values):
                raise ValueError(f"{path}:{number}: non-finite geometry")
            x, y, width, height = values
            if width <= 0 or height <= 0:
                raise ValueError(f"{path}:{number}: non-positive geometry")
            level_height = screens * LOGICAL_HEIGHT
            if x < 0 or x + width > LOGICAL_WIDTH or y < AUTO_GROUND_HEIGHT or y + height > level_height:
                raise ValueError(f"{path}:{number}: geometry outside {LOGICAL_WIDTH}x{level_height} or below implicit ground")
            platforms.append(Surface(x, y, width, height))
    return Level(path, name, screens, tuple(platforms))


def initial_surface(level: Level) -> Surface:
    del level
    return Surface(AUTO_GROUND_X, 0.0, LOGICAL_WIDTH, AUTO_GROUND_HEIGHT)


def launch_positions(source: Surface) -> tuple[float, ...]:
    minimum = max(0.0, source.x - PLAYER_WIDTH + 1.0)
    maximum = min(LOGICAL_WIDTH - PLAYER_WIDTH, source.right - 1.0)
    if maximum <= minimum:
        return (max(0.0, min(LOGICAL_WIDTH - PLAYER_WIDTH, source.x + source.w * 0.5 - PLAYER_WIDTH * 0.5)),)
    return tuple(
        minimum + (maximum - minimum) * index / (LAUNCH_X_SAMPLES - 1)
        for index in range(LAUNCH_X_SAMPLES)
    )


def landing_success(source: Surface, target: Surface, start_x: float, direction: int, charge: float) -> bool:
    landed, fx, fy, _, _ = simulate_jump(
        start_x,
        source.top,
        direction,
        charge,
        [rect(source), rect(target)],
    )
    if not landed or abs(fy - target.top) > 1.5:
        return False
    center = fx + PLAYER_WIDTH * 0.5
    return target.x <= center <= target.right


def landing_profile(source: Surface, target: Surface) -> dict[str, float | int | str] | None:
    """Measure the actual control-space available to land on the target.

    For each reachable launch position we allow the player to choose the better
    direction and measure the fraction of charge values that succeed. This is
    deliberately not based on the single best jump: it measures how forgiving
    the entire usable control window is.
    """
    positions = launch_positions(source)
    usable_positions = 0
    charge_success_sum = 0
    total_best_actions = 0
    best_overall = None
    for start_x in positions:
        per_direction: list[tuple[int, int, int]] = []
        for direction in (-1, 1):
            successes = sum(
                landing_success(source, target, start_x, direction, index / (CHARGE_SAMPLES - 1))
                for index in range(CHARGE_SAMPLES)
            )
            per_direction.append((successes, direction, start_x))
        best_successes, best_direction, best_start = max(per_direction, key=lambda item: item[0])
        if best_successes:
            usable_positions += 1
            charge_success_sum += best_successes / CHARGE_SAMPLES
            total_best_actions += best_successes
            candidate = (best_successes / CHARGE_SAMPLES, best_direction, best_start)
            if best_overall is None or candidate[0] > best_overall[0]:
                best_overall = candidate

    if not usable_positions:
        return None
    launch_coverage = usable_positions / len(positions)
    charge_coverage = charge_success_sum / usable_positions
    control_success = math.sqrt(launch_coverage * charge_coverage)
    return {
        "control_success_probability": control_success,
        "launch_coverage": launch_coverage,
        "charge_coverage": charge_coverage,
        "usable_launch_positions": usable_positions,
        "sampled_launch_positions": len(positions),
        "best_direction": int(best_overall[1]),
        "best_charge": round(
            next(
                index / (CHARGE_SAMPLES - 1)
                for index in range(CHARGE_SAMPLES)
                if landing_success(source, target, best_overall[2], best_overall[1], index / (CHARGE_SAMPLES - 1))
            ),
            3,
        ),
        "horizontal_displacement_px": abs(
            target.x + target.w * 0.5 - (best_overall[2] + PLAYER_WIDTH * 0.5)
        ),
        "vertical_gap_px": max(0.0, target.y - source.top),
        "target_width_px": target.w,
        "source_width_px": source.w,
        "best_launch_x_px": best_overall[2],
        "sampled_actions": total_best_actions,
    }


def flag_profile(source: Surface, flag: Surface, level: Level) -> dict[str, float | int | str] | None:
    positions = launch_positions(source)
    usable_positions = 0
    success_sum = 0.0
    best = None
    platforms = [rect(platform) for platform in level.platforms]
    for start_x in positions:
        direction_results = []
        for direction in (-1, 1):
            successes = 0
            for index in range(CHARGE_SAMPLES):
                charge = index / (CHARGE_SAMPLES - 1)
                hit, _, _ = simulate_jump_flag(
                    start_x,
                    source.top,
                    direction,
                    charge,
                    platforms,
                    rect(flag),
                )
                successes += int(hit)
            direction_results.append((successes, direction))
        best_successes, best_direction = max(direction_results, key=lambda item: item[0])
        if best_successes:
            usable_positions += 1
            success_sum += best_successes / CHARGE_SAMPLES
            candidate = best_successes / CHARGE_SAMPLES
            if best is None or candidate > best:
                best = candidate
    if not usable_positions:
        return None
    launch_coverage = usable_positions / len(positions)
    charge_coverage = success_sum / usable_positions
    return {
        "control_success_probability": math.sqrt(launch_coverage * charge_coverage),
        "launch_coverage": launch_coverage,
        "charge_coverage": charge_coverage,
        "usable_launch_positions": usable_positions,
        "sampled_launch_positions": len(positions),
        "best_direction": best_direction,
        "best_charge_window": best,
        "vertical_gap_px": max(0.0, flag.y - source.top),
        "target_width_px": flag.w,
    }


def derived_flag(level: Level) -> Surface | None:
    if not level.platforms:
        return None
    highest = max(level.platforms, key=lambda platform: (platform.top, platform.w, -platform.x))
    return Surface(highest.x, highest.top, highest.w, AUTO_FLAG_HEIGHT)


def build_graph(level: Level) -> dict[int, list[tuple[int, dict[str, float | int | str]]]]:
    surfaces = [initial_surface(level), *level.platforms]
    graph: dict[int, list[tuple[int, dict[str, float | int | str]]]] = {index: [] for index in range(len(surfaces))}
    for source_index, source in enumerate(surfaces):
        for target_index, target in enumerate(level.platforms, 1):
            if target_index == source_index:
                continue
            profile = landing_profile(source, target)
            if profile is not None and float(profile["control_success_probability"]) > 0.0:
                graph[source_index].append((target_index, profile))
    return graph


def choose_route(level: Level, final: bool) -> tuple[list[dict[str, object]], float, str]:
    graph = build_graph(level)
    surfaces = [initial_surface(level), *level.platforms]
    highest_index = max(range(1, len(surfaces)), key=lambda index: surfaces[index].top) if level.platforms else None
    flag = derived_flag(level) if final else None
    distances: dict[int, float] = {0: 0.0}
    previous: dict[int, tuple[int, dict[str, float | int | str]]] = {}
    heap = [(0.0, 0)]
    terminal: tuple[float, int, dict[str, float | int | str] | None] | None = None
    while heap:
        cost, source_index = heapq.heappop(heap)
        if cost != distances.get(source_index):
            continue
        if flag is not None:
            profile = flag_profile(surfaces[source_index], flag, level)
            if profile is not None:
                candidate = (cost - math.log(float(profile["control_success_probability"])), source_index, profile)
                if terminal is None or candidate[0] < terminal[0]:
                    terminal = candidate
        if highest_index is not None and source_index == highest_index:
            terminal = (cost, source_index, None)
            break
        for target_index, profile in graph[source_index]:
            probability = float(profile["control_success_probability"])
            edge_cost = -math.log(probability)
            next_cost = cost + edge_cost
            if next_cost < distances.get(target_index, float("inf")):
                distances[target_index] = next_cost
                previous[target_index] = (source_index, profile)
                heapq.heappush(heap, (next_cost, target_index))
    if terminal is None:
        return [], 0.0, "highest_platform_unreachable"
    total_cost, terminal_source, terminal_profile = terminal
    transitions: list[dict[str, object]] = []
    current = terminal_source
    while current != 0:
        source_index, profile = previous[current]
        transitions.append({"source_index": source_index, "target_index": current, "profile": profile})
        current = source_index
    transitions.reverse()
    if terminal_profile is not None:
        transitions.append({"source_index": terminal_source, "target_index": "flag", "profile": terminal_profile})
    completion_probability = math.exp(-total_cost)
    return transitions, completion_probability, "top_reached" if terminal_profile is None else "flag_reached"


def difficulty_from_completion(completion_probability: float, transitions: int) -> dict[str, float | int | str]:
    completion_probability = max(0.0, min(1.0, completion_probability))
    score = round(100.0 * (1.0 - completion_probability), 2)
    if score >= 80.0:
        rating = "extreme"
    elif score >= 55.0:
        rating = "hard"
    elif score >= 30.0:
        rating = "medium"
    elif score >= 10.0:
        rating = "easy"
    else:
        rating = "tutorial"
    return {
        "rating": rating,
        "score": score,
        "estimated_route_completion": round(completion_probability, 3),
        "required_jumps": transitions,
        "model": "control_space_route_completion",
    }


def validate_level(path: str, strict_flag_policy: bool = False, final: bool = False):
    del strict_flag_policy
    level = parse_level(path)
    errors = []
    if not level.platforms:
        errors.append("no platforms")
    transitions, completion_probability, reason = choose_route(level, final=final)
    if not transitions:
        errors.append(f"map is not mechanically reachable ({reason})")
        difficulty = difficulty_from_completion(0.0, 0)
    else:
        difficulty = difficulty_from_completion(completion_probability, len(transitions))
    if final and derived_flag(level) is None:
        errors.append("final campaign level has no platform from which to derive the goal")
    return {
        "path": os.path.normpath(path),
        "name": level.name,
        "screens": level.screens,
        "platforms": len(level.platforms),
        "has_flag": final and derived_flag(level) is not None,
        "valid": not errors,
        "errors": errors,
        "difficulty": difficulty,
        "route": transitions,
    }


def validate_campaign(campaign_path: str, strict_flag_policy: bool = False):
    del strict_flag_policy
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
        reports.append(validate_level(path, final=index == len(names) - 1))
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
    reports = validate_campaign(campaign, strict_flag_policy=args.strict_campaign_policy) if args.campaign else [validate_level(args.level, final=True)]
    valid = all(report["valid"] for report in reports)
    if args.json:
        print(json.dumps(reports, indent=2, sort_keys=True))
    else:
        for report in reports:
            status = "OK" if report["valid"] else "FAIL"
            diff = report["difficulty"]
            print(f"[{status}] {report['name']}: {diff['rating']} ({diff['score']}) "
                  f"completion={diff['estimated_route_completion']:.3f} jumps={diff['required_jumps']}")
            for error in report["errors"]:
                print(f"  - {error}")
    return 0 if valid else 1


if __name__ == "__main__":
    raise SystemExit(main())
