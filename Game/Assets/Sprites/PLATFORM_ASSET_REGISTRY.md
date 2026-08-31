# ASCENDENDO — Platform Asset Candidate Registry

Status: research candidates only. No external platform family is approved or shipped by this registry.

## Candidate A — Kenney Pixel Line Platformer
- `asset_id`: `kenney.pixel-line-platformer`
- source: `https://kenney.nl/assets/pixel-line-platformer`
- mirror: `https://opengameart.org/content/pixel-line-platformer`
- creator: Kenney
- licence: CC0 (verified against the official Kenney asset page)
- asset/version identifier: official asset page; 1.0 released in 2021
- retrieval date: 2026-08-31
- nominal tile size: 16×16
- role: platformer terrain / starter platform vocabulary
- status: `REVIEW`
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
- role: platformer terrain / comparison candidate
- status: `REVIEW`
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
- role: reserve/reference while the project canonical module remains 16×16
- status: `REFERENCE`
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
- role: comparison / background candidate
- status: `REVIEW`
- local path: not staged
- local modifications: none

## Provenance verification boundary

The registry records evidence gathered for candidate comparison. A field being populated does not approve an asset. Candidate promotion still requires the project-specific decision gate and product evidence.

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
REJECT
NO WINNER
```

`NO WINNER` is valid and must not trigger relaxed acceptance criteria.

## Boundary

This registry records candidate provenance and review state only. It does not fetch, approve, import, convert, or ship external assets.
