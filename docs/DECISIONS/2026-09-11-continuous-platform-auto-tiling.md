# Decision — Continuous-platform 16×16 auto-tiling

**Data:** 2026-09-11  
**Estado:** accepted product decision

## Decision

ASCENDENDO authors platforms as continuous material geometry, not as manually painted tiles.

- World position is pixel-perfect and is **not** locked to a global 16×16 grid.
- Platform width and height are both positive multiples of 16 px for canonical visual composition.
- A platform may therefore start at an arbitrary coordinate such as `x=347` and extend in 16 px increments.
- The renderer creates a local 16×16 visual lattice from the platform geometry.
- The compositor selects curated sprites using topology, material, adjacency, exposed/internal surfaces, corners and joins.
- Different material regions may compose visually when their actual geometry contacts.
- Edge treatments such as grass must only appear where their visual context is valid.
- Deterministic controlled variation may choose among eligible sprites to reduce repetition.
- Missing or ineligible variants fall back deterministically.
- Auto-tiling is presentation-only and never changes collision or gameplay geometry.

## Consequence

The author thinks in terms of a single continuous `Dirt`, `Stone`, etc. platform. Tile selection is a rendering decision.

`16×16` is the visual construction unit, **not** a global world-placement restriction.
