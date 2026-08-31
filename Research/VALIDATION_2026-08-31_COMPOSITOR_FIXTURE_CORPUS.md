# ASCENDENDO — Compositor Structural Fixture Corpus v1

**Date:** 2026-08-31  
**Work package:** explicit structural fixture corpus for the bounded 16×16 compositor  
**Branch:** `test/compositor-fixture-corpus-20260831-main-sync`

## Basis

`PROJECT-STUDIES/ASCENDENDO/RESEARCH_INBOX/2026-08-30-compositor-structural-fixture-pack-v1.md` defines an implementation-ready synthetic fixture corpus independent of player-facing campaign content.

The related compositor coverage gate requires separate product and structural corpora and does not allow promotion to generalized behaviour until both are validated.

## Implementation

Added `Tests/Unit/test_platform_compositor_fixture_corpus.cpp` with an explicit F01–F12 fixture manifest.

Each fixture records:

```text
fixture_id
description
seed/config
expected fallback boundary
candidate requirement
semantic geometry
expected topology/neighbour results
```

The suite validates:

```text
F01 isolated cell
F02 straight interior/endpoints
F03 left end
F04 right end
F05 upper-left corner
F06 upper-right corner
F07 stepped profile
F08 T junction
F09 cross junction
F10 material transition
F11 macro/modular equivalence
F12 missing-candidate no-winner boundary
```

F11 additionally compares a 32×16 semantic region expanded by the local lattice with its equivalent two-cell modular composition, checking topology and world-origin preservation.

F12 deliberately stops at the existing selector contract: an absent candidate set must produce `std::nullopt`. This is a deterministic **no-winner selection boundary**, not a claim that a renderer fallback has been implemented.

## Scope boundary

This tranche does **not**:

- add external PNGs;
- approve Candidate A or B;
- define an adjacency/contact epsilon;
- add a generic validator or asset pipeline;
- promote the compositor from bounded pilot to generalized;
- modify campaign gameplay geometry.

## Validation target

The next acceptance evidence should execute the explicit fixture corpus through CI and then combine it with real `Inicio` visual integration evidence. Structural fixture success alone is insufficient for final visual promotion.

## Production anchor

This branch was reconstructed from the latest production `main` after the independent platform provenance update merged as PR #203. Current anchor at reconstruction is `261d8b45b7ede470514b74ecdbdd79139ecced14`.
