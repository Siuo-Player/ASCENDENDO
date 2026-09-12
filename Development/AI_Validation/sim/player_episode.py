#!/usr/bin/env python3
"""Physics-backed player episode simulation for ASCENDENDO levels.

This is an experiment layer, not a human-calibrated difficulty oracle. It uses
the existing fixed-step physics replica and player-profile parameters to play an
entire single-screen level until the derived goal is reached.

Primary observations are completion time, pixel progress loss, setbacks and
recovery time. There is no gameplay timeout; ``max_events`` is only a compute
safety guard and is never converted into a difficulty measurement.
"""
from __future__ import annotations

import math
import random
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

SCRIPT_DIR = Path(__file__).resolve().parent
VALIDATION_DIR = SCRIPT_DIR.parent
if str(VALIDATION_DIR) not in sys.path:
    sys.path.insert(0, str(VALIDATION_DIR))

from validate_campaign import Level, Surface, initial_surface, parse_level, rect  # type: ignore
from engine import (  # type: ignore
    Body,
    FIXED_STEP,
    LOGICAL_WIDTH,
    PLAYER_CHARGE_TIME,
    PLAYER_MOVE_SPEED,
    PLAYER_WIDTH,
    do_jump,
    physics_step,
    resolve_collision,
)
from player_model import PlayerProfile


@dataclass(frozen=True)
class Action:
    launch_x: float
    direction: int
    charge: float
    target_index: int


@dataclass(frozen=True)
class JumpResult:
    target_index: int
    landed_index: int
    landed_x: float
    landed_top: float
    flight_time_s: float
    action_time_s: float
    reached_target: bool
    completed_level: bool


@dataclass(frozen=True)
class FailureEvent:
    source_index: int
    target_index: int
    landing_index: int
    progress_loss_px: float
    recovery_time_s: float | None


@dataclass(frozen=True)
class SessionTelemetry:
    level: str
    level_name: str
    profile: str
    seed: int
    completed: bool
    completion_time_s: float | None
    progress_max_px: float
    total_progress_loss_px: float
    largest_progress_loss_px: float
    failure_count: int
    jump_count: int
    recovery_count: int
    recovery_time_total_s: float
    recovery_time_median_s: float | None
    failure_events: tuple[FailureEvent, ...]


def _surfaces(level: Level) -> tuple[Surface, ...]:
    return (initial_surface(level), *level.platforms)


def _center_x(surface: Surface) -> float:
    return surface.x + surface.w * 0.5


def _launch_positions(surface: Surface, count: int = 7) -> tuple[float, ...]:
    minimum = max(0.0, surface.x - PLAYER_WIDTH + 1.0)
    maximum = min(LOGICAL_WIDTH - PLAYER_WIDTH, surface.right - 1.0)
    if maximum <= minimum or count <= 1:
        return (max(0.0, min(LOGICAL_WIDTH - PLAYER_WIDTH, _center_x(surface) - PLAYER_WIDTH * 0.5)),)
    return tuple(minimum + (maximum - minimum) * i / (count - 1) for i in range(count))


def _candidate_targets(surfaces: tuple[Surface, ...], source_index: int) -> list[int]:
    source_top = surfaces[source_index].top
    return [index for index in range(1, len(surfaces)) if surfaces[index].top > source_top + 4.0]


