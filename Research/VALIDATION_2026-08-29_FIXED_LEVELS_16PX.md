# ASCENDENDO — Validation of fixed levels after 16px platform sizing

**Date:** 2026-08-29
**Scope:** `Game/Assets/Levels/inicio.lvl`, `zigzag.lvl`, `precipicio.lvl`

## Rule applied

The 16×16 requirement applies to the **platform asset/geometry dimensions**, not to authored world positions.

```text
platform width  = 16 × integer
platform height = 16 × integer
platform X/Y    = free world/pixel coordinates
```

The logical world remains 640×360 px. Therefore the world itself is pixel-addressed rather than a mandatory 16px placement grid.

## Fixed-level changes

### Inicio

```text
old: 640×20, 180×20, 180×20, 180×20
new: 640×16, 176×16, 176×16, 176×16
```

The top surfaces were preserved by moving each platform's Y coordinate down/up by 4 px as required by the reduced thickness. This avoids changing the intended landing altitude while making dimensions modular.

### ZigZag

```text
old: 140×20
new: 144×16
```

The top surfaces remain at the original intended heights.

### O Precipício

```text
old: 100×20
new: 96×16

old final pedestal: 120×20
new final pedestal: 128×16
```

The flag remains 128×40; its dimensions are already multiples of 16 in both axes.

## Deterministic validation

The validation used the same fixed-step mechanics and collision logic represented by `Development/AI_Validation/sim/engine.py`: 60 Hz step, gravity -980, player 16×16, jump force 250–600, 60° trajectory and the documented collision procedure.

Result for intended sequential jumps at charge 0.78:

```text
Inicio       3/3 deterministic landings valid
ZigZag       3/3 deterministic landings valid
Precipício   3/3 deterministic landings valid
```

For each landing, the simulated player centre remained within the target platform's horizontal bounds and reached the target top surface within the simulator tolerance.

## Robustness check

For each of the 9 platform transitions:

```text
50 trials
±8% charge perturbation
±10 px starting-position perturbation
```

All 9 transitions achieved:

```text
50/50 valid landings
100% observed robustness in this test
```

## Flag check

The final flag interaction was also checked with the same perturbation family:

```text
49/50 flag contacts
98% observed contact rate
```

This remains above the existing 85% robustness criterion used by the project simulator.

## Interpretation

The 16px dimensional change did not invalidate the current three fixed levels under the tested deterministic/robust scenarios. It does not prove that their design is already good; it only establishes that the dimensional refactor preserved the tested physical route.

A future visual pass should still evaluate:

```text
platform contact-edge readability
visual seam quality
autotile correctness
character/platform contrast
camera readability
new platform widths vs actual intended difficulty
```

## Important implementation note

The current procedural generator still contains a legacy 20px `PLATFORM_H` constant in `Development/AI_Validation/sim/levelgen.py`. The next procedural-generator implementation pass should change its platform-size output policy to the 16px modular geometry described by the procedural terrain study before new canonical levels are generated from it.
