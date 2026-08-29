# Capture E2E failure — runtime asset layout — 2026-08-29

The first deterministic capture workflow reached a successful release build but could not enter the requested campaign level.

The failure was caused by the established `RuntimePaths::assetsRoot()` contract: assets are resolved relative to the executable directory. The CI executable is `build/game/game`, so production-style runtime resolution requires `build/game/Game/Assets`.

The repository keeps source assets under `Game/Assets`, and `make game` does not stage them beside the executable. The first E2E run therefore failed before rendering and produced no PPM artifacts.

The fix is CI-only: stage `Game/Assets` under `build/game/Game/Assets` and assert the campaign and player sprite are present before capture. This reproduces the existing distribution layout without adding a runtime override or changing asset ownership.

This finding is evidence about the validation environment, not a gameplay, Vulkan, camera, or viewport defect.
