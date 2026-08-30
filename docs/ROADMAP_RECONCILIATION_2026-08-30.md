# ASCENDENDO — Roadmap reconciliation — 2026-08-30

## Purpose

This document reconciles the operational state of `ASCENDENDO/main` with the historical roadmap and the available `PROJECT-STUDIES/ASCENDENDO` evidence.

`ASCENDENDO/main` is authoritative for implementation state. `PROJECT-STUDIES/ASCENDENDO` is evidence for investigation and methodology and may be stale.

## Completed since the roadmap's older checkpoint

- PR #170 — Camera follow Lerp bound — merged.
- PR #172 — deterministic camera/viewport benchmark — merged.
- PR #174 — procedural 16px module alignment — merged.
- PR #177 — visual stress-scene / render-capture definition — merged.
- PR #183 — deterministic visual stress-scene manifest — merged.
- PR #184 — optional Vulkan PPM frame capture — merged.
- PR #185 — deterministic level-selection capture launcher — merged.
- PR #186 — deterministic capture end-to-end validation — merged.
- PR #187 — roadmap/base-hardening reconciliation — merged.
- PR #188 — Vulkan `VkResult` → `FrameStatus` classification — merged.
- PR #190 — fail-closed `recreateSwapchain()` characterization — merged.
- PR #191 — deterministic viewport capture matrix — merged.
- PR #192 — human playtesting protocol — merged.

## Current deterministic evidence

The capture pipeline now proves real framebuffer capture at:

```text
3 levels × 3 viewport classes = 9 captures

16:9
4:3
21:9
```

The captures establish framebuffer creation and PPM readback. They do not establish human readability, camera feel, route comprehension, goal salience, or difficulty balance.

## Current Vulkan lifecycle evidence

The renderer now has an explicit `VkResult` classification policy and an explicit characterization of fail-closed swapchain recreation. No restartable or transactional recovery policy has been introduced.

## Current empirical boundary

The next evidence class is human, not another synthetic validator:

```text
deterministic captures
        ↓
controlled stimulus
        ↓
human playtesting pilot
        ↓
repeatable observations
        ↓
candidate presentation/camera findings
```

No camera, level-geometry, difficulty, or procedural-generation change should be promoted from the deterministic evidence alone.

## Studies reconciliation

The available `PROJECT-STUDIES/ASCENDENDO` material remains predominantly dated 2026-08-28 and therefore does not describe the current post-#192 implementation state. Its methodological constraints remain applicable: do not infer broad properties from narrow tests, and require a concrete property, affected consumer, demonstrable risk, and a test/evidence path before changing production behavior.

No dedicated human-playtesting Study was found in the available Studies tree. The human-playtesting protocol is therefore an ASCENDENDO operational proposal, not a Study-derived conclusion.

## Next work

1. Run the first controlled human-playtesting pilot using `docs/05-work-packages/WORK_PACKAGE_HUMAN_PLAYTESTING_PROTOCOL_2026-08-30.md`.
2. Record observations without changing gameplay/content during the pilot.
3. Promote only repeatable, player-impacting findings into production work packages.
4. Only after review of those findings, proceed to difficulty/progression calibration and later procedural assistance.
