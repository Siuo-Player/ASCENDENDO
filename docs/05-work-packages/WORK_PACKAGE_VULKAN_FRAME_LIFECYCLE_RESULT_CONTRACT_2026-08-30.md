# Work Package — Vulkan Frame Lifecycle Result Contract — 2026-08-30

## Objective

Characterize the narrow result contract shared by Vulkan swapchain acquire and present operations.

The current runtime already intends to treat:

```text
VK_SUCCESS
    -> Ready
VK_ERROR_OUT_OF_DATE_KHR / VK_SUBOPTIMAL_KHR
    -> SwapchainNeedsRecreate
other VkResult failures
    -> Fatal
```

This tranche makes that policy explicit and prevents acquire/present from drifting into different interpretations.

## Evidence

- `RendererCore::beginFrame()` and `RendererCore::submitFrame()` now use the same classification function;
- unit tests cover success, both recreation-triggering results, and representative fatal results;
- no gameplay, camera, queue-selection or swapchain configuration behavior is changed.

## Scope boundary

Included:

- one centralized `VkResult -> FrameStatus` contract;
- focused characterization coverage;
- documentation of the existing lifecycle policy.

Excluded:

- retry policy;
- queue ownership redesign;
- swapchain recreation algorithm changes;
- multi-frame synchronization redesign;
- changes to `VK_SUBOPTIMAL_KHR` policy;
- gameplay or presentation behavior changes.

## Decision rule

This tranche is complete when all required CI gates pass and the classification remains consistent at both acquire and present call sites.

The next lifecycle tranche may address actual recreation failure paths only if a concrete failure mode or untested invariant is demonstrated.
