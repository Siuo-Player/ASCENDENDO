# Decision — GraphicsRuntime initialization rollback

**Date:** 2026-08-27  
**Status:** accepted  
**Scope:** `GraphicsRuntime::init()` lifecycle contract

## Context

`GraphicsRuntime::init()` acquires resources in sequence:

```text
Window
→ VulkanContext
→ Surface
→ Swapchain
→ RenderPass
→ Pipeline
→ RendererFacade
```

The previous implementation returned `false` immediately on a failure after earlier components had already initialized. Individual components had cleanup paths, but the aggregate object did not normalize itself before returning failure.

## Problem

After an intermediate failure, the object could remain partially initialized until destruction. A later `init()` therefore depended on the exact failure point and was not a clearly defined operation.

## Decision

`GraphicsRuntime` now owns an idempotent private `cleanup()` boundary and uses it:

1. before retrying a partially initialized object;
2. on every initialization failure;
3. from the destructor.

The cleanup order is the reverse of acquisition:

```text
RendererFacade
→ Pipeline
→ RenderPass
→ Swapchain
→ VulkanContext
→ Window
```

After a failed `init()`, the object is required to be neutral and a subsequent `init()` starts from that same neutral state.

Calling `init()` on an already fully initialized object remains idempotent and returns success without rebuilding the graphics stack.

## Why

This uses the existing ownership model and cleanup APIs instead of introducing a resource manager, transactional wrapper hierarchy or generic `Application` abstraction.

It also makes failure state observable through the existing component `isInitialized()` functions.

## Alternatives rejected

- **One-shot initialization:** would leave partially acquired resources until destruction and provide weaker retry semantics.
- **Transactional construction with local temporary graph:** stronger atomicity but requires broader ownership/movability changes that are not justified by the current runtime.

## Validation strategy

Use a real integration failure rather than mocks: start the runtime from a temporary working directory without the repository shader files. `Pipeline::init()` then fails after Window/Vulkan/swapchain/render-pass setup, exercising aggregate rollback. Restore the real project directory and retry on the same object.

This test remains conditional on the normal headless Vulkan test environment because it intentionally exercises real graphics resources.

## Remaining boundary

This decision does not claim that all Vulkan failure paths are recoverable. `RendererCore` still treats `vkQueueSubmit` failure after fence reset as fatal, and swapchain/present error handling remains a separate work package.
