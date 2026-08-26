# Work Package — Main loop architectural decomposition

**Roadmap:** `9.6 Base Engineering Gate`

## Discovery

`main.cpp` is the executable entry point, but currently owns several independent responsibilities:

1. GLFW/Vulkan/window bootstrap and shutdown;
2. asset/config/campaign loading;
3. construction and reset of gameplay services;
4. runtime state machine (`MENU`, `PLAYING`, `PAUSED`, `CREDITS`, `EDITOR`);
5. frame timing/input polling;
6. fixed-step simulation and collision resolution;
7. level streaming and campaign completion;
8. editor entry/exit transitions;
9. persistence of run results;
10. presentation submission.

The source-size warning is only the inspection signal. The reason for this work package is responsibility concentration and coupling.

## Observed coupling

The current loop directly coordinates `Window`, `VulkanContext`, `Swapchain`, rendering pipelines, `RendererFacade`, `InputManager`, `KeyBindings`, `Level`, `PhysicsWorld`, `Player`, `Camera`, `EditorSession`, campaign data and persistence. This makes lifecycle/state transitions difficult to isolate and makes later `RenderSnapshot` migration unnecessarily coupled to the entry point.

## Architectural hypothesis

A first decomposition should establish explicit ownership boundaries around:

```text
Application/bootstrap
    platform + graphics initialization
    configuration/campaign discovery

GameStateMachine / application state
    MENU
    PLAYING
    PAUSED
    CREDITS
    EDITOR

Runtime/frame orchestration
    input
    fixed-step simulation
    streaming
    state transitions
    presentation submission
```

The exact classes/names are not fixed. Extraction is justified only when the ownership and lifetime semantics become clearer.

## Invariants to preserve

- fixed-step simulation remains unchanged;
- state transitions preserve the current user-visible behavior;
- editor return state remains correct;
- campaign streaming order and offsets remain unchanged;
- run recording semantics remain unchanged;
- renderer initialization/shutdown ordering remains valid;
- Vulkan device idle/shutdown remains deterministic;
- no new global mutable state is introduced.

## Out of scope

- gameplay changes;
- physics tuning;
- new editor functionality;
- `RenderSnapshot` migration;
- save/schema redesign;
- adaptive difficulty;
- performance optimisation without profiling.

## Validation target

The refactor must preserve the current executable behavior and all existing tests. Additional tests should target extracted state/transition logic where this can be done without coupling tests to implementation details.

## Exit criteria

- bootstrap, state coordination and frame orchestration have explicit ownership boundaries;
- `main()` becomes orchestration rather than the owner of all runtime policy;
- no behavior change in the existing state machine;
- CI Linux/Clang/headless Vulkan remains green;
- campaign validation remains green;
- architecture documentation is updated with the resulting ownership model;
- the Gate records what remains for RAII/ownership and RenderSnapshot rather than treating this extraction as the end of 9.6.
