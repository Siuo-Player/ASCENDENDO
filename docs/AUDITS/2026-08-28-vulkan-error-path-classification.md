# ASCENDENDO — Vulkan lower-level failure-path classification

**Roadmap:** Gate 9.6 — Base Engineering Gate  
**Subsystem:** Graphics / Vulkan lifecycle  
**Status:** INVESTIGATED / DECISION PENDING  
**Date:** 2026-08-28

## Problem

Gate 9.6 retains an evidence gap around lower-level Vulkan failure paths. The implementation handles multiple `VkResult` values, but explicit handling is not identical to executable fault-path evidence.

## Current implementation

`RendererCore::beginFrame()` classifies `vkWaitForFences`, `vkAcquireNextImageKHR`, `vkResetCommandBuffer`, and image-index errors. `submitFrame()` classifies `vkResetFences`, `vkQueueSubmit`, and `vkQueuePresentKHR`. `VulkanContext::reconfigureForSurface()` checks device enumeration, surface suitability, `vkDeviceWaitIdle`, logical-device creation and queue acquisition.

`vkDeviceWaitIdle()` failure already has executable evidence through PR #94.

## Classification

**Executablely evidenced:** `vkDeviceWaitIdle()` in swapchain recreation.

**Explicit fail-closed, not fault-injected:** wait-for-fences, acquire-next-image fatal results, reset-command-buffer, reset-fences, queue-submit, fatal queue-present, and resource-creation failures.

**Positive capability/configuration evidence:** queue-family compatibility, required device/surface capabilities and deterministic software Vulkan setup.

## Decision

Do not introduce a general Vulkan mocking framework or fault-inject every API solely for branch-count coverage.

The next high-value question is the `vkQueueSubmit` post-failure contract because the in-flight fence is reset immediately before submission. The relevant question is whether the failure leaves ambiguous synchronization/resource state at the caller boundary, or whether the current fail-closed path is already sufficient.

Only an observable invariant that remains unproved should justify a new minimal seam.

## Non-goals

- no global Vulkan substitution;
- no renderer redesign;
- no transactional swapchain rewrite;
- no RenderSnapshot migration;
- no gameplay changes.

## Provenance

- `Game/Graphics/RendererCore.cpp` at `main` `bec96c3...`;
- `Game/Graphics/RendererFacade.cpp`;
- `Tests/Integration/test_renderer_core.cpp`;
- `PROJECT-STUDIES/ASCENDENDO/CURRENT_STATE_2026-08-28_1329.md`;
- `PROJECT-STUDIES/ASCENDENDO/RESEARCH_INBOX/2026-08-26-swapchain-recreation-failure-contract.md`.

## Conclusion

The current code demonstrates explicit fail-closed treatment of the inspected Vulkan results. The residual debt is evidence breadth and precise queue/capability contract definition, not a confirmed production defect.
