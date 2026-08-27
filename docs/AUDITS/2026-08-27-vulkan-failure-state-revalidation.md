# ASCENDENDO — Revalidation: Vulkan failure-state contract

**Date:** 2026-08-27  
**Project:** `Siuo-Player/ASCENDENDO`  
**Source:** `Siuo-Player-PROJECT-STUDIES` PR #8  
**Observed upstream main:** `87c195b559c15cf6b548150499a45c7b04a09f85`  
**Role:** 9.6 Base Engineering Gate evidence

## Finding

The PROJECT-STUDIES revalidation confirms that the lower-level `VulkanContext` failure-state gap remains distinct from the aggregate `GraphicsRuntime::init()` rollback addressed by PR #81.

The relevant sequence is:

```text
GraphicsRuntime::init()
→ VulkanContext::init()
→ create surface
→ VulkanContext::createSurface()
→ reconfigureForSurface()
→ createLogicalDevice()
```

`reconfigureForSurface()` destroys the existing logical device and clears its handles before attempting recreation. `createLogicalDevice()` can then successfully call `vkCreateDevice()` but return `false` if a required queue handle is `VK_NULL_HANDLE`; that failure path does not itself destroy the newly created device.

## Required invariant

After any failed logical-device creation attempt following successful `vkCreateDevice()`:

```text
m_device        == VK_NULL_HANDLE
m_graphicsQueue == VK_NULL_HANDLE
m_presentQueue  == VK_NULL_HANDLE
```

For failed surface-aware reconfiguration, the context must also expose an `isInitialized()` state consistent with its actual resources and remain safely destructible.

## Relationship to PR #81

PR #81 fixes the higher-level aggregate lifecycle:

```text
GraphicsRuntime
→ rollback all owned components
→ neutral state
→ retry-safe init
```

That does **not** prove that every lower-level `VulkanContext` failure path is transactional. The two concerns remain separate work items:

```text
GraphicsRuntime aggregate rollback  → PR #81
VulkanContext device/queue rollback → next focused Vulkan tranche
```

Do not mark the Vulkan lifecycle gate complete solely because PR #81 passes.

## Scope decision

The smallest next engineering tranche is a focused rollback invariant in `VulkanContext`, not a generic Vulkan transaction framework:

```text
failure after resource acquisition
→ local rollback
→ neutral observable state
→ regression test
```

The existing queue/capability selection policy should remain unchanged unless the failure-path analysis proves a separate defect.

## Provenance

This document mirrors the dated revalidation recorded in PROJECT-STUDIES PR #8. The study layer remains read-only with respect to implementation; this file records the finding in the implementation repository so that the RoadMap and subsequent work packages cannot silently lose the distinction.
