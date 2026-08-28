# ASCENDENDO — Gate 9.6 Final Review

**Date:** 2026-08-28
**Scope:** final Vulkan capability/queue/synchronization review, architecture/ownership review, replay-claim disposition.

## Executive result

Gate 9.6 remains **OPEN until this review is integrated and the repository records the final disposition**. No new production defect requiring implementation was identified in the reviewed areas.

## Evidence review

### Vulkan capability / queues

`VulkanContext` selects a graphics queue and, when a surface exists, a queue family with presentation support. It requires `VK_KHR_swapchain` and an API version compatible with the current 1.3 target. `Swapchain` independently validates presentation support for the selected family and queries surface capabilities, supported formats and present modes before creation.

The application therefore does not assume that graphics and present queues are necessarily the same family. This is an implementation review result, not a claim that every possible Vulkan device is supported.

### Vulkan synchronization / failure semantics

The already integrated evidence covers `vkDeviceWaitIdle()` failure handling. The post-`vkQueueSubmit()` contract is terminal/fail-closed: after fence reset, a failed submit propagates `Fatal`, rendering returns failure and the application leaves the frame loop rather than attempting unsafe reuse of the frame state.

No additional blanket fault-injection layer is justified by the current Gate requirements. Re-open only if a future requirement demands in-process recovery after submit/device failure.

### GraphicsRuntime ownership / rollback

`GraphicsRuntime::init()` normalizes previous state through `cleanup()` and uses aggregate failure rollback after each owned stage. Cleanup proceeds in reverse dependency order: renderer, pipeline, render pass, swapchain, Vulkan context, window.

This establishes a coherent retry/rollback boundary without requiring a new application-wide owner.

### Architecture / ownership

`GameSession` owns session/domain state without Vulkan/presentation ownership. `PresentationRuntime` owns presentation resources but keeps a non-owning pointer to `RendererFacade`. `RendererFacade` owns its renderer components through `unique_ptr` and keeps editor session/supplementary presentation resources as non-owning attachments.

`main.cpp` remains the process/frame composition root. It still passes `Player`/`Level` directly into `RendererFacade`; therefore the general `RenderSnapshot` migration is not complete. This is a known architectural debt and is **not** automatically a Gate 9.6 blocker because no 9.6 requirement currently depends on the full snapshot migration.

No `Application` class is introduced: no additional concrete ownership/lifecycle boundary was demonstrated to justify one.

### Replay claim disposition

The Gate can rely on:

- tick-semantic replay;
- state comparison by tick;
- current frame → `TickInput` boundary behavior.

The following remain future capabilities/properties unless a product decision promotes them into the Gate:

- live-input frame-rate independence across arbitrary render/event cadences;
- complete terminal/result replay of a whole `GameSession`;
- replay persistence/serialization.

No current product decision reviewed in the canonical documentation requires those properties for Gate 9.6.

### Collision / Level / paths

Collision-order evidence remains bounded to the exercised permutation scenario. Runtime-root CWD independence is proven for the tested process-root path on supported CI targets. World/chunk behavior is characterized by #113/#114. Malformed current-level syntax is covered by #92.

## CI / platform evidence

The mandatory merge gate remains:

```text
Linux / Clang / C++20 / Headless Vulkan
Linux / Clang / ASan + UBSan / Headless Vulkan
Windows / Clang / C++20
```

The Windows workflow records toolchain/Vulkan/GLFW provenance and uploads an evidence artifact. `Actions smoke` remains manual/informational.

## Final disposition

```text
Property / claim                                      Status
---------------------------------------------------  -------------------------------
Normal Linux test path                                PROVEN by required CI
ASan/UBSan cleanliness                                PROVEN by required CI
Windows build/test path                               PROVEN for configured environment
GraphicsRuntime aggregate rollback                    PROVEN by code + tests
VulkanContext terminal reconfigure invariant          PROVEN for integrated paths
device-wait failure contract                          PROVEN by #94 evidence
queue-submit post-failure contract                    ESTABLISHED / documented
queue-family capability assumptions                   REVIEWED / ACCEPTED
surface capability checks                             REVIEWED / ACCEPTED
TickInput semantic unit                               PROVEN
TickInput frame boundary                              PROVEN by #116
Replay tick-semantic                                  PROVEN within tested contract
Collision-order universal invariance                  NOT CLAIMED; scenario bounded
Runtime-root CWD independence                         PROVEN for tested targets
World/chunk current behavior                          CHARACTERIZED
Malformed syntax current grammar                      PROVEN
RenderSnapshot general migration                      NOT COMPLETE; future architecture
Live-input FR independence                            FUTURE / NOT A GATE REQUIREMENT
Terminal replay result                                FUTURE / NOT A GATE REQUIREMENT
Replay persistence                                    FUTURE / NOT A GATE REQUIREMENT
Level semantic/schema validation                      FASE 10

Gate 9.6                                            READY FOR FORMAL CLOSE
```

The wording `READY FOR FORMAL CLOSE` is intentionally distinct from `CLOSED`: the Git history must integrate this decision and then record the final Gate state in the canonical roadmap/debt documents.

## Next step after Gate close

Only after the formal close should the project begin the next architectural tranche: general `RenderSnapshot` migration, with a separate WP and explicit ownership/data-boundary design. The existing architecture direction remains valid: presentation data must not expose Vulkan resources or gameplay ownership.
