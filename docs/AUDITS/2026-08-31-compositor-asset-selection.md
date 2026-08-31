# ASCENDENDO — Compositor asset-selection audit — 2026-08-31

## Study update checked

`PROJECT-STUDIES/ASCENDENDO` now specifies:

```text
hard candidate eligibility
→ deterministic precedence/ranking
→ explicit no-winner outcome
```

The research also identifies T16–T18 as required selection tests and explicitly forbids filesystem/load-order/randomness-driven selection.

## Main-state finding

After the structural compositor and local-lattice tranches, ASCENDENDO had no asset-selection component or candidate metadata contract. Platform presentation still used direct shape rendering, so selecting artwork was not yet a runtime concern.

## Decision

Introduce a pure selection contract now, but keep source/provenance ingestion and rendering integration separate. This prevents the next implementation from inventing visual heuristics or coupling asset discovery to the renderer.

## Hard gates

A candidate is selectable only when its reviewed metadata satisfies provenance, pixel-scale, contact readability, gameplay decoupling and seam constraints, in addition to topology/footprint/material/orientation/scale compatibility.

## Ranking

Within eligible candidates the selector applies only the documented metadata ranking and falls back to lexical `assetId` as the final stable tie-break.

## No-winner behaviour

An empty eligible set returns `std::nullopt`. This is intentional: the runtime must not silently lower acceptance criteria to force a result.

## Deferred

```text
real asset registry
external source verification
A/B captures
human review
promotion decision
WorldRenderer integration
```

## Status

**IMPLEMENTED — selection contract only.**

The compositor is still not visually validated or generalized.
