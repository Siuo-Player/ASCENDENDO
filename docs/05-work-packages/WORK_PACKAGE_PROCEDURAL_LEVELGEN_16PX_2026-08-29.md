# Work Package — Procedural level generator 16px modules

## Context

The fixed canonical levels were migrated to platform dimensions compatible with the project's 16px visual modules. The validation record explicitly identified `Development/AI_Validation/sim/levelgen.py` as still emitting the legacy 20px platform height and non-modular widths.

## Finding

The procedural generator still used:

```text
PLATFORM_H = 20
Inicio width = 180
ZigZag width = 140
Precipicio width = 100
Final pedestal = 120
```

These dimensions are inconsistent with the now-established modular platform geometry used by the fixed levels.

## Decision

Align the generator's platform output with the established module:

```text
platform height = 16
Inicio width = 176
ZigZag width = 144
Precipicio width = 96
Final pedestal = 128
```

Keep authored world X/Y positions free; the 16px rule applies to platform dimensions, not to a mandatory world placement grid.

The full-width Inicio ground remains 640px wide and becomes 16px high.

## Scope

- `Development/AI_Validation/sim/levelgen.py` only;
- preserve existing jump physics, charge values, clearance policy and chunk heights;
- preserve the existing procedural verification logic;
- no runtime renderer/gameplay changes.

## Validation

The repository CI is the authoritative validation for this change. Required checks remain:

- Linux / Clang / C++20 / Headless Vulkan;
- Linux / Clang / ASan + UBSan / Headless Vulkan;
- Windows / Clang / C++20;
- source-size;
- build/tests;
- campaign validation.

A local simulator execution was not performed in the agent container because network access to retrieve the repository was unavailable.

## Exit criteria

```text
procedural output uses only modular platform dimensions
+ existing verification remains intact
+ CI green on all required environments
+ no runtime behavior outside the generator changes
```

## Next decision

After this generator alignment is validated, continue with the roadmap's visual validation tranche: deterministic stress scenes / render captures across representative viewports. Do not use this generator change as evidence that visual readability is already proven.
