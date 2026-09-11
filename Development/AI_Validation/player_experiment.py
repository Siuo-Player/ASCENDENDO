#!/usr/bin/env python3
"""Episode-level player simulation scaffold.

This first experiment layer intentionally does not replace the authoritative
mechanical validator. It records a reproducible session specification and
telemetry shape that can later be backed by a full controller/physics episode
simulator and calibrated against human play data.
"""
from __future__ import annotations

import argparse
import json
import random
from dataclasses import asdict, dataclass

from player_model import PlayerProfile, PROFILES, profile_by_name


@dataclass(frozen=True)
class AttemptTelemetry:
    attempt: int
    success: bool
    failure_reason: str | None
    progress: float
    elapsed_s: float
    profile: str


@dataclass(frozen=True)
class SessionTelemetry:
    level: str
    profile: str
    attempts: tuple[AttemptTelemetry, ...]
    completed: bool


def _attempt_outcome(profile: PlayerProfile, difficulty_proxy: float, rng: random.Random) -> bool:
    """Temporary stochastic controller proxy used only by the experiment scaffold.

    The proxy is deliberately not used by the mechanical validator or by the
    production game. Replace it with full episode simulation before treating
    any result as a difficulty estimate.
    """
    execution = (
        0.45 * profile.consistency
        + 0.20 * max(0.0, 1.0 - profile.horizontal_error_px / 20.0)
        + 0.15 * max(0.0, 1.0 - profile.charge_error)
        + 0.10 * max(0.0, 1.0 - profile.aim_error_px / 16.0)
        + 0.10 * max(0.0, 1.0 - profile.route_noise)
    )
    probability = max(0.01, min(0.995, execution - 0.55 * difficulty_proxy))
    return rng.random() < probability


def simulate_session(
    level: str,
    profile: PlayerProfile,
    difficulty_proxy: float,
    attempts: int,
    seed: int,
) -> SessionTelemetry:
    rng = random.Random(seed)
    current = profile
    records: list[AttemptTelemetry] = []

    for number in range(1, attempts + 1):
        success = _attempt_outcome(current, difficulty_proxy, rng)
        progress = 1.0 if success else max(0.0, min(0.99, rng.random()))
        elapsed = 20.0 + 90.0 * (1.0 - progress) + rng.random() * 8.0
        records.append(
            AttemptTelemetry(
                attempt=number,
                success=success,
                failure_reason=None if success else "execution_error",
                progress=round(progress, 4),
                elapsed_s=round(elapsed, 3),
                profile=profile.name,
            )
        )
        if success:
            return SessionTelemetry(level, profile.name, tuple(records), True)
        current = current.after_failure()

    return SessionTelemetry(level, profile.name, tuple(records), False)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--level", required=True)
    parser.add_argument("--profile", default="all")
    parser.add_argument("--attempts", type=int, default=10)
    parser.add_argument("--seeds", type=int, default=100)
    parser.add_argument("--difficulty-proxy", type=float, default=0.5)
    args = parser.parse_args()
    if args.attempts < 1 or args.seeds < 1:
        parser.error("attempts and seeds must be positive")

    profiles = PROFILES if args.profile == "all" else (profile_by_name(args.profile),)
    summary = []
    for profile in profiles:
        sessions = [
            simulate_session(args.level, profile, args.difficulty_proxy, args.attempts, seed)
            for seed in range(args.seeds)
        ]
        clear_rate = sum(session.completed for session in sessions) / len(sessions)
        summary.append(
            {
                "profile": profile.name,
                "clear_rate": round(clear_rate, 4),
                "median_attempts": sorted(len(session.attempts) for session in sessions)[len(sessions) // 2],
            }
        )

    print(json.dumps({"level": args.level, "profiles": summary}, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
