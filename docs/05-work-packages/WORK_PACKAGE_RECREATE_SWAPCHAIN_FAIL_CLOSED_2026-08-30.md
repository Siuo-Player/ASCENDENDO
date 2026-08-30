# ASCENDENDO — Work Package: Swapchain recreation fail-closed contract

**Date:** 2026-08-30  
**Scope:** RendererCore / Swapchain lifecycle  
**Status:** validation tranche

## Evidence

The current `RendererCore::recreateSwapchain()` waits for the device, marks the core inactive, destroys frame resources, asks `Swapchain::recreate()` to rebuild the swapchain, and recreates frame resources. If frame-resource creation fails after the wait-idle barrier, the method returns `false` and leaves `m_initialized == false`.

The corresponding Study audit dated 2026-08-26 identifies this as an intentional fail-closed policy candidate and recommends characterizing it before considering restartable or transactional recovery.

## Property

If swapchain recreation passes the wait-idle barrier but a later dependent resource creation fails, `RendererCore` must not report itself initialized and must reject a second recreation attempt through the existing initialization guard.

This tranche does **not** change that policy. It only characterizes it.

## Test mechanism

The integration test initializes the normal graphics stack, explicitly cleans the non-owning `RenderPass`, and then calls `RendererCore::recreateSwapchain()`.

Expected result:

```text
recreateSwapchain() -> false
isInitialized()     -> false
second recreation   -> false
```

This uses an existing dependency failure rather than a new mock seam.

## Decision boundary

No retry/restartable recreation is introduced here. A future recovery design requires a separate architectural decision and evidence that terminal rendering failure is unacceptable for the intended product/runtime behaviour.
