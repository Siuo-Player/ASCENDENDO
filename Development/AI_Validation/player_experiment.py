#!/usr/bin/env python3
"""Run reproducible full-level player episodes.

The command now delegates to the physics-backed episode simulator. There is no
hand-authored difficulty proxy and no artificial session timeout. Results remain
experimental until player profiles are calibrated against human telemetry.
"""
from __future__ import annotations

import argparse
import json
from dataclasses import asdict
from pathlib import Path

from player_model import PROFILES, profile_by_name
from sim.player_episode import simulate_session


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--level", required=True)
    parser.add_argument("--profile", default="all")
    parser.add_argument("--seeds", type=int, default=100)
    parser.add_argument("--max-events", type=int, default=500)
    args = parser.parse_args()
    if args.seeds < 1 or args.max_events < 1:
        parser.error("seeds and max-events must be positive")

    profiles = PROFILES if args.profile == "all" else (profile_by_name(args.profile),)
    output = []
    for profile in profiles:
        sessions = [
            simulate_session(
                Path(args.level),
                profile,
                seed=seed,
                max_events=args.max_events,
            )
            for seed in range(args.seeds)
        ]
        completed = [s for s in sessions if s.completed and s.completion_time_s is not None]
        output.append(
            {
                "profile": profile.name,
                "runs": len(sessions),
                "completion_rate": round(sum(s.completed for s in sessions) / len(sessions), 4),
                "completion_time_median_s": (
                    round(sorted(s.completion_time_s for s in completed)[len(completed) // 2], 4)
                    if completed else None
                ),
                "progress_loss_median_px": round(
                    sorted(s.total_progress_loss_px for s in sessions)[len(sessions) // 2], 4
                ),
                "largest_setback_p90_px": round(
                    sorted(s.largest_progress_loss_px for s in sessions)[int(0.9 * (len(sessions) - 1))], 4
                ),
            }
        )

    print(json.dumps({"level": args.level, "profiles": output}, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
