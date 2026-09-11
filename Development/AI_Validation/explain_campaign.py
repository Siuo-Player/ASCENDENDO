#!/usr/bin/env python3
"""Explain why campaign geometry receives its difficulty score.

This is diagnostic only: it reuses the authoritative validator's parser,
physics simulator and transition search, but does not change validation rules,
rating thresholds or gameplay physics.
"""
from __future__ import annotations

import argparse
import os
import sys
from dataclasses import dataclass

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(SCRIPT_DIR, "..", ".."))
sys.path.insert(0, SCRIPT_DIR)

import validate_campaign as validator  # type: ignore


@dataclass(frozen=True)
class TransitionDiagnostic:
    source: str
    target: str
    source_index: int
    target_index: int
    robustness: float
    landing_margin_px: float
    launch_margin_px: float
    charge_margin: float
    horizontal_displacement_px: float
    vertical_gap_px: float
    target_width_px: float
    launch_x_px: float
    direction: int
    charge: float

    @property
    def safety_components(self) -> dict[str, float]:
        clamp = lambda value, limit: max(0.0, min(1.0, value / limit))
        return {
            "robustness": self.robustness,
            "landing_margin": clamp(self.landing_margin_px, 48.0),
            "launch_margin": clamp(self.launch_margin_px, 48.0),
            "charge_margin": clamp(self.charge_margin, 0.25),
        }

    @property
    def weighted_risks(self) -> dict[str, float]:
        safety = self.safety_components
        return {
            "robustness": 0.40 * (1.0 - safety["robustness"]),
            "landing_margin": 0.30 * (1.0 - safety["landing_margin"]),
            "launch_margin": 0.10 * (1.0 - safety["launch_margin"]),
            "charge_margin": 0.20 * (1.0 - safety["charge_margin"]),
        }


def transition_diagnostic(source, target, transition, source_index, target_index):
    robustness, direction, charge, margin, launch_margin, charge_margin, neg_start_x = transition
    start_x = -neg_start_x
    source_center = start_x + validator.PLAYER_WIDTH * 0.5
    target_center = target.x + target.w * 0.5
    return TransitionDiagnostic(
        source="ground" if source_index == 0 else f"platform {source_index}",
        target=f"platform {target_index}",
        source_index=source_index,
        target_index=target_index,
        robustness=robustness,
        landing_margin_px=margin,
        launch_margin_px=launch_margin,
        charge_margin=charge_margin,
        horizontal_displacement_px=abs(target_center - source_center),
        vertical_gap_px=max(0.0, target.y - source.top),
        target_width_px=target.w,
        launch_x_px=start_x,
        direction=direction,
        charge=charge,
    )


def diagnose_level(level: validator.Level):
    surfaces = [validator.initial_surface(level), *level.platforms]
    queue = [0]
    seen = {0}
    transitions: list[TransitionDiagnostic] = []

    while queue:
        source_index = queue.pop(0)
        source = surfaces[source_index]
        for target_index, target in enumerate(level.platforms, 1):
            if target_index in seen:
                continue
            transition = validator.landing_transition(source, target)
            if transition is None:
                continue
            transitions.append(
                transition_diagnostic(
                    source, target, transition, source_index, target_index
                )
            )
            seen.add(target_index)
            queue.append(target_index)

    return {
        "level": level.name,
        "transitions": transitions,
        "unreachable_platforms": [
            index for index in range(1, len(surfaces)) if index not in seen
        ],
    }


def score_breakdown(difficulty: dict[str, float]) -> dict[str, float]:
    robustness = difficulty.get("minimum_robustness", 1.0)
    landing = difficulty.get("minimum_horizontal_margin_px", 48.0)
    launch = difficulty.get("minimum_launch_margin_px", 48.0)
    charge = difficulty.get("minimum_charge_margin", 0.25)
    components = {
        "robustness": 0.40 * (1.0 - max(0.0, min(1.0, robustness))),
        "landing_margin": 0.30 * (1.0 - max(0.0, min(1.0, landing / 48.0))),
        "launch_margin": 0.10 * (1.0 - max(0.0, min(1.0, launch / 48.0))),
        "charge_margin": 0.20 * (1.0 - max(0.0, min(1.0, charge / 0.25))),
    }
    return {name: round(100.0 * value, 2) for name, value in components.items()}


