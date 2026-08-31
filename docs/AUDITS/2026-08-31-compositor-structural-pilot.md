# ASCENDENDO — Compositor structural pilot audit — 2026-08-31

## Evidence source

The current `PROJECT-STUDIES/ASCENDENDO` research layer was updated on 2026-08-31 with a bounded structural fixture pack and a generalization gate for the proposed 16x16 semantic compositor.

The Study identifies a direct coverage gap: existing campaign levels mainly exercise horizontal rectangular platforms and therefore do not cover corners, junctions, material transitions or isolated terrain-cell cases.

## Current production finding

`ASCENDENDO/main` currently has no dedicated semantic platform compositor. `Game/Graphics/WorldRenderer.cpp` emits each `RenderSnapshot.platforms` rectangle directly through `ShapeRenderer`.

Therefore the first production tranche must be a bounded structural pilot rather than a full asset-driven compositor.

## Decision

Introduce a pure presentation-side structural classifier over an explicit semantic cell representation. Do not infer or invent a world-coordinate snapping policy while `LevelData` still permits continuous x/y coordinates.

## Invariants being established

```text
same semantic input → same ordered output
local neighbours determine topology
material changes are explicit boundaries
duplicate occupancy is rejected
presentation data remains separate from gameplay geometry
```

## Deferred

```text
world → 16x16 rasterization policy
asset ranking/promotion
visual macro/modular selection
missing-asset rendering fallback
WorldRenderer integration
golden-image claims
human readability claims
ZigZag generalization
```

## Status

**STRUCTURAL PILOT — IMPLEMENTED IN BRANCH, NOT YET MERGED.**

Promotion to validated/generalized remains blocked on the Study's structural, visual and experiential gates.
