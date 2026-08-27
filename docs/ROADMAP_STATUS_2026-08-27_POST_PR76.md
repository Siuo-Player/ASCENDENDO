# ASCENDENDO — Roadmap status after PR #76

**Date:** 2026-08-27  
**Canonical implementation baseline:** `main` @ `2e98adbd49a1604c942d045abbccf2d4380e1227`  
**Purpose:** dated operational snapshot; informative, not a replacement for `docs/ROADMAP.md`.

## Completed since the previous baseline

- PR #70 — independent Linux ASan/UBSan workflow.
- PR #72 — `GameSession` runtime/session boundary.
- PR #73 — single global GLFW lifetime owner.
- PR #74 — Vulkan capability matrix evidence for device/queue/swapchain prerequisites.
- PR #75 — common `LevelData` representation for parser/editor/runtime.
- PR #76 — `RuntimeBootstrap` composition boundary and strict current `.lvl` syntax handling.

## Current 9.6 boundary

```text
main.cpp
├── process / GLFW lifetime
├── RuntimeBootstrap
├── GraphicsRuntime
├── PresentationRuntime
├── InputManager / KeyBindings
├── Camera
└── GameSession
```

`RuntimeBootstrap` is composition only. It does not own Vulkan/GLFW, gameplay state, camera, presentation, or the frame loop.

## Gate status

The Base Engineering Gate remains **OPEN**. Green Linux normal and ASan/UBSan workflows are necessary evidence but are not sufficient to close the Gate.

Remaining material evidence/debt:

- Windows build/tests and Windows game build/link;
- Vulkan lifecycle/error-path behaviour beyond successful capability discovery;
- `GraphicsRuntime::init()` failure/retry contract;
- tick-exact input/replay semantics;
- collision-order determinism contract;
- explicit scope for accumulated world geometry versus per-chunk metadata;
- full LevelData schema/version/semantic validation;
- campaign identity and loader sharing a single logical campaign representation;
- general `RenderSnapshot` boundary.

## Important contract clarifications

`LevelDataIO` is currently a **strict syntax parser/serializer**, not a complete schema or semantic validator. Unknown directives and trailing tokens are rejected, but schema versioning, migrations, domain validation and canonicalization remain later work.

Observed collision-order dependence is a confirmed property of the current algorithm, but no replacement policy has yet been selected. Do not patch ordering ad hoc.

`GameSession` remains the session orchestration boundary. New functionality should not be added there merely for convenience.

## Next engineering order

```text
finish/record #76
    ↓
review remaining 9.6 Gate evidence
    ↓
Vulkan lifecycle/error-path contract
    ↓
Windows CI / portability evidence
    ↓
Gate review
    ↓
RenderSnapshot only after Gate acceptance
```

The content/data track should separately address `CampaignDocument`, schema/versioning and semantic validation before UGC/import work.

## Provenance

This snapshot incorporates the adversarial review stored in `Siuo-Player-PROJECT-STUDIES` PR #3 and the resulting canonical updates in `docs/TECH_DEBT.md` and `docs/ARCHITECTURE.md`. The snapshot itself remains informative; implementation and canonical roadmap documents remain authoritative.