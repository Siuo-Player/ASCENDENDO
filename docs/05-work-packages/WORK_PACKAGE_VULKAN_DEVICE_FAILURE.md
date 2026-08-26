# Work Package — Vulkan logical-device failure safety

## Observation

`VulkanContext::createLogicalDevice()` creates the logical device and then retrieves both graphics and present queues. If queue retrieval leaves an invalid handle, the function can currently return `false` while retaining a live `VkDevice` and partially initialized queue state.

## Required invariant

After `createLogicalDevice()` returns `false`:

```text
m_device        == VK_NULL_HANDLE
m_graphicsQueue == VK_NULL_HANDLE
m_presentQueue  == VK_NULL_HANDLE
```

The surrounding `VulkanContext::init()` failure path must remain safely repeatable.

## Design choice

Keep ownership local to `VulkanContext`. If queue acquisition does not produce both required queue handles, destroy the just-created logical device and clear all queue handles before returning `false`.

Do not add a generic Vulkan resource manager.

## Queue-family contract

Graphics and presentation families remain independent values. `Swapchain` already selects concurrent image sharing when they differ.

## Scope

- logical-device failure cleanup;
- regression evidence where available through normal initialization;
- preserve queue-family independence;
- no `main.cpp` changes;
- no CI workflow changes.

## Testability note

The exact invalid-queue-handle branch is driver-dependent and cannot be deterministically forced through the public API without mocking Vulkan. The test therefore locks the public queue-family contract, while the cleanup invariant is enforced directly in the implementation.
