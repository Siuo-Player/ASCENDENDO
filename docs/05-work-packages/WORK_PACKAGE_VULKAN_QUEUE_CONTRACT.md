# Work Package — Vulkan queue-family contract

## Observation

`VulkanContext` models graphics and presentation queue families independently. `Swapchain` uses `VK_SHARING_MODE_CONCURRENT` when the families differ.

## Required invariant

The renderer must never assume `graphicsFamily == presentFamily`.

The supported states are:

```text
graphics == present
or
graphics != present
```

Both are valid when the surface capabilities and queues support them.

## Evidence

- `QueueFamilyIndices` stores separate `graphics` and `present` indices.
- `createLogicalDevice()` creates unique queue-create infos for the two families.
- `Swapchain::createResources()` selects concurrent sharing when the indices differ.

## Scope

This document records the already-implemented contract. Further hardware-specific matrix coverage belongs to the cross-platform/runtime evidence work and should not be conflated with this architectural invariant.
