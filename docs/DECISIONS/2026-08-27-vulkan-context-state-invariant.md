# Decision — VulkanContext failure-state invariant

**Date:** 2026-08-27  
**Status:** accepted  
**Scope:** `VulkanContext::reconfigureForSurface()` lifecycle state

## Context

PROJECT-STUDIES PR #8 revalidated the Vulkan failure path against the current ASCENDENDO baseline. The study reported a possible leak after `vkCreateDevice()` succeeded but queue handles were invalid.

Current source inspection shows that this specific leak is already prevented: `createLogicalDevice()` destroys the local `VkDevice` before returning failure when either required queue handle is null.

A separate state inconsistency remains after a surface-aware reconfiguration destroys the previous device and the replacement device cannot be created: `m_initialized` was not explicitly normalized before failure returned.

## Decision

Treat the logical-device resource state as authoritative during surface reconfiguration.

Immediately after destroying the previous device:

```text
m_initialized = false
m_device = VK_NULL_HANDLE
m_graphicsQueue = VK_NULL_HANDLE
m_presentQueue = VK_NULL_HANDLE
```

After successful device recreation:

```text
m_initialized = true
```

If recreation fails, the context remains uninitialized and destructible.

## Rationale

This is the smallest correction to the confirmed invariant. It preserves the existing queue/device selection policy and the already-present local rollback in `createLogicalDevice()`.

No generic transaction abstraction is justified by this evidence.

## Validation boundary

The exact invalid-queue-handle failure is not currently injectable through a public API without introducing Vulkan call mocking or test seams that are outside this tranche. The implementation therefore adds the state normalization required by the confirmed control flow and documents that limitation rather than claiming untested failure injection.

Normal Linux/headless CI and ASan/UBSan remain required evidence.
