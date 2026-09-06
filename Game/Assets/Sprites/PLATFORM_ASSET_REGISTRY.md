# ASCENDENDO — Platform Asset Candidate Registry

Status: research candidates with explicit human review decisions. No external platform family is shipped merely by being registered here.

## Canonical metadata boundary

This file is the canonical human-auditable registry for candidate provenance, review state and the exact-file integration identity required for promotion.

For every candidate that is promoted to runtime, the same registry entry must bind the approval to:

```text
asset_id
runtime_path
content_sha256
```

`runtime_path` must name one concrete repository/runtime file. `content_sha256` must be the SHA-256 of that exact file content, represented as exactly 64 hexadecimal characters. Until those two fields are populated from an actually staged file, the candidate is not runtime-promotable.

The C++ selector remains metadata-only: it consumes the identity supplied by the registry/integration layer and never performs filesystem I/O or pixel inspection.

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
- `runtime_path`: `UNPOPULATED — exact binary not yet staged`
- `content_sha256`: `UNPOPULATED — exact binary not yet staged`
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
- `runtime_path`: `UNPOPULATED — exact binary not yet staged`
- `content_sha256`: `UNPOPULATED — exact binary not yet staged`
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
- `runtime_path`: `UNPOPULATED`
- `content_sha256`: `UNPOPULATED`
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
- `runtime_path`: `UNPOPULATED — exact binary not yet staged`
- `content_sha256`: `UNPOPULATED — exact binary not yet staged`
- local modifications: none

## ART-006 — FLAG / GOAL research candidate F1
- `asset_id`: `doughnutdev.cartoon-platformer-goal-flag`
- source: `https://doughnutdev.itch.io/platformer-assetpack`
- creator: DoughnutDev
- licence: Creative Commons Attribution-NoDerivatives 4.0 International; author requires credit and permits commercial/non-commercial use and modification, but prohibits redistribution/resale of the asset itself
- asset/version identifier: `PlatformerAssets.zip` (12 kB download listed on source page; exact file to be acquired and hashed before promotion)
- retrieval date: 2026-09-06
- concrete size evidence: source page states the flag is `16×80`, i.e. five 16×16 units stacked vertically
- role: level goal / end-point flag reference candidate
- human review: `REWORK`
- status: `REWORK — useful concrete goal-flag candidate, but not CC0; exact binary and per-file identity are not yet acquired, and no visual approval has been granted`
- `runtime_path`: `UNPOPULATED`
- `content_sha256`: `UNPOPULATED`
- local modifications: none

## ART-007 — PROPS research candidate P1
- `asset_id`: `aldrin572.tiny-forest-16x16-props`
- source: `https://aldrin572.itch.io/tiny-forest-16x16-platformer-asset-pack`
- creator: Aldrin572
- licence: CC0 (verified against the source page)
- asset/version identifier: `Tiny_Forest_AssetPack_16x16.zip` (6.9 kB download listed on source page; exact files to be acquired and hashed before promotion)
- retrieval date: 2026-09-06
- concrete size evidence: source page states a 16×16 terrain grid and lists scenery props (tree, bush, rock, wooden sign, box); individual prop PNG dimensions are not stated
- role: ambient/environment decoration without gameplay collision semantics
- human review: `REWORK`
- status: `REWORK — strong CC0 research candidate, but exact prop file dimensions are not yet verifiable and no visual approval has been granted`
- `runtime_path`: `UNPOPULATED`
- `content_sha256`: `UNPOPULATED`
- local modifications: none

## Other approval-inbox IDs

- `ART-001` actual player sprite: concrete PNG is 32×32 px, so human review is `APPROVE`; required player size remains `32×32 px`.
- `ART-004` remains `REWORK` because the canonical module is 16×16 while the candidate is 18×18.

## Size gate

The current project presentation module is `16×16 px`. Platform and terrain tiles must therefore be `16×16 px`; objects spanning multiple cells must use integer multiples of `16 px` on each axis. The current player sprite is a concrete `32×32 px` asset. Goal flags may compose multiple `16×16` units.

Human approval rule applied on 2026-09-04:

```text
verifiable + compatible size -> APPROVE
missing/unverifiable size -> REWORK
incompatible size -> REWORK
```

For `ART-006` and `ART-007`, the existence of package-level size claims is not treated as proof that each concrete runtime file has the required dimensions.

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

Required exact identity before shipping:

```text
runtime_path
content_sha256 (exactly 64 hexadecimal characters)
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

This registry records candidate provenance, review state and the canonical exact-file identity fields. It does not fetch, convert, import, hash or silently ship external assets. Exact identity values are only populated after the concrete binary is acquired, inspected and staged in `ART_APPROVAL_INBOX` or its approved runtime destination.
