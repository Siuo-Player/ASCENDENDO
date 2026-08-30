# ASCENDENDO — Work Package: deterministic viewport capture matrix

**Date:** 2026-08-30  
**Scope:** capture validation / presentation viewport  
**Status:** implementation tranche

## Finding

The deterministic capture workflow previously exercised only a fixed 1152×648 framebuffer. The visual stress manifest already defines 16:9, 4:3 and 21:9 viewport cases, but the capture process did not actually instantiate those window dimensions.

## Property

When capture mode requests a window size, the created GLFW window and resulting Vulkan framebuffer must use that requested size unless the existing monitor-size safety policy necessarily scales it.

The CI matrix therefore chooses Xvfb screen sizes large enough that the existing 90% monitor limit does not rescale any target:

```text
16:9  → window 1152×648  → Xvfb 1280×720
4:3   → window 800×600    → Xvfb 1280×960
21:9  → window 1680×720   → Xvfb 1920×1080
```

## Implementation

Capture mode uses two optional environment variables:

- `ASCENDENDO_CAPTURE_WINDOW_WIDTH`
- `ASCENDENDO_CAPTURE_WINDOW_HEIGHT`

Normal gameplay is unchanged when these variables are absent. The validator propagates the target dimensions to the game and verifies that the produced PPM has the same framebuffer dimensions.

## Evidence boundary

This proves that the requested viewport dimensions reach the actual rendered framebuffer. It does **not** prove subjective legibility, camera feel, or final art quality.

Those properties remain part of later visual/playtesting work.
