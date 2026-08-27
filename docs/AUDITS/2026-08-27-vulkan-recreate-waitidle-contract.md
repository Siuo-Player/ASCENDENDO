# ASCENDENDO — Vulkan swapchain recreation wait-idle contract

**Date:** 2026-08-27  
**Source:** `Siuo-Player/Siuo-Player-PROJECT-STUDIES` PR #11  
**Role:** 9.6 Base Engineering Gate evidence

## Finding

The current `RendererCore::recreateSwapchain()` keeps the renderer's existing resources intact when `vkDeviceWaitIdle()` fails, while returning only `false`. The downstream `RendererFacade::drawFrame()` therefore does not distinguish this condition from other renderer failures.

This is currently classified as a **contract ambiguity**, not a proven resource-corruption bug.

## Desired policy

The Study recommendation is to treat a `vkDeviceWaitIdle()` failure during swapchain recreation as fatal to rendering, while preserving the existing fail-closed behavior for failures after the renderer has been invalidated for reconstruction.

Required evidence:

```text
recreate requested
→ wait-idle failure
→ return false
→ caller does not submit another frame
```

Until that behavior is tested/documented in the upstream implementation, the item remains an evidence gap.

## Boundary

Do not introduce transactional Vulkan abstractions or redesign swapchain policy for this finding. The open question is the lifecycle contract exposed to the caller.

## Provenance

This document revalidates Studies PR #11 against the current ASCENDENDO state. The study layer is read-only with respect to the implementation repository.
