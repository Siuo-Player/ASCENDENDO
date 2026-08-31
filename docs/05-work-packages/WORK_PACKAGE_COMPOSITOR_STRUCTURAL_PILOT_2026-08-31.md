# ASCENDENDO — Work Package: 16x16 structural compositor pilot

## Basis

The live `PROJECT-STUDIES/ASCENDENDO` layer now promotes a bounded 16x16 semantic compositor pilot before general visual integration. The proposed structural fixture corpus covers isolated cells, straight runs, corners, stepped profiles, T/cross junctions, material boundaries, macro-vs-modular equivalence and missing-art fallback.

## Property

Semantic occupancy must map deterministically to a local topology classification using only neighbouring cells and material identity.

## Implementation

Added `gfx::compositor::compose()` with:

- semantic `GridCell {x, y, material}` input;
- four-neighbour mask (`Left`, `Right`, `Up`, `Down`);
- deterministic topology classes:
  - isolated;
  - left/right end;
  - interior;
  - vertical edge;
  - corner;
  - junction;
  - material boundary;
- deterministic ordering by semantic `(y, x)`;
- duplicate occupancy rejection.

## Deliberate boundaries

This pilot does **not** yet:

- rasterize continuous `LevelData` world coordinates into semantic cells;
- choose artwork assets;
- mutate `RenderSnapshot` or gameplay geometry;
- change `WorldRenderer` draw behaviour;
- introduce a generic asset/texture manager;
- claim final visual identity or human readability.

The missing world->semantic-grid contract is intentional: the current LevelData model permits continuous `x/y`, while no snapping policy has been justified yet.

## Validation coverage

The structural test covers:

```text
F01 isolated cell
F02 3x1 straight
F03 left end
F04 right end
F05/F06 corner topology
F07 stepped profile
F08 T junction
F09 cross junction
F10 material boundary
F11 macro/modular ordering invariance
F12 duplicate occupancy rejection
```

Additional determinism coverage verifies identical input yields identical ordered output.

## Acceptance boundary

This tranche can only establish:

```text
SPECIFIED
→ STRUCTURAL PILOT IMPLEMENTED
```

It does not promote the compositor to `VALIDATED` or `GENERALIZED` because the Study requires visual inspection, physical/visual contact validation, and second-level `ZigZag` generalization before those claims.
