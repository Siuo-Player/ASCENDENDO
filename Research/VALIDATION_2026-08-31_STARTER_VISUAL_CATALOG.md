# ASCENDENDO — Starter Visual Catalog

**Date:** 2026-08-31  
**Work package:** first metadata-backed runtime bridge for the current starter visual asset  
**Production anchor:** `176b37266518c727622e872d3e4da3b0b334d96c`

## Basis

The current PROJECT-STUDIES work package calls for a small metadata-backed visual catalogue between curated artwork and the existing renderer. It explicitly rejects a premature general asset pipeline and requires the runtime chain to preserve provenance, semantic role, logical footprint and anchor information.

The only currently staged gameplay sprite is:

```text
Game/Assets/Sprites/personagem.png
```

Its source artwork is:

```text
Game/Assets/Sprites/Source/personagem.pixil
```

The PNG is 32×32. The current sprite vertex contract places `objPos` at the lower-left of the logical quad, and the vertical slice's intended player footprint is 16×16.

## Implementation

Added:

```text
Game/Graphics/StarterVisualCatalog.h
Game/Graphics/StarterVisualCatalog.cpp
Tests/Unit/test_starter_visual_catalog.cpp
```

and changed `PresentationRuntime` so the player sprite path is consumed through the catalog metadata rather than passing the raw path directly to `SpriteRenderer`.

The metadata records:

```text
asset id
runtime path
source path
licence/provenance state
semantic role
32×32 nominal source size
16×16 logical draw size
bottom-left anchor
horizontal flip policy
gameplay-critical flag
visual contact edge
```

## Provenance boundary

The source `.pixil` file identifies Pixilart as the application/website but does not establish a project-level licence statement for this artwork. Therefore the catalog deliberately records:

```text
licence = UNVERIFIED
provenance = Unverified
```

This is intentional. The catalogue does not convert an existing runtime asset into an approved/shipping asset by assumption.

## Scope boundary

This tranche does **not**:

- import Candidate A or B;
- fabricate platform PNGs;
- approve any platform family;
- change player collision geometry;
- add atlas/animation infrastructure;
- introduce a generic asset validator or asset pipeline;
- generalize the compositor;
- claim human visual approval.

## Validation

Unit tests assert the metadata contract and, critically, assert that missing licence/provenance evidence remains `Unverified` rather than silently becoming approved.

The exact head SHA `03d02b3e23d65c9537caa6898e11dfcab410b2e9` passed all required CI workflows before merge:

```text
Tests — run 1438 — SUCCESS
Windows — run 514 — SUCCESS
Deterministic Capture Evidence — run 34 — SUCCESS
```

The deterministic capture workflow also produced the declared per-level PPM evidence artifacts for the tested viewports.

## Merge record

```text
PR: #205
head: 03d02b3e23d65c9537caa6898e11dfcab410b2e9
base: 176b37266518c727622e872d3e4da3b0b334d96c
merge commit: 037e48b5647e09ec33d63fb18e6a652770ae999b
main after merge: 037e48b5647e09ec33d63fb18e6a652770ae999b
```

The post-merge `main` ref was independently fetched and confirmed at the merge commit.

## Next dependency / blocker

The next roadmap unit is the `Inicio` platform corpus and compositor-pilot evidence. External Candidates A/B remain `REVIEW` and are not staged. The current GitHub writing interface used for this tranche cannot safely create/commit arbitrary binary PNG trees, so no external or synthetic platform binary has been introduced merely to satisfy the roadmap mechanically.

When a verifiable platform binary source is available through a suitable repository/file transfer path, the next evidence chain is:

```text
platform binary
→ provenance verification
→ metadata catalogue entry
→ controlled Inicio composition
→ deterministic capture
→ structural + physical↔visual QA
→ human review
```

Until that prerequisite exists, do not claim A/B selection, platform-family approval, or golden-corpus promotion.
