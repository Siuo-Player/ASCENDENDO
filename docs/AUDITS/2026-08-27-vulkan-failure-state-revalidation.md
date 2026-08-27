# ASCENDENDO — Revalidation: Vulkan failure-state contract

**Date:** 2026-08-27  
**Source:** `Siuo-Player/Siuo-Player-PROJECT-STUDIES` PR #8  
**Observed implementation baseline:** `main` after PR #81  
**Role:** 9.6 Base Engineering Gate evidence

## Revalidation result

The Studies PR identifies two concerns, but current source inspection distinguishes them:

1. **Already fixed in current implementation:** `createLogicalDevice()` destroys the newly-created `VkDevice` when queue-handle validation fails. Therefore the Study's specific device-leak assertion is stale against the current source.
2. **Still confirmed:** `reconfigureForSurface()` can call `createLogicalDevice()` after destroying the previous device, and may return `false` while `m_initialized` still reflects the previous successful initialization state. `createSurface()` then destroys the surface and returns `false`, but does not normalize `m_initialized` itself.

## Confirmed invariant gap

After a failed surface-aware reconfiguration, `VulkanContext` must not report initialized while its logical device is absent or otherwise invalid.

The relevant state transition is:

```text
initialized context
    ↓
destroy old VkDevice
    ↓
reconfigure/createLogicalDevice()
    ↓
FAIL
    ↓
createSurface() returns false
```

The object must have an explicitly defined observable state after this failure.

## Scope

This revalidation does **not** justify a Vulkan transaction framework or queue-selection redesign. The focused next tranche is:

```text
failed surface reconfiguration
→ normalize initialized-state invariant
→ regression evidence
→ preserve existing queue/capability policy
```

The existing `createLogicalDevice()` rollback on invalid queue handles remains part of the confirmed baseline and should not be reimplemented unnecessarily.

## Roadmap consequence

The 9.6 order remains:

```text
GraphicsRuntime aggregate rollback ✅
→ VulkanContext state invariant 🔒
→ Windows/platform evidence 🔒
→ determinism/error-path evidence 🔒
→ Base Engineering Gate review 🔒
→ general RenderSnapshot
```

## Provenance

This document deliberately records both the Study finding and the contradictory current-source evidence. It prevents a stale audit statement from becoming an implementation requirement without revalidation.
