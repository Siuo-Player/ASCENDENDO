# ASCENDENDO — Work Package: local 16x16 compositor lattice

## Basis

The latest `PROJECT-STUDIES/ASCENDENDO/2026-08-30-16x16-semantic-compositor-v2.md` defines the 16x16 visual lattice as local to each semantic platform region. World coordinates remain continuous and must not be globally snapped.

## Property

For a modular platform region:

```text
width  = 16 × kx
height = 16 × ky

cell(i,j).x = region.x + 16*i
cell(i,j).y = region.y + 16*j
```

The presentation layer must preserve the continuous region origin.

## Implementation

Added `PlatformRegion` and `composeRegion()` to `gfx::compositor`.

The implementation:

- validates finite positive origins;
- validates positive dimensions that are exact multiples of 16 within a narrow floating-point tolerance;
- builds the visual lattice using local integer cell indices;
- composes topology using the existing local-neighbour classifier;
- returns world-space cell origins without snapping or mutating the source coordinates.

## Evidence

The `Inicio` platform at:

```text
x=436
y=89
width=176
height=16
```

produces 11 visual cells whose world origins are `436, 452, ..., 596`.

## Deliberate boundary

Cross-platform adjacency is **not** implemented in this tranche. The Study requires an explicit presentation-only contact/adjacency tolerance, but no canonical tolerance value is currently specified. Inventing one would silently create a new semantic rule.

World-to-grid snapping, asset selection, fallback rendering and `WorldRenderer` integration remain deferred.

## Promotion boundary

This tranche establishes:

```text
semantic region
→ local modular lattice
→ continuous world-space presentation coordinates
```

It does not establish `VALIDATED` or `GENERALIZED` compositor status.
