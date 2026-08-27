# ASCENDENDO — Adversarial snapshot reconciliation

**Date:** 2026-08-27  
**Source:** `Siuo-Player-PROJECT-STUDIES` PR #3  
**Implementation baseline checked:** current `main` after PR #75 (`d5d69a7…` at snapshot time) and open PR #76 (`9aeeaf1…` at snapshot time)

## Purpose

Reconcile the latest external project snapshot with the implementation repository. This document records only findings that were checked against current source and are relevant to canonical engineering contracts.

## Confirmed findings

### 1. `LevelDataIO` is parser/serializer, not validator

The current `LevelDataIO::load()` accepts known `NAME`, `PLATFORM`, `FLAG` and `SPAWN` records and returns `LevelData`. Unknown record types are not rejected, and the current parser does not establish a full schema/version or semantic validation boundary.

Conclusion: the current LevelData tranche is a representation convergence, not schema validation. Validation/versioning/canonicalization remain Phase 10 work.

### 2. `.lvl` is already evolving before versioning

`SPAWN` is already represented as an optional field in `LevelData`. Therefore format evolution has started even though explicit versioning remains deliberately deferred.

Conclusion: avoid expanding persisted syntax unnecessarily before the Phase 10 version/migration policy exists.

### 3. `Level` has mixed scope

`Level::appendFromData()` appends platform geometry to an accumulated vector while resetting `name` and `hasFlag` for the newest appended chunk. Therefore geometry has world/accumulation scope while some metadata has latest-chunk scope.

Conclusion: future campaign/UGC work needs an explicit model separating chunk/entry metadata from world aggregate state. Do not repair individual fields ad hoc.

### 4. Collision order dependence is confirmed by algorithm

`Level::resolveCollision()` walks `m_platforms` sequentially and mutates the body after each collision. Consequently, multiple relevant contacts can make vector order part of the state transition. This is a confirmed implementation property; it is not yet proof that current authored levels visibly diverge in gameplay.

Conclusion: define a deterministic collision-order contract before calling the representation order irrelevant or before changing the solver.

### 5. Input edges are render-frame scoped

`justPressed`/`justReleased` are cleared at render-frame boundaries, while fixed-step simulation can consume more than one tick in a frame.

Conclusion: a future tick-exact replay/evaluation protocol must define tick-indexed commands or an equivalent explicit sampling rule.

### 6. `GameSession` should remain an orchestration boundary

The extraction is architecturally justified, but `GameSession::update()` now coordinates several concerns. This is not sufficient reason for another split. It is a guardrail: new capabilities should not turn the session object into a generic service container.

### 7. `RuntimeBootstrap` is intentionally narrow

PR #76 is a valid 9.6 extraction for startup composition. It must not absorb frame loop, Vulkan, Camera, presentation, or gameplay state. It also does not solve the independent `CampaignLoader`/`CampaignID` interpretation problem.

### 8. Vulkan capability evidence is not lifecycle-recovery evidence

The capability matrix added by PR #74 proves minimum device/queue/swapchain assumptions under the tested environment. It does not prove all acquire/reset/submit/present failure paths or reinitialization semantics.

### 9. Windows and broader hardware evidence remain open

Linux normal/headless and ASan/UBSan evidence materially improve confidence, but they do not imply Windows or hardware/driver coverage.

## Process finding

The snapshot layer is a dated navigation/audit source, not a replacement for `ROADMAP.md`, `TECH_DEBT.md` or `ARCHITECTURE.md`. The canonical implementation documents have been updated to preserve this hierarchy.

## Decision

No broad refactor is introduced from this audit. The current order remains:

```text
finish/narrow PR #76
→ resolve remaining 9.6 Gate evidence
→ define collision-order contract
→ define robust LevelData validation/versioning in its roadmap phase
→ define campaign canonical source before UGC
→ review Gate
→ RenderSnapshot migration
```
