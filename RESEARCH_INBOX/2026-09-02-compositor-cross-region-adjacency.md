# ASCENDENDO — bounded compositor cross-region adjacency

## Scope

This tranche closes the compositor evidence gap identified by PROJECT-STUDIES T13–T15:

- preserve non-grid-aligned world-space origins;
- derive adjacency between two independent semantic regions from world-space rectangles;
- map contacts back to local 16×16 cells;
- keep tolerance presentation-only and explicit;
- reject invalid/non-modular regions without silently correcting them.

## Boundary

The implementation does not modify `LevelData`, collision geometry, player physics, reachability or asset state. It does not introduce a global visual grid, generic asset pipeline or renderer framework.

## Evidence basis

PROJECT-STUDIES `ec1713e906771cebece95862c37b3d981ce2a49f` defines local region anchoring, explicit cross-region geometric adjacency with tolerance, no global-grid snapping, and tests T13–T15.

## Implementation

`PlatformCompositor` now exposes a bounded `findRegionContacts(lhs, rhs, tolerance)` operation. It:

1. validates both regions as finite and 16×16 modular;
2. compares rectangle edges in continuous world coordinates;
3. checks positive overlap on the perpendicular axis;
4. maps every overlapping edge-cell pair to local coordinates;
5. returns opposing presentation neighbour masks.

Origins are never rewritten or snapped.

## Structural validation

Tests cover:

- fractional/non-grid-aligned origin preservation;
- horizontal contacts with different local origins;
- vertical contacts where one edge overlaps multiple local cells;
- explicit tolerance behaviour;
- disconnected regions;
- invalid/non-modular inputs producing no contact.

## Promotion boundary

This tranche can reach `STRUCTURALLY VALIDATED` after Linux normal, ASan/UBSan, Windows and deterministic capture gates pass. It does not imply human visual validation or asset approval.
