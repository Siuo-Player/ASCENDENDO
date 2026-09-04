# ASCENDENDO — Concrete asset preview evidence

This manifest makes the approved/rework visual candidates inspectable without importing them into runtime.
It is evidence only; it does not itself promote an asset.

## ART-002 — Kenney Pixel Line Platformer
- decision: `APPROVE`
- required project tile size: `16×16 px`
- creator: Kenney
- licence: CC0
- official source: https://kenney.nl/assets/pixel-line-platformer
- OpenGameArt mirror: https://opengameart.org/content/pixel-line-platformer
- concrete preview: https://kenney.nl/media/pages/assets/pixel-line-platformer/63aa003270-1668793749/preview.png
- evidence: official page states tile size `16 × 16` and CC0.
- intended use: platform/terrain visual vocabulary.

## ART-003 — HDST Platformer Pack 16×16
- decision: `APPROVE`
- required project tile size: `16×16 px`
- creator: hdst
- licence: CC0
- source: https://hdst.itch.io/platformer-pack-16x16
- OpenGameArt mirror: https://opengameart.org/content/platformer-pack-16x16
- concrete preview: https://img.itch.zone/aW1hZ2UvNTA2NjE2LzI2MjYxNjgucG5n/original/ayfVDF.png
- package: `PlatformerPack_16x16.zip`
- intended use: alternative platform/terrain visual candidate.

## ART-004 — Kenney Pixel Platformer
- decision: `REWORK`
- nominal tile size: `18×18 px`
- required project tile size: `16×16 px`
- creator: Kenney
- licence: CC0
- official source: https://kenney.nl/assets/pixel-platformer
- OpenGameArt mirror: https://opengameart.org/content/pixel-platformer-0
- reason: the verified nominal tile size is `18×18`; do not integrate at that scale. A 16×16-compatible variant or conversion is required before re-review.

## ART-005 — Generic Platformer Tileset (16×16) + Background
- decision: `APPROVE`
- required project tile size: `16×16 px` for tiles; composed elements in integer multiples of `16 px`.
- creator: etqws3
- licence: CC0
- source: https://opengameart.org/content/generic-platformer-tileset-16x16-background
- concrete preview page: https://opengameart.org/content/generic-platformer-tileset-16x16-background
- concrete files listed by source:
  - `[DB32](Generic Platformer)(Tileset).png`
  - `[DB32](Generic Platformer)(Mountains).png`
  - `[DB32](Generic Platformer)(Clouds).png`
- intended use: terrain/background comparison candidate.

## ART-001 — Current player sprite
- decision: `APPROVE`
- required project size: `32×32 px`
- concrete repository file: `Game/Assets/Sprites/personagem.png`
- editable source: `Game/Assets/Sprites/Source/personagem.pixil`

## ART-006 — Flag / goal
- decision: `REWORK`
- required base unit: `16×16 px`
- status: no concrete candidate supplied yet.

## ART-007 — Props / environment
- decision: `REWORK`
- required base modules: `16×16 px`; larger objects must use integer multiples of `16 px` per axis.
- status: no concrete candidate supplied yet.

## Promotion boundary

A candidate marked `APPROVE` here is approved by the product owner under the recorded size gate, but runtime integration still requires:

1. exact file identification;
2. preserved provenance;
3. technical compatibility with the renderer/compositor;
4. deterministic capture evidence after integration.

No gameplay collision, physics, or level geometry is changed by this manifest.
