# ASCENDENDO — Compositor local-lattice audit — 2026-08-31

## Study update checked

The current `PROJECT-STUDIES/ASCENDENDO` compositor study defines a local 16x16 lattice per semantic platform region. It explicitly rejects global-grid snapping and requires non-grid-aligned `LevelData` origins to remain unchanged.

## Main-state finding

After the structural compositor pilot, `ASCENDENDO/main` had enough machinery to classify explicit semantic cells but not to derive those cells from the actual continuous platform origin without inventing a global snapping rule.

## Change

`PlatformRegion` + `composeRegion()` now provide a presentation-only local lattice:

```text
platform origin (continuous)
        ↓
local integer cell indices
        ↓
world-space cell origins = origin + 16*index
```

No gameplay geometry is rewritten.

## Verified property

The real `Inicio` platform at `(436, 89, 176, 16)` maps to 11 local cells starting at world x=436 and ending at x=596. The origin remains non-grid-aligned.

## Failure/validation boundary

Dimensions not divisible by 16 are rejected instead of silently cropped or snapped.

Cross-platform adjacency remains deferred because the Study requires an explicit presentation-only tolerance, and no canonical value is currently defined.

## Classification

**DIRECT EVIDENCE:** the Study now specifies local continuous-boundary rasterization.

**IMPLEMENTED:** local lattice expansion and origin preservation.

**NOT IMPLEMENTED:** cross-platform adjacency tolerance, asset selection, fallback rendering, renderer integration, golden-image validation, and human review.
