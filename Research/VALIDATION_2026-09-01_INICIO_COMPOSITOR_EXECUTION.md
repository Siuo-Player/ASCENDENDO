# ASCENDENDO — `Inicio` Compositor Execution Corpus

**Date:** 2026-09-01  
**Work package:** real-content execution coverage for the bounded 16×16 compositor  
**Production anchor:** `20b1a800cec6f2de7889f3487a509af7b415e418`

## Basis

The PROJECT-STUDIES execution corpus defines four real `Inicio` fixtures:

```text
R1  full 640×16 floor
R2  176×16 platform at (436,89)
R3  176×16 platform at (148,174)
R4  176×16 platform at (436,259)
```

The compositor must derive a local 16×16 visual lattice while preserving continuous world coordinates and must not write presentation changes back into gameplay geometry.

## Implementation

Added `Tests/Unit/test_inicio_compositor_execution.cpp` covering:

```text
R1–R4 exact lattice footprint
continuous world origin preservation
full-width floor coverage
non-mutation of gameplay-space input
```

Expected lattice sizes are:

```text
R1 → 40 cells
R2 → 11 cells
R3 → 11 cells
R4 → 11 cells
```

For each region, the first visual cell starts at the exact world-space origin and successive cells advance by 16 pixels. No global snapping is introduced.

## Important scope boundary

This tranche is intentionally **structural real-content evidence**, not visual asset approval.

It does not:

- select Candidate A/B;
- approve an external family;
- import PNGs;
- define an adjacency epsilon;
- change campaign geometry;
- integrate `WorldRenderer` with platform sprite assets;
- claim visual or human validation.

## Acceptance meaning

A passing suite establishes that the existing bounded compositor can execute the canonical `Inicio` platform dimensions into its local lattice without mutating the gameplay-space representation.

It does **not** yet establish the complete Study rendering contract:

```text
LevelData
→ semantic occupancy
→ topology
→ approved candidate
→ emitted visual instances
```

because the actual candidate binaries and the final asset decision are still absent.

## Next dependency

The remaining product-side blocker is obtaining a safely transferable, provenance-verifiable platform asset family. After that prerequisite, the next tranche should connect the verified metadata entries to controlled `Inicio` visual composition and capture, while preserving the existing player/camera/geometry conditions.
