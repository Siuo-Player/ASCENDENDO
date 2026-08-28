# ASCENDENDO — Collision-order determinism revalidation

**Date:** 2026-08-28  
**Baseline:** `main` at `5edab5ba68fe9c9f1e36f8399c855b21bde211b6`  
**Classification:** AUDIT / CURRENT-STATE TEST DESIGN

## Finding

The previous collision-order PR (#97) was based on an older `main` and has been closed. The collision-order question remains valid because `Level::resolveCollision()` processes `m_platforms` sequentially and mutates body state during traversal.

## Current evidence boundary

The existing code and isolated collision tests do not establish permutation invariance for multiple contacts. No production physics change is justified before a reproducible order-dependent result.

## Test design

Use the same initial `PhysicsBody` and the same geometrically overlapping platform set in two storage orders. Compare the resulting logical state after `resolveCollision()`:

- position;
- velocity;
- grounded state.

A passing test establishes invariance only for the exercised geometry and initial state. A failure is direct evidence of order dependence for that case.

## Decision

Carry only the minimal permutation test onto the current `main` baseline. Do not introduce a solver rewrite, tuning, or abstraction unless a concrete counterexample is observed.

## Validation

Required before changing production physics:

- Linux/Clang normal check green;
- ASan/UBSan check green;
- Windows check green;
- test result interpreted with the bounded conclusion above.

## Remaining limitation

One or a small number of passing permutations cannot prove universal collision-order determinism. Broader coverage would require evidence-driven expansion after the first result.
