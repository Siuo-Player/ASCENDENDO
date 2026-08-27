# ASCENDENDO — Collision-order determinism investigation

**Date:** 2026-08-28  
**Baseline:** `main` at `ff99571a819ed56b71a54a8187699b940618d9ab`  
**Classification:** AUDIT / INVESTIGATION — no implementation change

## Question

Is `Level::resolveCollision()` deterministic with respect to the order of `Level` platform storage when more than one platform can affect the same body during a step?

## Current implementation observation

`Level::resolveCollision()` iterates `m_platforms` in stored order. It recomputes the body AABB after each collision and mutates position, velocity and grounded state immediately. Therefore collision resolution is sequential rather than an explicitly order-independent contact reduction.

This is an architectural risk, not yet a demonstrated defect.

## Evidence currently available

Existing tests cover isolated vertical contacts, ceiling response, ground fallback and isolated left/right lateral contacts. They do not establish a permutation property for multiple simultaneous/overlapping contacts.

No claim is made that changing platform order currently changes the outcome.

## Decision

Before changing physics code, create a minimal permutation test:

```text
same initial PhysicsBody
same set of colliding platforms
same simulation step

order A: [P1, P2, ...]
order B: [P2, P1, ...]

compare resulting logical state
```

The test must compare at least position, velocity and grounded state. If the outcome differs for geometrically equivalent contact sets, the result is a concrete non-determinism/order-dependence finding.

If no difference is found, do not add an abstraction merely to enforce commutativity; retain the empirical result and document the remaining assumption.

## Out of scope

- broad physics rewrite;
- changing collision response coefficients;
- spatial partitioning/optimization;
- gameplay tuning;
- RenderSnapshot;
- declaring collision determinism proven from isolated-contact tests.

## Next decision

Only after the permutation experiment: either document the observed order dependence and design a focused resolution contract, or record the tested invariance and move to the next Gate gap.
