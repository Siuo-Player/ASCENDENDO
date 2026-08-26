# Work Package — PresentationRuntime wiring

## Roadmap

`9.6 Base Engineering Gate`

## Purpose

Wire the already-tested `PresentationRuntime` into `main.cpp` without changing frame orchestration, gameplay, input, or editor behaviour.

## Boundary

Before this tranche, `main.cpp` directly owned:

- `TextPipeline`;
- `FontRenderer`;
- `SpritePipeline`;
- `SpriteRenderer`.

After this tranche, ownership is:

```text
GraphicsRuntime
    └── core Vulkan graph

PresentationRuntime
    ├── TextPipeline
    ├── FontRenderer
    ├── SpritePipeline
    └── SpriteRenderer

main.cpp
    └── orchestration only
```

## Invariants

- optional text initialization remains best-effort;
- optional sprite initialization remains best-effort;
- BitmapFont/rectangle fallbacks remain unchanged;
- `RendererFacade` remains the non-owning presentation attachment boundary;
- no `Application` class is introduced;
- frame-loop logic is unchanged;
- `CampaignID` and `CampaignRuntime` remain independent;
- renderer/resource destruction order is preserved by object scope.

## Exit criteria

- four optional presentation objects are removed from direct `main.cpp` ownership;
- `PresentationRuntime` is initialized exactly once in the graphics scope;
- logging reflects the readiness state from `PresentationRuntime`;
- existing frame orchestration compiles unchanged;
- CI passes build/tests and active campaign validation.