def explain_level(path: str, final: bool = False) -> None:
    level = validator.parse_level(path)
    report = validator.validate_level(path, final=final)
    difficulty = report["difficulty"]
    diagnosis = diagnose_level(level)
    transitions = diagnosis["transitions"]

    print(f"\n=== {level.name} ===")
    print(f"rating={difficulty['rating']} score={difficulty['score']}")
    print(
        "global minima: "
        f"robustness={difficulty.get('minimum_robustness', 0):.3f}, "
        f"landing_margin={difficulty.get('minimum_horizontal_margin_px', 0):.2f}px, "
        f"launch_margin={difficulty.get('minimum_launch_margin_px', 0):.2f}px, "
        f"charge_margin={difficulty.get('minimum_charge_margin', 0):.3f}"
    )

    breakdown = score_breakdown(difficulty)
    print(
        "score contributions: "
        + ", ".join(f"{name}={value:.2f}" for name, value in breakdown.items())
    )
    if breakdown["charge_margin"] >= 20.0 and difficulty.get("minimum_charge_margin", 0.0) == 0.0:
        print(
            "charge note: a feasible solution uses the minimum charge boundary; "
            "this is part of the current score formula, but is not treated as an "
            "actionable bottleneck by this diagnostic."
        )

    if diagnosis["unreachable_platforms"]:
        print(
            "UNREACHABLE: "
            + ", ".join(f"platform {index}" for index in diagnosis["unreachable_platforms"])
        )

    if not transitions:
        print("diagnosis: no reachable transition was found")
        return

    print("reachable transitions:")
    for transition in transitions:
        risks = transition.weighted_risks
        actionable = {
            name: value for name, value in risks.items() if name != "charge_margin"
        }
        dominant = max(actionable, key=actionable.get)
        print(
            f"  {transition.source} -> {transition.target}: "
            f"rob={transition.robustness:.3f}, "
            f"land={transition.landing_margin_px:.1f}px, "
            f"launch={transition.launch_margin_px:.1f}px, "
            f"charge={transition.charge_margin:.3f}, "
            f"dx={transition.horizontal_displacement_px:.1f}px, "
            f"dy={transition.vertical_gap_px:.1f}px, "
            f"width={transition.target_width_px:.1f}px, "
            f"actionable={dominant}"
        )

    bottlenecks = {
        "robustness": min(transitions, key=lambda item: item.robustness),
        "landing_margin": min(transitions, key=lambda item: item.landing_margin_px),
        "launch_margin": min(transitions, key=lambda item: item.launch_margin_px),
    }
    print("actionable bottlenecks:")
    for metric, transition in bottlenecks.items():
        value = {
            "robustness": transition.robustness,
            "landing_margin": transition.landing_margin_px,
            "launch_margin": transition.launch_margin_px,
        }[metric]
        print(f"  {metric}: {transition.source} -> {transition.target} ({value:.3f})")

    strongest_risk = max(
        (
            (name, transition.weighted_risks[name], transition)
            for name, transition in bottlenecks.items()
        ),
        key=lambda item: item[1],
    )
    metric, risk, transition = strongest_risk
    recommendations = {
        "robustness": "o salto é sensível a erro; para aumentar dificuldade, reduza tolerância geométrica ou aumente o deslocamento lateral.",
        "landing_margin": "a aterragem é o gargalo; para aumentar dificuldade, reduza a largura útil do alvo ou desloque-o lateralmente.",
        "launch_margin": "o posicionamento de lançamento é o gargalo; para aumentar dificuldade, reduza a margem de preparação no suporte de origem.",
    }
    print(
        f"feedback principal: {metric} ({risk:.3f} weighted risk) em "
        f"{transition.source} -> {transition.target}."
    )
    print(f"  {recommendations[metric]}")
    print("  Para facilitar, faça a alteração inversa. Não altere a física para corrigir um nível isolado.")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--campaign", action="store_true", help="diagnose every campaign level")
    parser.add_argument("--level", help="diagnose one level file")
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
