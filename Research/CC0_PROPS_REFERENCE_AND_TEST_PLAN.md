# ASCENDENDO — CC0 Props Reference and Visual Test Plan

**Date:** 2026-08-29
**Status:** curated reference candidates; runtime integration deferred until the sprite/texture presentation path is ready.

## 1. Goal

Build a small, coherent CC0 visual vocabulary that can be composed around the existing vertical platforming levels without introducing arbitrary external artwork.

The purpose of these references is twofold:

1. provide immediately usable candidate props once the sprite presentation path is enabled;
2. provide style/scale examples for future project-made assets.

No paid or licence-ambiguous asset is approved by this document.

## 2. Current world coordinate rule

ASCENDENDO currently uses a logical world of 640×360 pixels. The 16×16 unit is an **asset/module size**, not a mandatory world-position grid.

Therefore:

```text
platform width  = 16 × integer
platform height = 16 × integer
platform X/Y    = continuous pixel/world coordinates
```

This preserves authored jump geometry while making the visual terrain decomposable into 16 px modules.

## 3. Priority order

```text
P0 PLAYER
P1 PLATFORM / LANDING EDGE
P2 HAZARD / CHECKPOINT / GOAL
P3 LANDMARK
P4 NEAR DECORATION
P5 MID/FAR ENVIRONMENT
P6 SKY / ATMOSPHERE
```

Props must never visually compete with P0/P1.

## 4. Recommended CC0 prop families

### A. Forest micro-props — Pav Creations

Source: https://opengameart.org/content/forest-tileset-for-16-x-16

Author: Pav Creations
License: CC0

The source is explicitly tagged 16×16 and contains forest, trees, pine, bushes and rocks. This is a strong candidate for the first 16×16 prop vocabulary because its native unit already matches the project's modular art direction.

Suggested use:

```text
16×16
small bush cluster
small rock
pine segment
leaf/ground detail
```

Use primarily on L4/L5, not as the high-salience player/platform vocabulary.

### B. Nature surface props — Reactorcore

Source: https://opengameart.org/content/nature-props-surface-forest

Author: Reactorcore
License: CC0

The pack explicitly contains forest/nature pixel-art sprites including trees, bushes, rocks, fruit, plants and grass, with transparent PNG assets.

Suggested use:

```text
small grass clusters → L5
bushes                → L4/L5
rocks                 → L4/L5
plants                → L5
larger trees          → L4
```

These are especially useful for testing composition density rather than platform geometry.

### C. Forest objects — patvanmackelberg

Source: https://opengameart.org/content/forest-scene-items-animated-slimeexp-orbs

Author: patvanmackelberg
License: CC0

The page explicitly lists big/small trees, big/small rocks and forest litter such as grass, stones, sticks, logs and flowers. The page also explicitly says the creator made the assets and allows commercial use.

Suggested use:

```text
large/small trees → L4
rocks             → L4/L5
logs              → L5
sticks            → L5
flowers           → L5
```

Do not use the included character as the ASCENDENDO protagonist without a separate visual-fit review; the pack's character is not the purpose of this test.

### D. 16×16 crates / chests — Kutejnikov

Source: https://opengameart.org/content/16x16-boxes-crates-chests

Author: Kutejnikov
License: CC0

The source provides a 64×64 sheet of small 16×16-style boxes, crates and chests.

Suggested use:

```text
abandoned camp
ruin decoration
rest platforms
landmark dressing
```

These are props, not gameplay surfaces, unless a future mechanic explicitly assigns them collision semantics.

### E. Wooden crates — freestockimages

Source: https://opengameart.org/content/wooden-boxes-crates

Author: freestockimages
License: CC0

This pack provides multiple wooden box variants suitable for platformer scenes.

Use as low-frequency structural props; repeated identical crates should be avoided in long uninterrupted sequences.

### F. Mountain tiles — Impossible Realms

Source: https://opengameart.org/content/mountain-tileset

Author: Impossible Realms
License: CC0

The source explicitly identifies the set as 16×16 mountain/pixel-art/platformer material.

Suggested use:

```text
mountain-rock modules
cliff dressing
snow/rock transition study
background rock motifs
```

Because this is an older tileset, it should be style-matched against the chosen ASCENDENDO platform family before adoption.

### G. Platformer tiles with varied terrain — MattBas

Source: https://opengameart.org/content/pixel-art-platformer-asset-pack

Author: MattBas
License: CC0

The pack includes clouds, foliage, dirt/rocks, ruins and trees, plus platformer tiles with diagonals and alternate variants. It is particularly useful as a reference for variation without changing semantic terrain classes.

