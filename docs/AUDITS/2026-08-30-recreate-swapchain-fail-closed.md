# ASCENDENDO — Audit: `recreateSwapchain()` fail-closed behaviour

**Observation date:** 2026-08-30  
**Repository:** `Siuo-Player/ASCENDENDO`  
**Evidence source:** current `main` plus focused integration characterization

## Reconciliation with Studies

The `PROJECT-STUDIES/ASCENDENDO` audit `2026-08-26-renderer-swapchain-recovery-state.md` described a gap: successful recreation was tested, but the caller-visible policy after a later recreation failure was not characterized.

The current ASCENDENDO code still follows the same fail-closed structure. The current window is non-resizable, so a synthetic zero-extent resize path is not a legitimate current test mechanism.

## Current property

A post-`vkDeviceWaitIdle()` dependency failure causes `RendererCore::recreateSwapchain()` to return `false`, leave `m_initialized == false`, and reject a subsequent recreation attempt.

The focused test exercises this by cleaning the non-owning `RenderPass` before calling `recreateSwapchain()`. The failure occurs while rebuilding framebuffers after the swapchain recreation work has crossed the wait-idle boundary.

## Interpretation

This is consistent with the existing fail-closed runtime policy:

```text
recreation failure
      ↓
RendererCore inactive
      ↓
drawFrame() failure propagates
      ↓
application stops rendering
```

The audit does **not** conclude that this is the best long-term product behaviour. It only establishes that the policy is explicit and testable.

## Deferred decision

Restartable recovery (B) and transactional recreation (C) remain deferred. They require a separate architectural decision and a concrete product requirement demonstrating that terminal rendering failure is insufficient.
