# ASCENDENDO — Live roadmap snapshot — 2026-08-31

## Canonical production state

`ASCENDENDO/main` = `bd6f850ac430741f3c291a2866be30c2960a76f9`.

The historical `docs/ROADMAP.md` remains authoritative for its existing detailed entries, but its tail predates PRs #183–#196. This snapshot reconciles the live state without rewriting historical material.

## Completed since the historical roadmap tail

- #183 — deterministic capture launcher / level selection
- #184 — deterministic capture integration groundwork
- #185 — deterministic capture launcher hardening
- #186 — deterministic capture E2E
- #187 — roadmap/status reconciliation
- #188 — Vulkan `VkResult` lifecycle classification
- #190 — fail-closed swapchain recreation characterization
- #191 — 3 viewport classes × 3 levels deterministic capture matrix
- #192 — human playtesting protocol
- #193 — roadmap reconciliation
- #194 — human playtest session preparation
- #195 — Linux tests + ASan/UBSan workflow consolidation
- #196 — obsolete Actions smoke diagnostic removal
- #197 — live roadmap and visual issue reconciliation

## Current evidence

Technical evidence now covers:

```text
Camera follow Lerp bound
Vulkan result classification
fail-closed swapchain recreation
real PPM readback
3 levels × 3 viewport classes
16:9 / 4:3 / 21:9
9 deterministic captures
Linux + ASan/UBSan + Windows gates
```

## Current open evidence gap

Human evidence is still missing. The existing protocol and session sheet are preparation artifacts, not evidence about camera feel, readability, route comprehension, goal salience or difficulty.

## Current next step

1. Execute real human playtest sessions.
2. Record raw observations separately from investigator interpretation.
3. Promote only repeatable observations to findings.
4. Create production work packages only for findings with an explicit property, affected consumer and validation test.

## Visual issue triage

- #166 — policy/design reference; no direct implementation from issue alone.
- #167 — UGC rights/provenance policy; no direct implementation from issue alone.
- #168 — visual acceptance criterion; requires deterministic/golden scene or human evidence.
- #169 — permanent visual acceptance rule; requires deterministic/golden scene or human evidence.

## CI topology after #196

```text
.github/workflows/tests.yml
    ├─ Linux / Clang / C++20 / Headless Vulkan
    └─ Linux / Clang / ASan + UBSan / Headless Vulkan

.github/workflows/windows.yml

.github/workflows/deterministic-capture.yml
```

`actions-smoke.yml` is removed.
