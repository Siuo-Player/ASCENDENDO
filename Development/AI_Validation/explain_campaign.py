#!/usr/bin/env python3
"""Explain the control-space route-completion difficulty model."""
from __future__ import annotations

import argparse
import os

import validate_campaign as validator

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))


def explain_level(path: str, final: bool = False) -> None:
    report = validator.validate_level(path, final=final)
    difficulty = report["difficulty"]
    print(f"\n=== {report['name']} ===")
    print(
        f"rating={difficulty['rating']} score={difficulty['score']} "
        f"estimated_completion={difficulty['estimated_route_completion']:.3f} "
        f"required_jumps={difficulty['required_jumps']}"
    )
    print("model: control-space route completion")
    if not report["route"]:
        print("diagnosis: no reachable route to the required goal")
        return

    route_success = 1.0
    for step_number, step in enumerate(report["route"], 1):
        profile = step["profile"]
        probability = float(profile["control_success_probability"])
        route_success *= probability
        target = "flag" if step["target_index"] == "flag" else f"platform {step['target_index']}"
        print(
            f"  jump {step_number}: {target} "
            f"control={probability:.3f}, "
            f"launch_coverage={float(profile['launch_coverage']):.3f}, "
            f"charge_coverage={float(profile['charge_coverage']):.3f}, "
            f"width={float(profile['target_width_px']):.1f}px, "
            f"dy={float(profile['vertical_gap_px']):.1f}px"
        )
    print(f"route product={route_success:.3f}")
    if abs(route_success - float(difficulty["estimated_route_completion"])) > 0.005:
        print("WARNING: reported completion differs from route product; inspect validator implementation.")
    if difficulty["score"] < 10:
        print("feedback: extremely forgiving control space; this is tutorial-level geometry.")
    elif difficulty["score"] < 30:
        print("feedback: broad control window; suitable for early easy content.")
    elif difficulty["score"] < 55:
        print("feedback: moderate control precision; suitable for the campaign medium band.")
    elif difficulty["score"] < 80:
        print("feedback: substantial precision required; suitable for hard content.")
    else:
        print("feedback: very small control window or an unreachable route; reserve for intentional extreme content.")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--campaign", action="store_true")
    parser.add_argument("--level")
    args = parser.parse_args()
    campaign = os.path.join(ROOT, "Game", "Assets", "Levels", "campaign.txt")
    if args.level:
        explain_level(args.level, final=True)
        return 0
    if not args.campaign:
        parser.error("use --campaign or --level")
    with open(campaign, encoding="utf-8") as stream:
        names = [line.strip() for line in stream if line.strip() and not line.startswith("#")]
    for index, name in enumerate(names):
        explain_level(os.path.join(os.path.dirname(campaign), name), final=index == len(names) - 1)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
