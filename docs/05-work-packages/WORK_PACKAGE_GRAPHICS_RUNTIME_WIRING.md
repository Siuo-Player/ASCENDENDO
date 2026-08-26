# Work Package — GraphicsRuntime wiring

## Status

In progress.

## Purpose

Replace the duplicated core graphics bootstrap in `main.cpp` with the already-tested `gfx::GraphicsRuntime` composition boundary.

## Invariant

`main.cpp` must not independently construct or initialize the core graph:

```text
Window → VulkanContext → Swapchain → RenderPass → Pipeline → RendererFacade
```

The entry point may keep non-core optional services (text/sprite pipelines, input, gameplay/editor state), while obtaining the core graphics objects through `GraphicsRuntime`.

## Scope

- include `GraphicsRuntime`;
- instantiate one `GraphicsRuntime` owner;
- expose references used by the existing loop;
- remove duplicate core bootstrap code;
- preserve optional text/sprite setup and all gameplay state transitions.

## Out of scope

- RenderSnapshot migration;
- gameplay changes;
- campaign identity redesign;
- new Vulkan abstractions.
