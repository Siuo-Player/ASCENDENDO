# ASCENDENDO — Vulkan wait-idle contract revalidation

**Date:** 2026-08-28  
**Project:** `Siuo-Player/ASCENDENDO`  
**Observed upstream:** `main` at `be6eb18beca668dcb5880febe64e641767970aea`  
**Evidence:** E1 / LOCAL — current implementation inspection

## Scope

Revalidate the earlier `RendererCore::recreateSwapchain()` `vkDeviceWaitIdle()` finding against the current `main` after the malformed-level evidence merge.

## Observation

Current `RendererCore::recreateSwapchain()` performs:

```text
validate initialized/context/swapchain
→ vkDeviceWaitIdle(device)
→ on failure: return false
→ only after success: m_initialized = false
→ destroy old frame resources
→ recreate swapchain/resources
```

Therefore a `vkDeviceWaitIdle()` failure does **not** invalidate the existing `RendererCore` state or destroy its current frame resources.

The current `RendererFacade::drawFrame()` handles the recreate request as:

```text
SwapchainNeedsRecreate
→ m_core->recreateSwapchain()
→ return that boolean directly
```

No second frame submission occurs inside that path. The top-level loop treats a `false` result from `drawFrame()` as a renderer failure and breaks the frame loop.

## What is proven

The current implementation has the following structural property:

```text
wait-idle failure
→ recreateSwapchain() == false
→ drawFrame() == false
→ current drawFrame() invocation does not submit another frame
→ main loop breaks
```

This is consistent with the recommended fatal policy for a failed synchronization barrier.

## What is not yet proven

The repository has no focused test that induces `vkDeviceWaitIdle()` to return an error and observes the complete consumer path. The existing renderer-core integration test exercises successful swapchain recreation and unrelated real-resource failures, but not this specific fault path.

Therefore the project does **not** claim a fully executable failure-path proof for `vkDeviceWaitIdle()`.

## Engineering decision for this tranche

Do not introduce a broad Vulkan mock layer or speculative fault-injection framework solely to close this checkbox.

The current production behavior already fails closed, and the unresolved item is the absence of a focused fault-path execution mechanism.

The gap remains tracked as:

```text
implementation semantics: ✅
consumer fail-closed behavior: ✅
induced failure-path evidence: 🟡
```

## Consequence for Gate 9.6

This finding remains a P0 evidence gap, but it is narrower than the historical audit suggested. It should be considered together with the other remaining Vulkan error paths rather than reopened as a renderer redesign.

## Provenance

The source and current documentation were inspected read-only before this document was created. No PROJECT-STUDIES content was modified.
