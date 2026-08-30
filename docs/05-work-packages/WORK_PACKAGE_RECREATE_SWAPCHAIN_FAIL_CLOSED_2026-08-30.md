# ASCENDENDO — Work Package: Swapchain recreation fail-closed contract

**Date:** 2026-08-30  
**Scope:** RendererCore / Swapchain lifecycle  
**Status:** characterization tranche

## Evidence

The current `RendererCore::recreateSwapchain()` waits for the device, marks the core inactive, destroys frame resources, asks `Swapchain::recreate()` to rebuild the swapchain, and recreates frame resources. When a dependent recreation step fails after the successful wait-idle boundary, the method returns `false` and leaves `m_initialized == false`.

The corresponding Study audit dated 2026-08-26 identifies this as an intentional fail-closed policy candidate and recommends characterizing it before considering restartable or transactional recovery.

## Property

If swapchain recreation crosses the wait-idle boundary but a later dependent operation fails, `RendererCore` must not report itself initialized and must reject a second recreation attempt through the existing initialization guard.

This tranche does **not** change that runtime policy. It only characterizes it.

## Test mechanism

The integration test initializes the normal graphics stack, explicitly waits for the device to become idle, then invalidates the `Swapchain` object through its public `cleanup()` method. `RendererCore::recreateSwapchain()` subsequently reaches the existing `Swapchain::recreate()` guard and fails closed because the dependency is no longer initialized.

Expected result:

```text
recreateSwapchain() -> false
isInitialized()     -> false
second recreation   -> false
```

The test does not manually tear down the Vulkan context. Object lifetime is left to RAII so `RendererCore` is destroyed before the Vulkan objects it references.

## Decision boundary

No retry/restartable recreation is introduced here. A future recovery design requires a separate architectural decision and evidence that terminal rendering failure is unacceptable for the intended product/runtime behaviour.
