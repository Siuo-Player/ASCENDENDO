# ASCENDENDO — Platform Asset Candidate Registry

Status: research candidates only. No external platform family is approved or shipped by this registry.

## Candidate A — Kenney Pixel Line Platformer
- `asset_id`: `kenney.pixel-line-platformer`
- source: `https://kenney.nl/assets/pixel-line-platformer`
- mirror: `https://opengameart.org/content/pixel-line-platformer`
- licence: CC0 (reported by the current PROJECT-STUDIES asset shortlist)
- nominal tile size: 16×16
- role: platformer terrain / starter platform vocabulary
- status: `REVIEW`
- local path: not staged
- local modifications: none

## Candidate B — HDST Platformer Pack 16×16
- `asset_id`: `hdst.platformer-pack-16x16`
- source: `https://opengameart.org/content/platformer-pack-16x16`
- licence: CC0 (reported by the current PROJECT-STUDIES asset shortlist)
- nominal tile size: 16×16
- role: platformer terrain / comparison candidate
- status: `REVIEW`
- local path: not staged
- local modifications: none

## Candidate C — Kenney Pixel Platformer
- `asset_id`: `kenney.pixel-platformer`
- source: `https://kenney.nl/assets/pixel-platformer`
- mirror: `https://opengameart.org/content/pixel-platformer-0`
- licence: CC0 (reported by the current PROJECT-STUDIES asset shortlist)
- nominal tile size: 18×18 according to the cited shortlist
- role: reserve/reference while the project canonical module remains 16×16
- status: `REFERENCE`
- local path: not staged
- local modifications: none

## Candidate D — Generic Platformer Tileset (16×16) + Background
- `asset_id`: `generic.platformer-tileset-16x16`
- source: `https://opengameart.org/content/generic-platformer-tileset-16x16-background`
- licence: CC0 (reported by the current PROJECT-STUDIES asset shortlist)
- nominal tile size: 16×16
- role: comparison / background candidate
- status: `REVIEW`
- local path: not staged
- local modifications: none

## Promotion record

A candidate may move beyond `REVIEW` only after the Asset Selection Decision Gate is executed against the canonical `Inicio` scenes and the hard constraints are evidenced.

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
