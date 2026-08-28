# ASCENDENDO — Vulkan lower-level failure-path classification

**Roadmap:** Gate 9.6 — Base Engineering Gate  
**Subsystem:** Graphics / Vulkan lifecycle  
**Status:** INVESTIGATED / DECISION PENDING  
**Date:** 2026-08-28  

## Problem

The Gate 9.6 evidence matrix still identifies lower-level Vulkan error-path evidence as incomplete. Existing implementation checks several `VkResult` values, but implementation coverage and executable fault-path evidence are distinct properties.

## Current implementation evidence

`RendererCore::beginFrame()` explicitly classifies failures from:

- `vkWaitForFences` → `Fatal` on non-success;
- `vkAcquireNextImageKHR` → `SwapchainNeedsRecreate` for `VK_ERROR_OUT_OF_DATE_KHR` / `VK_SUBOPTIMAL_KHR`, otherwise `Fatal`;
- `vkResetCommandBuffer` → `Fatal` on failure;
- image-index bounds → `Fatal`.

`RendererCore::submitFrame()` explicitly classifies:

- `vkResetFences` → `Fatal`;
- `vkQueueSubmit` → `Fatal` after the fence has been reset;
- `vkQueuePresentKHR` → `SwapchainNeedsRecreate` for out-of-date/suboptimal, otherwise `Fatal`.

`VulkanContext::reconfigureForSurface()` explicitly checks:

- physical-device enumeration;
- surface suitability/capabilities;
- `vkDeviceWaitIdle`;
- logical-device creation;
- queue acquisition.

`vkDeviceWaitIdle()` failure has already received an executable integration test in PR #94.

## Classification

The remaining lower-level paths fall into three categories:

### A — Already executablely evidenced

`vkDeviceWaitIdle()` in `RendererCore::recreateSwapchain()`.

### B — Behavior is explicit and fail-closed, but not currently fault-injected

- `vkWaitForFences`;
- `vkAcquireNextImageKHR` non-recreation failure;
- `vkResetCommandBuffer`;
- `vkResetFences`;
- `vkQueueSubmit`;
- `vkQueuePresentKHR` fatal result;
- framebuffer/command-pool/command-buffer/synchronization-object creation failures.

### C — Capability/configuration assumptions better addressed by deterministic positive evidence

- graphics/present queue-family compatibility;
- required device/surface capabilities;
- software Vulkan driver assumptions already exercised by CI.

## Decision

Do **not** introduce a general Vulkan mocking framework.

Do **not** add broad fault injection merely to increase the number of tested `VkResult` branches.

A new executable fault seam is justified only when it proves a lifecycle or ownership invariant that cannot be established convincingly from existing implementation structure and ordinary integration runs.

The highest-value next question is therefore not “can every `VkResult` be forced?”, but:

> which failure path can leave observable synchronization/resource state ambiguous at its caller boundary?

`vkQueueSubmit` is the strongest candidate because the implementation deliberately resets the in-flight fence immediately before submission and documents that a failed submission is not safely reusable for that frame. This warrants a focused investigation before any seam is added.

## Remaining evidence question

Investigate whether `vkQueueSubmit` failure requires an explicit executable contract at the `RendererCore` boundary, including what remains valid for the next frame and whether the application must terminate rather than retry.

Only if that investigation identifies an observable invariant worth proving should a minimal per-instance seam be introduced.

## Non-goals

- no Vulkan mock framework;
- no global function substitution;
- no rendering redesign;
- no swapchain transactional rewrite;
- no RenderSnapshot work;
- no changes to gameplay.

## Provenance

- `Game/Graphics/RendererCore.cpp` at `main` 8f01d53...;
- `Game/Graphics/RendererFacade.cpp` at the same baseline;
- `Tests/Integration/test_renderer_core.cpp` with existing `vkDeviceWaitIdle` failure test;
- `PROJECT-STUDIES/ASCENDENDO/CURRENT_STATE_2026-08-28_DEEP.md`;
- `PROJECT-STUDIES/ASCENDENDO/RESEARCH_INBOX/2026-08-26-swapchain-recreation-failure-contract.md`.

## Conclusion

The repository currently demonstrates explicit fail-closed handling across the inspected Vulkan result paths. The remaining Gate debt is evidence breadth plus precise queue/capability contract definition, not an observed production failure. Further work should therefore target the highest-value ambiguous boundary rather than enumerate every theoretical API failure.
