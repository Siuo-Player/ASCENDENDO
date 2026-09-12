#!/usr/bin/env python3
"""Player-profile primitives for difficulty experiments.

This module is intentionally independent from the authoritative mechanical
validator. A profile represents a hypothesis about how a player introduces
error and learns across attempts; it is not a claim about real players until
calibrated against telemetry.
"""
from __future__ import annotations

from dataclasses import dataclass, replace


@dataclass(frozen=True)
class PlayerProfile:
    name: str
    reaction_time_s: float
    reaction_jitter_s: float
    horizontal_error_px: float
    charge_error: float
    braking_error: float
    aim_error_px: float
    route_noise: float
    learning_rate: float
    consistency: float

    def after_failure(self) -> "PlayerProfile":
        """Return the profile after one failed attempt.

        Learning reduces execution noise and route randomness without ever
        creating a perfect player. This is a deliberately simple hypothesis
        layer for experiments; human-data calibration comes later.
        """
        factor = max(0.0, min(1.0, self.learning_rate))
        return replace(
            self,
            horizontal_error_px=max(0.25, self.horizontal_error_px * (1.0 - 0.20 * factor)),
            charge_error=max(0.005, self.charge_error * (1.0 - 0.20 * factor)),
            braking_error=max(0.25, self.braking_error * (1.0 - 0.15 * factor)),
            aim_error_px=max(0.25, self.aim_error_px * (1.0 - 0.20 * factor)),
            route_noise=max(0.01, self.route_noise * (1.0 - 0.15 * factor)),
        )


PROFILES: tuple[PlayerProfile, ...] = (
    PlayerProfile("novice", 0.34, 0.10, 18.0, 0.18, 0.25, 14.0, 0.30, 0.18, 0.60),
    PlayerProfile("beginner", 0.27, 0.07, 12.0, 0.13, 0.19, 10.0, 0.23, 0.22, 0.68),
    PlayerProfile("intermediate", 0.21, 0.05, 8.0, 0.09, 0.13, 7.0, 0.16, 0.24, 0.77),
    PlayerProfile("advanced", 0.16, 0.035, 5.0, 0.06, 0.09, 4.0, 0.10, 0.18, 0.86),
    PlayerProfile("expert", 0.12, 0.02, 2.5, 0.035, 0.05, 2.0, 0.05, 0.12, 0.94),
)


def profile_by_name(name: str) -> PlayerProfile:
    for profile in PROFILES:
        if profile.name == name:
            return profile
    raise KeyError(f"unknown player profile: {name}")
