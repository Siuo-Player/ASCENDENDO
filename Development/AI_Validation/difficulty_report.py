#!/usr/bin/env python3
"""Summarise physics-backed player sessions without collapsing them to a score."""
from __future__ import annotations

import argparse
import json
from pathlib import Path

from player_model import PROFILES, profile_by_name
from sim.player_episode import SessionTelemetry, quantile, simulate_session

ROOT = Path(__file__).resolve().parents[2]
LEVEL_DIR = ROOT / "Game" / "Assets" / "Levels"
CAMPAIGN_FILE = LEVEL_DIR / "campaign.txt"


def campaign_levels() -> list[Path]:
    levels: list[Path] = []
    for raw in CAMPAIGN_FILE.read_text(encoding="utf-8").splitlines():
        name = raw.strip()
        if not name or name.startswith("#"):
            continue
        levels.append(LEVEL_DIR / name)
    return levels


def summarise(sessions: list[SessionTelemetry]) -> dict[str, object]:
    completed = [s.completion_time_s for s in sessions if s.completed and s.completion_time_s is not None]
    losses = [s.total_progress_loss_px for s in sessions]
    largest = [s.largest_progress_loss_px for s in sessions]
    recoveries = [s.recovery_time_median_s for s in sessions if s.recovery_time_median_s is not None]
    setbacks = [float(s.failure_count) for s in sessions]
    return {
        "runs": len(sessions),
        "completion_rate": round(sum(s.completed for s in sessions) / len(sessions), 4),
        "completion_time_s": {
            "median": quantile(completed, 0.50),
            "p75": quantile(completed, 0.75),
            "p90": quantile(completed, 0.90),
            "mean": round(sum(completed) / len(completed), 4) if completed else None,
        },
        "progress_loss_px": {
            "median": quantile(losses, 0.50),
            "p75": quantile(losses, 0.75),
            "p90": quantile(losses, 0.90),
            "mean": round(sum(losses) / len(losses), 4),
        },
        "largest_setback_px": {
            "median": quantile(largest, 0.50),
            "p90": quantile(largest, 0.90),
            "max": max(largest) if largest else 0.0,
        },
        "recovery_time_s": {
            "median_of_session_medians": quantile(recoveries, 0.50),
            "p90_of_session_medians": quantile(recoveries, 0.90),
        },
        "failure_count": {
            "median": quantile(setbacks, 0.50),
            "p90": quantile(setbacks, 0.90),
        },
    }


def run_level(path: Path, profile_name: str, agents: int, seed: int) -> dict[str, object]:
    from validate_campaign import parse_level

    profile = profile_by_name(profile_name)
    level = parse_level(str(path))
    sessions = [simulate_session(level, profile, seed=seed + index) for index in range(agents)]
    return {
        "level": path.name,
        "level_name": level.name,
        "profile": profile_name,
        "summary": summarise(sessions),
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--level", type=Path)
    parser.add_argument("--campaign", action="store_true")
    parser.add_argument("--profile", default="all", choices=[p.name for p in PROFILES] + ["all"])
    parser.add_argument(
        "--agents",
        type=int,
        default=None,
        help="population size; default 1000 for one level, 100 for campaign exploration",
    )
    parser.add_argument("--seed", type=int, default=20260912)
    args = parser.parse_args()
    if not args.level and not args.campaign:
        parser.error("choose --level or --campaign")
    agents = args.agents if args.agents is not None else (1000 if args.level else 100)
    if agents < 1:
        parser.error("agents must be positive")

    profiles = [p.name for p in PROFILES] if args.profile == "all" else [args.profile]
    levels = campaign_levels() if args.campaign else [args.level]
    output = []
    for level in levels:
        for profile_name in profiles:
            output.append(run_level(level, profile_name, agents, args.seed))
    print(
        json.dumps(
            {"model": "physics_episode_experiment", "agents": agents, "results": output},
            indent=2,
            sort_keys=True,
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
