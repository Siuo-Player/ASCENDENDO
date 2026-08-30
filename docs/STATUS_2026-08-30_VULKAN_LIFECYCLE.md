# ASCENDENDO — Vulkan lifecycle status — 2026-08-30

Current status after PR #188: `VkResult` classification is explicit and shared between acquire and present.

Current characterization tranche: `recreateSwapchain()` fail-closed behaviour after a successful wait-idle boundary followed by a dependent swapchain failure.

The corresponding 2026-08-26 Study recommendation was to characterize the policy before implementing restartable recovery. The current non-resizable window means zero-extent transient recreation is not a suitable synthetic test path at this stage.

The focused characterization deliberately uses existing public lifecycle operations and leaves Vulkan ownership to RAII during teardown.