### H. Kenney Pixel Platformer

Source: https://kenney.nl/assets/pixel-platformer

License: Creative Commons CC0
Tile size: 18×18

This does **not** match the 16×16 base module, so it is not the preferred source for the core 16×16 terrain vocabulary. It remains valuable as a separate reference for platformer prop composition and variant density.

### I. Kenney Pixel Line Platformer

Source: https://kenney.nl/assets/pixel-line-platformer

License: Creative Commons CC0
Tile size: 16×16

This is a stronger technical reference for the exact module size used by ASCENDENDO, although its visual language may be more minimalist than the final project direction.

### J. Kenney Foliage Pack / Foliage Sprites

Sources:
- https://kenney.nl/assets/foliage-pack
- https://kenney.nl/assets/foliage-sprites

License: Creative Commons CC0

Useful for testing how tree/leaf density behaves independently of terrain and platform assets.

### K. Tree 16×16 — isaiah658

Source: https://opengameart.org/content/tree-16x16

Author: isaiah658
License: CC0

The source explicitly describes light and dark green 16×16-compatible tree sprites and suggests recolouring for seasonal variation.

Use this as a micro-scale foliage reference, not as a final giant-tree solution.

### L. Mega Pixel Art Sheet — Monster Logix Studio

Source: https://opengameart.org/content/mega-pixel-art-sheet

Author: Monster Logix Studio
License: CC0

Contains small trees, bushes, rocks, animals and miscellaneous objects. Useful for studying how many semantic prop categories can coexist under a constrained pixel language.

## 5. Background references (also CC0)

### MatiasVME — Parallax Background Forest

Source: https://opengameart.org/content/parallax-background-forest-pixel-art

License: CC0

Separate layers include clouds, three mountain layers, rocks, sky, sun and trees. This is particularly valuable for testing the ASCENDENDO multi-layer depth model.

### FabinhoSC — Clouds and Mountains Parallax

Source: https://opengameart.org/content/background-clouds-and-mountains-parallax

License: CC0 / public-domain wording on source page

Useful for testing whether the background can provide depth without stealing contrast from gameplay.

### Scikho — Floresta Parallax

Source: https://opengameart.org/content/floresta-parallax

License: CC0

A forest-parallax reference useful for layer separation and atmosphere.

## 6. First prop test set

For a first visual test, prefer the smallest coherent set rather than importing everything:

```text
PLAYER
→ existing project protagonist placeholder

PLATFORM
→ future project-approved 16×16 terrain family

L4
→ Pav pine
→ Pav broadleaf/forest vegetation where compatible
→ Reactorcore bush
→ Reactorcore rock

L5
→ grass cluster
→ flower
→ small rock
→ stick/log

L3/L2
→ MatiasVME trees/mountains as temporary background references

L1
→ MatiasVME clouds / atmospheric layer
```

The purpose is to test depth, density and visual hierarchy, not to declare these assets final project art.

## 7. Composition recipes

### Sparse forest

```text
L2  wide mountain silhouette
L3  1–2 large tree masses
L4  3–5 medium bushes/trees per screen
L5  sparse grass + 1–3 rocks
P1  platform strongly isolated
P0  player unobstructed
```

### Dense forest

Increase L4 density, not P1 detail.

```text
L4  overlapping vegetation masses
L5  clustered low foliage
P1  same contact-edge contrast
P0  reserved local negative space
```

### Mountain pass

```text
L2  huge low-contrast mountain wall
L3  medium cliff silhouettes
L4  sparse pine
L5  small exposed stones
P1  rock platform family
```

### Ruin approach

```text
L2  distant tower silhouette
L3  broken arches / walls
L4  vines + columns
L5  rubble
P1  stone platform
```

## 8. Licence rule

Every external asset adopted into the actual game must retain:

```text
creator
source URL
exact licence
licence URL
verification date
modification status
```

The project will cite the source even when CC0 does not legally require attribution.

Any source whose current licence cannot be verified is excluded.

## 9. Important implementation note

The current ASCENDENDO renderer/runtime still has a presentation path that is not yet the final sprite/terrain system. Therefore this document deliberately does not claim that these PNGs are already rendered by the game.

The next implementation step for visual assets should be a proper sprite/atlas path and then a small curated prop scene. The existing visual-composer policy remains: creators select and arrange approved assets; arbitrary image upload is not part of the initial product.

## 10. References

All references above were checked for an explicitly stated CC0 basis or equivalent public-domain wording on their source page at the time of this research. Source pages remain the authority for their exact terms.
