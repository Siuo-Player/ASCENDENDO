# ASCENDENDO — Gate 9.6 status synthesis

**Date:** 2026-08-27
**Source:** latest `PROJECT-STUDIES/ASCENDENDO` status matrix and roadmap-conformance audit, reconciled against ASCENDENDO `main`
**Role:** Base Engineering Gate evidence

## Purpose

Preserve the distinction between implementation status and property evidence. A merged implementation does not automatically make the corresponding Gate property proven.

## Current status

| Area | Status | Interpretation |
|---|---|---|
| GraphicsRuntime aggregate rollback | PROVEN | PR #81 is merged and has real headless failure/retry evidence. |
| VulkanContext failure-state invariant | IMPLEMENTED / EVIDENCE PARTIAL | PR #82 is merged; the exact adverse reconfiguration failure remains difficult to induce through the current public API without new seams/mocks. |
| Vulkan queue/capability | PARTIAL | Queue-family separation and capability checks are implemented and tested, but the broader adversarial lifecycle/error matrix is not closed. |
| Windows build/runtime | PLANNED / IN VALIDATION | Scoped Windows evidence remains pending. |
| Tick-exact replay determinism | OPEN | Input sampling is still frame-scoped relative to potentially multiple fixed ticks. |
| Malformed/semantic level validation | PARTIAL | Syntax parsing is hardened, but parser, semantic validation and canonical schema remain separate contracts. |
| Presentation/domain separation | PARTIAL | Existing ownership boundaries are sound; general gameplay `RenderSnapshot` migration remains gated. |

## Study reconciliation

The latest Studies synthesis uses `PROVEN`, `PARTIAL`, `PLANNED`, and `OPEN` to prevent implementation presence, CI health and property proof from being conflated. ASCENDENDO adopts the same distinction for Gate 9.6.

## Roadmap consequence

The execution order is unchanged:

```text
1. Finish/validate Vulkan failure-state/error semantics
2. Finish Windows reproducibility evidence
3. Define/validate tick-exact deterministic replay
4. Complete malformed/error-path evidence
5. Review architecture/ownership against implementation
6. Formal Base Engineering Gate review
7. General RenderSnapshot migration only after Gate closure
```

A green Windows run would prove compatibility for the selected pinned runner/toolchain; it would not close replay, malformed-input, Vulkan recovery or architecture properties.

## Gate rule

For every transition:

```text
property
→ test/evidence
→ environment
→ observed result
→ artifact/documentation
→ Gate decision
```

The Gate remains OPEN until every required blocking property has matching evidence.
