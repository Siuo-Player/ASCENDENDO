# Work Package — Application / graphics ownership boundary

## Status

Planned next after PR #46.

## Context

`main.cpp` still owns/composes Window, VulkanContext, Swapchain and rendering infrastructure directly. The next decomposition must follow actual ownership/lifetime instead of creating a generic application abstraction for line-count reduction.

## Current ownership facts

- `Window` is RAII, non-copyable and destroys its GLFW window in its destructor.
- `VulkanContext` is RAII, non-copyable and destroys Vulkan resources in `shutdown()`.
- `Window` creates the Vulkan surface; `VulkanContext` adopts that surface before swapchain creation.
- Swapchain/render objects depend on the context and therefore cannot outlive it.
- RendererFacade is configured from render infrastructure and must not outlive the objects it references.

## Invariant

For any successful graphics composition:

```text
Window alive
→ VulkanContext initialized
→ surface associated
→ swapchain/render infrastructure initialized
→ renderer attached
```

For failure at any step, already-created resources must be released through their existing RAII/destruction paths and the process must return without accessing destroyed dependencies.

## Implementation strategy

1. Map exact member/reference ownership and destruction order.
2. Introduce the smallest composition boundary that captures the verified graph.
3. Keep `main.cpp` responsible only for process-level orchestration.
4. Add tests where the property can be exercised without a physical GPU; use headless Vulkan for integration validation.
5. Update architecture/roadmap before merging.

## Out of scope

- generic Vulkan engine abstractions;
- renderer redesign;
- RenderSnapshot migration;
- performance work without profiling;
- gameplay changes.
