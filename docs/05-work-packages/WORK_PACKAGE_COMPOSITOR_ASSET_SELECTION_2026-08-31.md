# ASCENDENDO — Work Package: deterministic platform asset selection

## Basis

The current `PROJECT-STUDIES/ASCENDENDO` compositor v2 contract defines hard candidate eligibility and deterministic ranking before any asset can be used by the compositor. The result may be APPROVE/LIMIT/REFERENCE/REJECT; no-winner is a valid outcome.

## Property

Given a platform presentation request and a reviewed candidate set:

```text
eligible candidates only
→ explicit precedence metadata
→ deterministic ranking
→ stable asset id tie-break
```

No candidate may be selected merely because it visually resembles the requested role.

## Implementation

Added `gfx::assets::selectBestPlatformAsset()` with a compositor-specific candidate model.

Hard eligibility requires:

- semantic topology coverage;
- exact requested footprint;
- compatible material metadata;
- permitted mirroring when requested;
- exact supported scale;
- verified provenance;
- pixel-scale safety;
- contact readability;
- gameplay decoupling;
- acceptable seams.

Ranking uses the documented order:

```text
1. topology match
2. footprint match
3. exact material match over generic
4. variant rank
5. lexical asset id
```

If no candidate passes the hard constraints, the selector returns `std::nullopt` rather than inventing a winner.

## Tests

```text
T16 manifest order invariance
T17 ineligible candidate rejection
T18 explicit lexical tie-break stability
```

Additional tests cover generic-vs-exact material and mirrored requests.

## Boundary

This tranche does **not**:

- load PNGs or other files;
- prove licence/provenance of real external assets;
- choose Kenney/HDST/project-created art;
- integrate assets into `WorldRenderer`;
- assign final asset promotion states;
- perform visual or human review.

The selector consumes review metadata; a future asset registry will be responsible for producing that metadata from verified sources.

## Promotion

```text
selector contract implemented
→ asset registry / candidate provenance
→ A/B visual comparison
→ human review
→ APPROVE / LIMIT / REFERENCE / REJECT / NO WINNER
```

The compositor remains structurally reusable but is not yet visually validated or generalized.