def _simulate_action(
    source: Surface,
    action: Action,
    surfaces: tuple[Surface, ...],
    *,
    max_seconds: float = 3.5,
) -> JumpResult:
    body = Body(action.launch_x, source.top)
    body.grounded = True
    do_jump(body, action.direction, action.charge)
    rects = [rect(surface) for surface in surfaces]
    max_steps = int(max_seconds / FIXED_STEP)
    highest_top = max(surface.top for surface in surfaces[1:])

    for step in range(max_steps):
        physics_step(body, FIXED_STEP)
        resolve_collision(body, rects)
        if not (body.grounded and body.vy == 0.0):
            continue

        center = body.x + PLAYER_WIDTH * 0.5
        landed_index = 0
        best_top_distance = abs(body.y - surfaces[0].top)
        for index in range(1, len(surfaces)):
            surface = surfaces[index]
            distance = abs(body.y - surface.top)
            if distance < best_top_distance and surface.x <= center <= surface.right:
                landed_index = index
                best_top_distance = distance
        landed_top = surfaces[landed_index].top
        flight_time = (step + 1) * FIXED_STEP
        return JumpResult(
            target_index=action.target_index,
            landed_index=landed_index,
            landed_x=body.x,
            landed_top=landed_top,
            flight_time_s=flight_time,
            action_time_s=flight_time + PLAYER_CHARGE_TIME * action.charge,
            reached_target=landed_index == action.target_index,
            completed_level=landed_top >= highest_top,
        )

    center = body.x + PLAYER_WIDTH * 0.5
    landed_index = 0
    for index in range(1, len(surfaces)):
        surface = surfaces[index]
        if abs(body.y - surface.top) < 2.0 and surface.x <= center <= surface.right:
            landed_index = index
            break
    landed_top = surfaces[landed_index].top
    return JumpResult(
        target_index=action.target_index,
        landed_index=landed_index,
        landed_x=body.x,
        landed_top=landed_top,
        flight_time_s=max_steps * FIXED_STEP,
        action_time_s=max_steps * FIXED_STEP + PLAYER_CHARGE_TIME * action.charge,
        reached_target=landed_index == action.target_index,
        completed_level=landed_top >= highest_top,
    )


def _nominal_actions(
    surfaces: tuple[Surface, ...], source_index: int, target_index: int
) -> Iterable[Action]:
    source = surfaces[source_index]
    target = surfaces[target_index]
    target_center = _center_x(target)
    for launch_x in _launch_positions(source):
        launch_center = launch_x + PLAYER_WIDTH * 0.5
        preferred_direction = 1 if target_center >= launch_center else -1
        for direction in (preferred_direction, -preferred_direction):
            for charge in (0.25, 0.4, 0.55, 0.7, 0.85, 1.0):
                yield Action(launch_x, direction, charge, target_index)


def choose_action(
    surfaces: tuple[Surface, ...], source_index: int, profile: PlayerProfile, rng: random.Random
) -> Action | None:
    targets = _candidate_targets(surfaces, source_index)
    if not targets:
        return None

    candidates: list[tuple[float, Action]] = []
    for target_index in targets:
        target = surfaces[target_index]
        gap = target.top - surfaces[source_index].top
        for action in _nominal_actions(surfaces, source_index, target_index):
            result = _simulate_action(surfaces[source_index], action, surfaces)
            if result.landed_index != target_index:
                continue
            center = result.landed_x + PLAYER_WIDTH * 0.5
            landing_margin = min(center - target.x, target.right - center)
            displacement = abs(center - _center_x(target))
            effort = abs(action.charge - 0.7) + displacement / max(16.0, target.w)
            score = 4.0 * gap + 1.5 * landing_margin - 18.0 * effort - 0.08 * result.action_time_s
            candidates.append((score, action))

    if not candidates:
        target_index = min(targets, key=lambda index: surfaces[index].top)
        target = surfaces[target_index]
        launch = max(0.0, min(LOGICAL_WIDTH - PLAYER_WIDTH, _center_x(surfaces[source_index]) - PLAYER_WIDTH * 0.5))
        direction = 1 if _center_x(target) >= _center_x(surfaces[source_index]) else -1
        return Action(launch, direction, 0.85, target_index)

    candidates.sort(key=lambda item: item[0], reverse=True)
    pool = candidates[: min(3, len(candidates))]
    if rng.random() < profile.route_noise and len(pool) > 1:
        return rng.choice(pool[1:])[1]
    return pool[0][1]


def _execute_action(
    source: Surface,
    action: Action,
    target_index: int,
    surfaces: tuple[Surface, ...],
    profile: PlayerProfile,
    rng: random.Random,
) -> tuple[Action, JumpResult]:
    consistency_scale = 1.0 + 0.35 * (1.0 - profile.consistency)
    launch_noise = rng.gauss(0.0, profile.horizontal_error_px * consistency_scale)
    aim_noise = rng.gauss(0.0, profile.aim_error_px * 0.5 * consistency_scale)
    braking_noise = rng.gauss(0.0, profile.braking_error * 20.0 * consistency_scale)
    charge_noise = rng.gauss(0.0, profile.charge_error * 0.35 * consistency_scale)
    charge = max(0.0, min(1.0, action.charge + charge_noise))
    launch_x = max(
        0.0,
        min(LOGICAL_WIDTH - PLAYER_WIDTH, action.launch_x + launch_noise + aim_noise + braking_noise),
    )
    executed = Action(launch_x, action.direction, charge, target_index)
    return executed, _simulate_action(source, executed, surfaces)


