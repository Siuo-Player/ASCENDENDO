# Camera follow Lerp bound — 2026-08-29

## Finding

`Camera::follow()` used `speed * dt` directly as its interpolation factor. With `speed * dt > 1`, the operation becomes extrapolation and can overshoot the target.

## Current remediation

PR branch `fix/camera-follow-lerp-clamp-20260829` clamps the interpolation factor to `[0, 1]` and adds a regression test with a large `dt`.

## Scope

This is a camera-local numerical invariant. It does not change the fixed-step policy, camera offset, coordinate system, or gameplay semantics.
