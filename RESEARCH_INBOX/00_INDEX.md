# ASCENDENDO — Research Inbox Index

This directory contains research notes, source investigations and implementation snapshots. It is not the final project specification.

## Reading order

### 01 — Core engineering / runtime
- `2026-08-25-determinism-differential-branch-handoff.md`
- `2026-08-25-determinism-differential-testing-refresh.md`
- `2026-08-26-snapshot-campaign-loader-23111960.md`
- `2026-08-26-snapshot-graphics-runtime-d68971c.md`
- `2026-08-26-snapshot-actions-queue-accumulation.md`
- `2026-08-26-snapshot-actions-orphaned-run-pattern.md`
- `2026-08-26-swapchain-recreation-failure-contract.md`

Purpose: determinism, runtime boundaries, content loading, graphics ownership, swapchain lifecycle and CI execution lifecycle.

### 02 — PCG / evaluation / difficulty
- `2025-pcg-benchmark-standardized-evaluation.md`
- `2025-structural-metric-difficulty-diversity.md`
- `2026-08-25-drl-agent-evaluation-for-pcg.md`
- `2026-08-25-internal-pcg-difficulty-benchmark.md`
- `2026-08-29-next-roadmap-research-synthesis-movement-difficulty-pcg-player-modelling.md` — deeper synthesis for movement/camera measurement, multidimensional difficulty, progression, recovery, playtesting, player modelling, PCG evaluation, diversity and experience-driven/adaptive generation.

### 03 — Player modelling / adaptive content
Place action-based player modelling, adaptive PCG and human-vs-agent evaluation here.

### 04 — UGC / recommendation / ecosystem
Place web content validation, recommendation, exposure bias, creator incentives and UGC research here.

### 05 — Runtime performance / Vulkan
Place frame-time, synchronization, latency, profiling and hardware-aware content-generation research here.

### 06 — Game design / visual presentation
- `2026-08-28-visual-identity-and-presentation-adaptation.md` — adapts cross-project visual/HCI principles to ASCENDENDO's movement-centric platformer context: player silhouette, route readability, hazards, goal cues, camera, HUD economy, VFX and automatic capture.
- `2026-08-28-game-design-deep-synthesis.md` — consolidated game-design model covering movement-learning, multi-dimensional difficulty, teach→test→vary→combine progression, recovery, pacing, camera, physics readability, level grammar, player modelling and mixed-initiative PCG.
- `2026-08-29-movement-camera-gamefeel-deep-dive.md` — deeper evidence on movement as measurable control system, camera as information filter, audiovisual feedback, impact feel and temporal VFX hierarchy.
- `2026-08-29-movement-feel-benchmark.md` — deterministic future benchmark for comparing movement/camera/feedback variants while preserving controlled geometry and capturing gameplay telemetry plus visual evidence.
- `2026-08-29-visual-ugc-privacy-and-licensing-policy.md` — retained policy for gameplay-first visual hierarchy, curated assets, copyright/licensing, creator art, UGC and privacy.
- `2026-08-29-visual-composer-curated-assets-policy.md` — future visual composer based on approved assets rather than arbitrary image upload/editing.
- `2026-08-29-visual-asset-catalog.md` — curated art catalogue for the future Visual Composer: player and platform families, hazards, checkpoints/goals, sky/atmosphere/far/mid/near/foreground assets, environmental themes, target dimensions, palette/value rules, parallax ranges, scene presets, golden-scene validation and vetted external-asset references with licensing/provenance requirements.
- `2026-08-29-layering-and-depth-composition-study.md` — deeper scene-layering/render-order policy, depth cues, occlusion safety, parallax-vs-z-order separation and visual golden-scene validation.
- `2026-08-29-visual-composer-curated-assets-policy.md` — retained decision that the editor is a safe visual composer using only approved assets; no arbitrary artwork upload in the initial product.
- `2026-08-29-procedural-modular-16x16-terrain-and-platform-composition.md` — procedural terrain/platform construction from 16×16 modules, neighbour masks, autotile/RuleTile-inspired matching, macro-asset selection, 16×16 fallback and semantic decoration rules.

### 07 — Licensed prop references
- `../Research/CC0_PROPS_REFERENCE_AND_TEST_PLAN.md` — curated CC0 prop/background candidates and a planned in-game visual test set; all source URLs and licence basis are retained.

## Rule

The inbox is chronological at the filename level but **this index is the canonical reading order**. Do not treat alphabetical order as project priority.

When a research item becomes an agreed engineering requirement, promote the conclusion into stable project documentation. Do not silently rewrite historical snapshots.
