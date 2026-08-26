# Work Package — PresentationRuntime ownership

## Roadmap

`9.6 Base Engineering Gate`

## Discovery

After `GraphicsRuntime` was wired into `main.cpp`, the entry point still owns the optional presentation resources:

```text
TextPipeline
FontRenderer
SpritePipeline
SpriteRenderer
```

These resources are not part of the core Vulkan bootstrap and are intentionally optional because the renderer has fallbacks. They nevertheless share one lifetime boundary: they are initialized after the core renderer and attached to `RendererFacade` as non-owning references.

## Decision

Introduce `PresentationRuntime` as the smallest ownership boundary for these optional presentation resources.

It owns:

- `TextPipeline`;
- `FontRenderer`;
- `SpritePipeline`;
- `SpriteRenderer`.

It does not own or coordinate:

- `Window`;
- `VulkanContext`;
- `Swapchain`;
- `RenderPass`;
- `Pipeline`;
- `RendererFacade`;
- input;
- gameplay/editor state;
- frame orchestration.

`RendererFacade` remains the presentation submission owner and receives non-owning attachments.

## Invariants

- optional resource failure does not make core graphics initialization fail;
- successful resources are attached only while their owner remains alive;
- destroying `PresentationRuntime` removes its owned resources before the owner `RendererFacade` is destroyed;
- no global state is introduced;
- no gameplay or frame-loop behaviour changes.

## Scope

This tranche creates and tests the ownership boundary only. `main.cpp` wiring is a separate tranche so the resource lifetime and integration remain independently reviewable.

## Non-goals

- no `Application` class;
- no RenderSnapshot migration;
- no text/sprite rendering redesign;
- no gameplay changes;
- no CI workflow changes.

## Exit criteria

- ownership is explicit and non-copyable;
- optional text/sprite initialization remains best-effort;
- lifecycle is covered by integration evidence;
- the subsequent wiring PR can remove the four optional resource objects from `main.cpp` without changing frame orchestration.
