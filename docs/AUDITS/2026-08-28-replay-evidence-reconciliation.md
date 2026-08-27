# ASCENDENDO — Replay evidence reconciliation

**Date:** 2026-08-28  
**Baseline:** `main` at `ff99571a819ed56b71a54a8187699b940618d9ab`  
**Studies checked:** 2026-08-27 replay matrix + input-contract study  
**Classification:** AUDIT / CURRENT-STATE RECONCILIATION

## Finding

The replay Studies currently classify replay determinism as open because they were written before PR #88/#90. The current `main` has since introduced `TickInput` and has executable tick-by-tick replay tests.

## Revalidated evidence

- **D2 continuous command determinism:** demonstrated at the simulation layer by repeated fixed-step execution with the same `TickInput` sequence and full relevant player/physics snapshot comparison.
- **D3 edge-event representation:** `TickInput` carries `jumpPressed` and `jumpReleased`; ReplayManager stores the complete semantic tick input.
- **D4 semantic frame repartition:** demonstrated for an already-defined identical `TickInput` sequence under different external grouping. This is **not** evidence that live GLFW sampling is frame-rate independent.
- **D5 state replay:** demonstrated tick-by-tick in `Tests/Unit/test_replay.cpp` across player transform/velocity, grounded state, jump state and physics accumulator.
- **D6 terminal/result replay:** not demonstrated. `ReplayManager` does not own `GameSession` terminal state, elapsed time, campaign progression, or run-record result.
- **D7 persistence:** explicitly outside the current WP and remains open.

## Decision

Do not redesign ReplayManager merely to close D6/D7 inside Gate 9.6. Keep the established boundary: the current proof establishes deterministic replay of the semantic simulation input/state subset, while terminal/result semantics and persistence remain separate properties.

The remaining Gate priority after this reconciliation is collision-order determinism, unless a new higher-level Study supersedes it.

## Non-equivalences

```text
TickInput replay evidence
≠
live input frame-rate independence
≠
terminal/result replay
≠
persistent replay format
```

## Next decision

Proceed to collision-order determinism investigation with explicit multiple-contact/permutation evidence before changing physics code.
