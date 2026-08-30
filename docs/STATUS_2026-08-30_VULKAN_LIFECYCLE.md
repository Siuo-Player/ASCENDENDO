# ASCENDENDO — Vulkan lifecycle status — 2026-08-30

Current status after PR #188: `VkResult` classification is explicit and shared between acquire and present.

Current characterization tranche: `recreateSwapchain()` fail-closed behaviour after a post-wait-idle dependent-resource failure.

The corresponding 2026-08-26 Study recommendation was to characterize the policy before implementing restartable recovery. The current non-resizable window means zero-extent transient recreation is not a suitable synthetic test path at this stage.
