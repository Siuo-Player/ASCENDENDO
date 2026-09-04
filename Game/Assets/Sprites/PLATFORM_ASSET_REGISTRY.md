# ASCENDENDO — Platform Asset Candidate Registry

Status: research candidates with explicit human review decisions. No external platform family is shipped merely by being registered here.

## Candidate A — Kenney Pixel Line Platformer
- `asset_id`: `kenney.pixel-line-platformer`
- source: `https://kenney.nl/assets/pixel-line-platformer`
- mirror: `https://opengameart.org/content/pixel-line-platformer`
- creator: Kenney
- licence: CC0 (verified against the official Kenney asset page)
- asset/version identifier: official asset page; 1.0 released in 2021
- retrieval date: 2026-08-31
- nominal tile size: 16×16
- required project size: 16×16
- role: platformer terrain / starter platform vocabulary
- human review: `APPROVE`
- status: `APPROVED FOR REVIEWED INTEGRATION`
- local path: not staged
- local modifications: none

## Candidate B — HDST Platformer Pack 16×16
- `asset_id`: `hdst.platformer-pack-16x16`
- source: `https://opengameart.org/content/platformer-pack-16x16`
- creator: hdst
- licence: CC0 (verified against the OpenGameArt source page)
- asset/version identifier: OpenGameArt node 107035 / `PlatformerPack_16x16.zip`
- retrieval date: 2026-08-31
- nominal tile size: 16×16
- required project size: 16×16
- role: platformer terrain / comparison candidate
- human review: `APPROVE`
- status: `APPROVED FOR REVIEWED INTEGRATION`
- local path: not staged
- local modifications: none

## Candidate C — Kenney Pixel Platformer
- `asset_id`: `kenney.pixel-platformer`
- source: `https://kenney.nl/assets/pixel-platformer`
- mirror: `https://opengameart.org/content/pixel-platformer-0`
- creator: Kenney
- licence: CC0 (verified against the official Kenney asset page)
- asset/version identifier: official asset page; 1.2 current page revision
- retrieval date: 2026-08-31
- nominal tile size: 18×18
- required project size: 16×16
- role: reserve/reference while the project canonical module remains 16×16
- human review: `REWORK`
- status: `REWORK — 18×18 não corresponde à grelha 16×16; requer variante/conversão 16×16 antes de integração`
- local path: not staged
- local modifications: none

## Candidate D — Generic Platformer Tileset (16×16) + Background
- `asset_id`: `generic.platformer-tileset-16x16`
- source: `https://opengameart.org/content/generic-platformer-tileset-16x16-background`
- creator: etqws3
- licence: CC0 (verified against the OpenGameArt source page)
- asset/version identifier: OpenGameArt source page published 2015-02-26; filenames identify the DB32 tileset/background PNGs
- retrieval date: 2026-08-31
- nominal tile size: 16×16
- required project size: 16×16 for tiles; multiples of 16×16 for composed elements
- role: comparison / background candidate
- human review: `APPROVE`
- status: `APPROVED FOR REVIEWED INTEGRATION`
- local path: not staged
- local modifications: none

## Other approval-inbox IDs

- `ART-001` actual player sprite: concrete PNG is 32×32 px, so human review is `APPROVE`; required player size remains `32×32 px`.
- `ART-006` FLAG: no concrete candidate/size was supplied, so human review is `REWORK`; required base unit is `16×16 px`.
- `ART-007` PROPS: no concrete candidate/size was supplied, so human review is `REWORK`; required base modules are `16×16 px`, with larger pieces in integer multiples of 16 px.

## Size gate

The current project presentation module is `16×16 px`. Platform and terrain tiles must therefore be `16×16 px`; objects spanning multiple cells must use integer multiples of `16 px` on each axis. The current player sprite is a concrete `32×32 px` asset.

Human approval rule applied on 2026-09-04:

```text
verifiable + compatible size -> APPROVE
missing/unverifiable size -> REWORK
incompatible size -> REWORK
```

## Provenance verification boundary

The registry records evidence gathered for candidate comparison and the human decision above. A populated field or `APPROVE` status does not by itself import an asset into the runtime. Candidate promotion still requires the project-specific integration gate and exact-file provenance.

Required provenance fields before shipping:

```text
source URL
creator
licence
asset/version identifier
retrieval date
local modification status
```

Required product evidence:

```text
pixel-scale safety
platform contact readability
player silhouette readability
route readability
seam/contact quality
no gameplay-semantic coupling
human visual review
```

Final outcomes:

```text
APPROVE
LIMIT
REFERENCE
REWORK
REJECT
NO WINNER
```

`NO WINNER` is valid and must not trigger relaxed acceptance criteria.

## Boundary

This registry records candidate provenance and review state only. It does not fetch, convert, import, or silently ship external assets.
