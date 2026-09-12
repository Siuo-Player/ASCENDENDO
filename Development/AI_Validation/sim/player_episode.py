#!/usr/bin/env python3
"""Physics-backed player episode simulation for ASCENDENDO levels.

This module is an experiment layer, not a human-calibrated difficulty oracle.
It uses the existing fixed-step physics replica and player-profile parameters to
play an entire single-screen level until the derived goal is reached.

The primary observations are deliberately multidimensional:
- completion time;
- progress lost in pixels after failed transitions;
- setback count and largest setback;
- recovery time until the pre-failure high-water mark is restored.

No artificial gameplay timeout is used. ``max_events`` is only a computational
safety guard against a pathological controller and is never treated as a
failure or difficulty measurement.
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
    PLAYER_HEIGHT,
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
    return tuple(
        minimum + (maximum - minimum) * i / (count - 1)
        for i in range(count)
    )


def _candidate_targets(surfaces: tuple[Surface, ...], source_index: int) -> list[int]:
    source_top = surfaces[source_index].top
    return [
        index
        for index in range(1, len(surfaces))
        if surfaces[index].top > source_top + 4.0
    ]


def _simulate_action(
    source: Surface,
    target: Surface,
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

    for step in range(max_steps):
        physics_step(body, FIXED_STEP)
        resolve_collision(body, rects)

        if body.grounded and body.vy == 0.0:
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
            reached = landed_index == action.target_index
            completed = landed_top >= max(surface.top for surface in surfaces[1:])
            action_time = flight_time + PLAYER_CHARGE_TIME * action.charge
            return JumpResult(
                target_index=action.target_index,
                landed_index=landed_index,
                landed_x=body.x,
                landed_top=landed_top,
                flight_time_s=flight_time,
                action_time_s=action_time,
                reached_target=reached,
                completed_level=completed,
            )

    center = body.x + PLAYER_WIDTH * 0.5
    landed_index = 0
    for index in range(1, len(surfaces)):
        surface = surfaces[index]
        if abs(body.y - surface.top) < 2.0 and surface.x <= center <= surface.right:
            landed_index = index
            break
    landed_top = surfaces[landed_index].top
    flight_time = max_steps * FIXED_STEP
    return JumpResult(
        target_index=action.target_index,
        landed_index=landed_index,
        landed_x=body.x,
        landed_top=landed_top,
        flight_time_s=flight_time,
        action_time_s=flight_time + PLAYER_CHARGE_TIME * action.charge,
        reached_target=landed_index == action.target_index,
        completed_level=landed_top >= max(surface.top for surface in surfaces[1:]),
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
        directions = (preferred_direction, -preferred_direction)
        for direction in directions:
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
            result = _simulate_action(surfaces[source_index], target, action, surfaces)
            if result.landed_index != target_index:
                continue
            landing_margin = min(
                result.landed_x + PLAYER_WIDTH * 0.5 - target.x,
                target.right - (result.landed_x + PLAYER_WIDTH * 0.5),
            )
            if landing_margin < 0:
                continue
            displacement = abs(result.landed_x + PLAYER_WIDTH * 0.5 - _center_x(target))
            effort = abs(action.charge - 0.7) + displacement / max(16.0, target.w)
            score = (
                4.0 * gap
                + 1.5 * landing_margin
                - 18.0 * effort
                - 0.08 * result.action_time_s
            )
            candidates.append((score, action))

    if not candidates:
        # A route can still be playable if the coarse planner misses a very
        # narrow control window. Fall back to the geometrically closest target.
        target_index = min(targets, key=lambda i: surfaces[i].top)
        target = surfaces[target_index]
        launch = max(
            0.0,
            min(LOGICAL_WIDTH - PLAYER_WIDTH, _center_x(surfaces[source_index]) - PLAYER_WIDTH * 0.5),
        )
        direction = 1 if _center_x(target) >= _center_x(surfaces[source_index]) else -1
        return Action(launch, direction, 0.85, target_index)

    candidates.sort(key=lambda item: item[0], reverse=True)
    # Route noise means the agent occasionally chooses a non-optimal but still
    # plausible target/action. It is not an artificial death probability.
    pool_size = min(3, len(candidates))
    pool = candidates[:pool_size]
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
    target = surfaces[target_index]
    result = _simulate_action(source, target, executed, surfaces)
    return executed, result


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
    pending_recoveries: list[tuple[float, float]] = []
    current_profile = profile

    for _event in range(1, max_events + 1):
        action = choose_action(surfaces, current_index, current_profile, rng)
        if action is None:
            break

        source = surfaces[current_index]
        target = surfaces[action.target_index]
        walked = abs(action.launch_x - (source.x + PLAYER_WIDTH * 0.5)) / PLAYER_MOVE_SPEED
        reaction = max(
            0.0,
            current_profile.reaction_time_s
            + rng.gauss(0.0, current_profile.reaction_jitter_s),
        )
        reaction = max(0.0, reaction)
        executed, result = _execute_action(
            source, action, action.target_index, surfaces, current_profile, rng
        )
        elapsed += walked + reaction + result.action_time_s
        new_index = result.landed_index
        new_progress = surfaces[new_index].top

        if result.completed_level:
            return SessionTelemetry(
                level=str(level_path),
                level_name=level.name,
                profile=profile.name,
                seed=seed,
                completed=True,
                completion_time_s=elapsed,
                progress_max_px=max(high_water, new_progress),
                total_progress_loss_px=total_loss,
                largest_progress_loss_px=largest_loss,
                failure_count=len(failure_events),
                jump_count=_event,
                recovery_count=len(pending_recoveries),
                recovery_time_total_s=sum(value for _, value in pending_recoveries),
                recovery_time_median_s=_median(value for _, value in pending_recoveries),
                failure_events=tuple(failure_events),
            )

        if new_progress < high_water - 1e-6:
            loss = high_water - new_progress
            total_loss += loss
            largest_loss = max(largest_loss, loss)
            failure_events.append(
                FailureEvent(
                    source_index=current_index,
                    target_index=action.target_index,
                    landing_index=new_index,
                    progress_loss_px=loss,
                    recovery_time_s=None,
                )
            )
            pending_recoveries.append((high_water, elapsed))
            current_profile = current_profile.after_failure()
        elif new_progress > high_water + 1e-6:
            # Close recovery windows whose prior high-water mark has been
            # restored. Their duration is measured from failure landing until
            # this point, so it includes all intervening execution and errors.
            closed: list[tuple[float, float]] = []
            still_open: list[tuple[float, float]] = []
            for marker, start_time in pending_recoveries:
                if new_progress >= marker:
                    closed.append((marker, elapsed - start_time))
                else:
                    still_open.append((marker, start_time))
            pending_recoveries = still_open
            pending_recoveries.extend(closed)
            high_water = new_progress

        current_index = new_index

    return SessionTelemetry(
        level=str(level_path),
        level_name=level.name,
        profile=profile.name,
        seed=seed,
        completed=False,
        completion_time_s=None,
        progress_max_px=high_water,
        total_progress_loss_px=total_loss,
        largest_progress_loss_px=largest_loss,
        failure_count=len(failure_events),
        jump_count=max_events,
        recovery_count=len(pending_recoveries),
        recovery_time_total_s=sum(value for _, value in pending_recoveries),
        recovery_time_median_s=_median(value for _, value in pending_recoveries),
        failure_events=tuple(failure_events),
    )


def _median(values: Iterable[float]) -> float | None:
    data = sorted(values)
    if not data:
        return None
    middle = len(data) // 2
    if len(data) % 2:
        return round(data[middle], 4)
    return round((data[middle - 1] + data[middle]) / 2.0, 4)


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
