# Work Package — Deterministic Capture Launcher — 2026-08-29

## Objective
Provide a validation-only launch path that selects a campaign level deterministically before rendering, so the Vulkan PPM capture path can produce repeatable golden-scene evidence.

## Scope
- environment-variable driven selection only;
- normal menu/game flow unchanged when the variable is absent;
- selected level must be loaded without streaming unrelated preceding levels into the capture scene;
- capture remains opt-in through `ASCENDENDO_CAPTURE_PPM`.

## Required evidence
- selected level is the sole loaded scene;
- unsupported/out-of-range selection fails closed with an explicit message;
- existing three CI gates remain green;
- no generic application/scene-management abstraction is introduced.

## Follow-up
After this launcher is validated, run the existing stress manifest across the target viewport matrix and inspect actual PPM captures.
