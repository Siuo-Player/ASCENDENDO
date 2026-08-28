# ASCENDENDO — Vulkan lower-level failure-path classification

**Roadmap:** Gate 9.6 — Base Engineering Gate  
**Subsystem:** Graphics / Vulkan lifecycle  
**Status:** INVESTIGATED / DECISION PENDING  
**Date:** 2026-08-28  

## Problem

Gate 9.6 still has a residual evidence gap around lower-level Vulkan failure paths. The implementation handles multiple `VkResult` values, but implementation coverage and executable fault-path evidence are distinct properties.

## Current implementation evidence

`RendererCore::beginFrame()` explicitly classifies failures from `vkWaitForFences`, `vkAcquireNextImageKHR`, `vkResetCommandBuffer`, and image-index bounds.

`RendererCore::submitFrame()` explicitly classifies `vkResetFences`, `vkQueueSubmit`, and `vkQueuePresentKHR`. The submit path resets the in-flight fence immediately before submission and treats `vkQueueSubmit` failure as terminal for the current frame.

`VulkanContext::reconfigureForSurface()` explicitly checks physical-device enumeration, surface suitability/capabilities, `vkDeviceWaitIdle`, logical-device creation, and queue acquisition.

`vkDeviceWaitIdle()` failure already has executable integration evidence from PR #94.

## Classification

### A — executablely evidenced

`vkDeviceWaitIdle()` in `RendererCore::recreateSwapchain()`.

### B — explicit fail-closed behavior without fault injection

- `vkWaitForFences`;
- `vkAcquireNextImageKHR` non-recreation failure;
- `vkResetCommandBuffer`;
- `vkResetFences`;
- `vkQueueSubmit`;
- fatal `vkQueuePresentKHR`;
- framebuffer/command-pool/command-buffer/synchronization-object creation failures.

### C — capability/configuration evidence

- graphics/present queue-family compatibility;
- required device/surface capabilities;
- deterministic software Vulkan driver setup already exercised by CI.

## Decision

Do not introduce a general Vulkan mocking framework and do not add fault injection merely to increase branch-count coverage.

A new executable seam is justified only if it proves a lifecycle or ownership invariant that cannot be established convincingly from the current implementation plus ordinary integration evidence.

The highest-value next question remains `vkQueueSubmit`: because the in-flight fence is reset immediately before submission, the post-failure synchronization contract deserves explicit investigation before any implementation seam is added.

## Remaining evidence question

Determine whether `vkQueueSubmit` failure creates an observable ambiguous state at `RendererCore`/`RendererFacade`, or whether the current fail-closed behavior and application termination path are already sufficient evidence.

Only an evidence-backed invariant should produce a follow-up implementation WP.

## Non-goals

- Vulkan mock framework;
- global function substitution;
- renderer redesign;
- transactional swapchain rebuild;
- RenderSnapshot migration;
- gameplay changes.

## Provenance

- `Game/Graphics/RendererCore.cpp` at current `main` `bec96c3...`;
- `Game/Graphics/RendererFacade.cpp`;
- `Tests/Integration/test_renderer_core.cpp`;
- `PROJECT-STUDIES/ASCENDENDO/CURRENT_STATE_2026-08-28_1329.md`;
- `PROJECT-STUDIES/ASCENDENDO/RESEARCH_INBOX/2026-08-26-swapchain-recreation-failure-contract.md`.

## Conclusion

The current implementation demonstrates explicit fail-closed handling for the inspected Vulkan result paths. Remaining debt is evidence breadth and precise queue/capability contract definition, not a confirmed production defect.
