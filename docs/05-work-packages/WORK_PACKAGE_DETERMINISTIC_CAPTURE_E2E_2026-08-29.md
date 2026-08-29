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

## Scope boundary

Included:

- deterministic launcher behavior already introduced by PR #185;
- E2E capture execution;
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
