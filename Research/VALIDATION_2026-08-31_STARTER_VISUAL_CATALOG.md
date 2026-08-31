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

The existing CI then remains responsible for Linux correctness, ASan+UBSan, Windows and deterministic capture.

## Next dependency

Platform A/B visual integration remains blocked on obtaining/staging the actual candidate binaries with verifiable provenance. Once those binaries are available, this catalogue can be extended with platform gameplay-critical entries and the controlled `Inicio` A/B capture can proceed.
