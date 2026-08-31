# ASCENDENDO — Live roadmap snapshot — 2026-08-31

## Canonical production state

`ASCENDENDO/main` is now the canonical implementation state. This document is a live supplement to the historical `docs/ROADMAP.md`; it does not rewrite historical entries.

## Completed production tranches

- #183–#187 — deterministic visual-stress/capture foundation
- #188–#190 — Vulkan lifecycle/result characterization and fail-closed recreation evidence
- #191 — 3 viewport classes × 3 levels deterministic capture matrix
- #192 — human playtesting protocol
- #193–#194 — roadmap reconciliation and human-session preparation
- #195–#196 — CI topology consolidation and obsolete smoke removal
- #197 — live roadmap and visual-issue reconciliation
- #198 — bounded 16×16 semantic compositor structural pilot

## Current technical evidence

```text
Camera follow Lerp bound                         DONE
Vulkan result classification                     DONE
Fail-closed swapchain recreation                 DONE
deterministic PPM readback                       DONE
16:9 / 4:3 / 21:9 capture                         DONE
3 levels × 3 viewport classes                    DONE
9 deterministic captures                         DONE
Linux + ASan/UBSan + Windows                     DONE
Semantic grid neighbour masks                    DONE
Local topology classification                    DONE
deterministic compositor ordering                DONE
duplicate semantic occupancy rejection           DONE
```

## Current compositor state

```text
SPECIFIED
   ↓
STRUCTURAL PILOT IMPLEMENTED      ← #198
   ↓
EXECUTE R1–R4 + T1–T10
   ↓
visual/contact validation
   ↓
Inicio integration
   ↓
ZigZag generalization
   ↓
GOLDEN CORPUS
```

The compositor is **not** yet generalized. It does not yet define a continuous world→16×16 rasterization policy, choose final assets, integrate with `WorldRenderer`, or claim human visual validation.

## Current evidence gap

Human evidence remains missing. Camera feel, readability, route comprehension, goal salience and difficulty must not be inferred from deterministic renderer tests alone.

## Next implementation target

Execute the bounded compositor against the `Inicio` corpus and the synthetic structural fixtures defined by the latest PROJECT-STUDIES research.

Real product corpus:

```text
R1 full-width floor
R2 isolated platform
R3 alternating platform
R4 vertically separated pair
```

Synthetic corpus:

```text
T1 1×1
T2 2×1
T3 3×1
T4 1×2
T5 L
T6 T
T7 corner
T8 material boundary
T9 missing variant
T10 mirrored/flip candidate
```

The corpus must remain separate from campaign content and must not mutate gameplay geometry.

## Second-level generalization

After the `Inicio` pilot, `ZigZag` is the first real-level generalization target. The compositor must remain independent of absolute platform coordinates and must not require level-specific artwork hacks.

## Visual issue triage

- #166 — policy/design reference; not an implementation trigger by itself.
- #167 — UGC rights/provenance policy; not an implementation trigger by itself.
- #168 — visual acceptance criterion; requires deterministic/golden or human evidence.
- #169 — visual acceptance criterion; requires deterministic/golden or human evidence.

## CI topology

```text
.github/workflows/tests.yml
    ├─ Linux / Clang / C++20 / Headless Vulkan
    └─ Linux / Clang / ASan + UBSan / Headless Vulkan

.github/workflows/windows.yml

.github/workflows/deterministic-capture.yml
```

`actions-smoke.yml` is removed.
