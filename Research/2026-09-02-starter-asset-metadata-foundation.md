# Starter asset metadata foundation — 2026-09-02

## Scope

This tranche implements only the semantic metadata boundary identified by the PROJECT-STUDIES starter visual integration work package. It does **not** import external artwork, choose a platform candidate, or claim visual validation.

## Evidence boundary

Current `Game/Assets/Sprites/` contains the project player PNG, its `.pixil` source, and the research-only platform candidate registry. External platform candidates remain un-staged and unapproved.

The existing player PNG is 32×32 pixels. Its gameplay/display contract uses a logical 16×16 footprint.

## Implemented contract

`Game/Assets/StarterAssetCatalog.h` provides:

- semantic roles for the canonical starter vocabulary;
- explicit `Runtime` vs `Reserved` availability;
- asset id, runtime/source paths and provenance identifier;
- licence/provenance class;
- nominal pixel size, logical draw size and anchor;
- flip permission and gameplay-critical classification;
- a deliberately tiny runtime catalogue containing only the existing player asset;
- an explicit reserved-role list so missing assets cannot be mistaken for shipped content.

## Why this is bounded

The contract creates the bridge between curated artwork and future rendering/integration without coupling gameplay geometry to sprites and without adding an asset-loading framework.

External candidate assets remain outside the runtime catalogue until the provenance, technical and human-visual gates pass.

## Validation target

The unit tests verify that:

1. the existing player is represented with its known 32×32 source/runtime image size and 16×16 logical draw size;
2. missing starter roles remain explicitly reserved;
3. the runtime catalogue does not silently promote unverified artwork;
4. the current player metadata stays presentation-only.

## Not proven by this tranche

This does not prove sprite readability, contact-edge alignment, scaling quality, layer order, or human visual acceptance. Those require rendered deterministic captures and human review with the appropriate gameplay-critical assets.
