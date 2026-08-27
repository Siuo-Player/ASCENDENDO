# ASCENDENDO — Input/tick determinism revalidation

**Date:** 2026-08-27  
**Source:** `Siuo-Player/Siuo-Player-PROJECT-STUDIES` PR #12  
**Role:** 9.6 Base Engineering Gate evidence

## Finding

The simulation uses fixed steps, but input is sampled at frame scope. `GameSession::update()` receives one `InputManager` state for an outer frame, and `SimulationOrchestrator::advance()` can execute multiple fixed simulation steps using that same state.

This is not evidence that ordinary gameplay is incorrect. It is a concrete evidence gap for deterministic replay and tick-exact reproduction of edge-triggered input.

## Required contract

A future deterministic replay boundary should map input explicitly to simulation ticks:

```text
raw/window input
→ frame aggregation
→ tick input mapping
→ SimulationOrchestrator
→ Player / PhysicsWorld
```

Each simulation tick should consume a value representing that tick's held/pressed/released state rather than inferring edge timing from rendered-frame boundaries.

## Priority

**P0 / Base Engineering Gate evidence gap.** The work should remain separate from rendering/`RenderSnapshot` and should not replace the existing configurable key-binding system.

## Engineering trigger

Do not introduce the abstraction solely for aesthetic reasons. Implement it when replay/regression evidence becomes a required gate or when a deterministic tick-by-tick test needs the contract.

## Provenance

This document revalidates Studies PR #12 against the current ASCENDENDO implementation. The study layer is read-only with respect to implementation.
