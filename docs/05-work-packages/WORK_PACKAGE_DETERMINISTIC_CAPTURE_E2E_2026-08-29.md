# Work Package — Deterministic Capture E2E Evidence — 2026-08-29

## Objective

Prove that the deterministic capture launcher and the existing `VulkanFrameCapture` path form one working end-to-end pipeline:

```text
capture level index
        ↓
CampaignRuntime / Level
        ↓
RendererFacade
        ↓
RendererCore::submitFrame()
        ↓
VulkanFrameCapture
        ↓
PPM on disk
```

This tranche validates existence and structural integrity of the captured frame. It does not establish visual quality, gameplay correctness, golden-image equivalence, or frame-rate independence.

## Evidence required

For each active campaign level in the current campaign:

1. the game starts in deterministic capture mode;
2. exactly one capture is emitted;
3. the process exits successfully after the rendered frame;
4. the PPM exists and has the expected dimensions for the headless framebuffer;
5. the PPM payload has the expected RGB byte count;
6. the pixel data is not completely uniform;
7. the PPM is retained as a CI artifact for inspection.

## Current implementation

- `ASCENDENDO_CAPTURE_LEVEL_INDEX` selects the level;
- `ASCENDENDO_CAPTURE_PPM` arms the existing `RendererCore` readback path;
- `Development/AI_Validation/validate_deterministic_capture.py` executes the game and validates the binary P6 PPM;
- `.github/workflows/deterministic-capture.yml` runs the validator for levels 0, 1 and 2 at `1280x720` and uploads the resulting PPMs.

## Findings from first E2E run

The first real E2E run reached the release-game build successfully but failed before level loading because `RuntimePaths::assetsRoot()` resolves assets relative to the executable directory. With the normal build layout, the executable is `build/game/game`, so the runtime correctly looked for:

```text
build/game/Game/Assets/...
```

while the repository assets remained under:

```text
Game/Assets/...
```

The failure was therefore a packaging/layout mismatch in the validation workflow, not a Vulkan or campaign-format failure. No PPM artifact was produced.

The CI workflow is corrected to stage `Game/Assets` beside the executable after `make game` and explicitly checks the campaign and player-sprite files before launching capture. This preserves the runtime path contract and does not change production runtime code.

## Validator hardening

During review, the P6 parser was corrected so it consumes only the mandatory header/pixel separator instead of stripping arbitrary whitespace from the binary payload. This matters because P6 pixel bytes are arbitrary binary data and may themselves equal whitespace byte values.

Therefore the validator now checks the binary boundary without mutating the payload before the exact byte-count check.

## Scope boundary

Included:

- deterministic launcher behavior already introduced by PR #185;
- E2E capture execution;
- runtime-layout staging required to reproduce the real distribution layout;
- structural PPM validation;
- artifact retention.

Excluded:

- image-diff thresholds;
- perceptual/golden-image approval;
- viewport/aspect-ratio policy changes;
- camera behavior changes;
- renderer abstraction redesign;
- gameplay changes;
- adding a fourth required CI gate merely for convenience.

## Decision rule

A successful PPM capture proves that the capture path is operational for the tested environment. It does not prove that the image is visually correct.

The next tranche may inspect the captured images and then characterize viewport/camera behavior only where a concrete visual or geometric discrepancy is reproduced.