def simulate_session(
    level_path: str | Path,
    profile: PlayerProfile,
    *,
    seed: int,
    max_events: int = 500,
) -> SessionTelemetry:
    level = parse_level(str(level_path))
    surfaces = _surfaces(level)
    rng = random.Random(seed)
    current_index = 0
    high_water = surfaces[0].top
    elapsed = 0.0
    total_loss = 0.0
    largest_loss = 0.0
    failure_events: list[FailureEvent] = []
    active_recoveries: list[tuple[float, float]] = []
    completed_recovery_times: list[float] = []
    current_profile = profile
    events = 0

    for event in range(1, max_events + 1):
        action = choose_action(surfaces, current_index, current_profile, rng)
        if action is None:
            break
        events = event
        source = surfaces[current_index]
        walked = abs(action.launch_x - (source.x + PLAYER_WIDTH * 0.5)) / PLAYER_MOVE_SPEED
        reaction = max(0.0, current_profile.reaction_time_s + rng.gauss(0.0, current_profile.reaction_jitter_s))
        _, result = _execute_action(
            source, action, action.target_index, surfaces, current_profile, rng
        )
        elapsed += walked + reaction + result.action_time_s
        new_index = result.landed_index
        new_progress = surfaces[new_index].top

        if result.completed_level:
            return _telemetry(
                level, level_path, profile, seed, True, elapsed, high_water, total_loss,
                largest_loss, failure_events, events, completed_recovery_times,
            )

        if new_progress < high_water - 1e-6:
            loss = high_water - new_progress
            total_loss += loss
            largest_loss = max(largest_loss, loss)
            failure_events.append(
                FailureEvent(current_index, action.target_index, new_index, loss, None)
            )
            active_recoveries.append((high_water, elapsed))
            current_profile = current_profile.after_failure()
        elif new_progress > high_water + 1e-6:
            remaining: list[tuple[float, float]] = []
            for marker, start_time in active_recoveries:
                if new_progress >= marker:
                    completed_recovery_times.append(elapsed - start_time)
                else:
                    remaining.append((marker, start_time))
            active_recoveries = remaining
            high_water = new_progress

        current_index = new_index

    return _telemetry(
        level, level_path, profile, seed, False, None, high_water, total_loss,
        largest_loss, failure_events, events, completed_recovery_times,
    )


def _telemetry(
    level: Level,
    level_path: str | Path,
    profile: PlayerProfile,
    seed: int,
    completed: bool,
    completion_time_s: float | None,
    high_water: float,
    total_loss: float,
    largest_loss: float,
    failures: list[FailureEvent],
    jump_count: int,
    recoveries: list[float],
) -> SessionTelemetry:
    return SessionTelemetry(
        level=str(level_path),
        level_name=level.name,
        profile=profile.name,
        seed=seed,
        completed=completed,
        completion_time_s=round(completion_time_s, 4) if completion_time_s is not None else None,
        progress_max_px=round(high_water, 4),
        total_progress_loss_px=round(total_loss, 4),
        largest_progress_loss_px=round(largest_loss, 4),
        failure_count=len(failures),
        jump_count=jump_count,
        recovery_count=len(recoveries),
        recovery_time_total_s=round(sum(recoveries), 4),
        recovery_time_median_s=_median(recoveries),
        failure_events=tuple(failures),
    )


def _median(values: Iterable[float]) -> float | None:
    data = sorted(values)
    if not data:
        return None
    middle = len(data) // 2
    value = data[middle] if len(data) % 2 else (data[middle - 1] + data[middle]) / 2.0
    return round(value, 4)


def quantile(values: Iterable[float], q: float) -> float | None:
    data = sorted(values)
    if not data:
        return None
    q = max(0.0, min(1.0, q))
    position = (len(data) - 1) * q
    lower = math.floor(position)
    upper = math.ceil(position)
    if lower == upper:
        return round(data[lower], 4)
    fraction = position - lower
    return round(data[lower] + (data[upper] - data[lower]) * fraction, 4)
