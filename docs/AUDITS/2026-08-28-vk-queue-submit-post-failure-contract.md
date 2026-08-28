# ASCENDENDO — vkQueueSubmit post-failure contract audit

**Roadmap:** Gate 9.6 — Base Engineering Gate
**Status:** INVESTIGATED / CONTRACT ESTABLISHED
**Date:** 2026-08-28

## Question

Does the `vkResetFences → vkQueueSubmit` sequence require executable fault injection to prove that a failed submit leaves the renderer in a safely recoverable state?

## Observed implementation

`RendererCore::submitFrame()` resets `m_inFlightFence` immediately before calling `vkQueueSubmit()`. Any non-success result returns `FrameStatus::Fatal`.

`RendererFacade::drawFrame()` returns `false` for `FrameStatus::Fatal`.

`main.cpp` treats a failed `renderer.drawFrame(...)` as terminal for the main loop: it logs the renderer failure and executes `break`, followed by `vkDeviceWaitIdle()` during normal shutdown.

## Relevant invariant

The design does **not** require recovery of the failed frame within the same process execution. The caller does not reuse the reset fence or attempt another submission after the failure.

Therefore the relevant invariant for Gate 9.6 is:

> A `vkQueueSubmit()` failure must not be followed by normal frame reuse through the current caller path.

The current control flow satisfies that invariant structurally.

## What is not claimed

This audit does not claim that every Vulkan implementation leaves every synchronization object in a universally reusable state after `vkQueueSubmit()` returns an error. That stronger API-level property is unnecessary for the current terminal error contract and is not established by this code inspection.

## Decision

No new `vkQueueSubmit()` fault-injection seam is warranted at present. Adding one would test a recovery property that the production control flow intentionally does not promise.

Re-open this WP only if a future requirement introduces in-process recovery/retry after submission failure.

## Conclusion

The residual Gate 9.6 question is resolved at the application boundary: submission failure is terminal and fail-closed. The remaining Vulkan gap is therefore limited to other evidence categories, especially queue/capability assumptions, rather than submit-failure recovery.
