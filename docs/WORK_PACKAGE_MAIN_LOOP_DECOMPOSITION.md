# Work Package — Main loop architectural decomposition

## Identification

**Roadmap:** `9.6 Base Engineering Gate`  
**Subsystem:** `Runtime`  
**Work Package:** `9.6 Main Loop / GameSession Boundary`  
**Branch:** `refactor/9-6-game-session-boundary`  
**PR:** `#72`

## Discovery

`main.cpp` is the executable entry point, but currently coordinates several independent responsibilities:

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

## Current ownership investigation

`GraphicsRuntime` already owns the non-copyable `Window → VulkanContext → Swapchain → RenderPass → Pipeline → RendererFacade` graph. `PresentationRuntime` now owns the optional text/sprite presentation resources. Recreating an `Application` around these objects would add a second composition owner without solving the remaining gameplay/editor coupling.

The non-graphics state currently co-located in `main.cpp` has a coherent session lifetime:

```text
GameSession
├── GameStateMachine
├── CampaignRuntime
├── Level
├── PhysicsWorld
├── SimulationOrchestrator
├── Player
└── EditorSession
```

`InputManager`, `KeyBindings`, `Camera`, graphics objects and window-title/presentation concerns remain outside this first boundary because they are respectively platform/input adapters or presentation responsibilities.

## Architectural decision

The first extraction is a `logic::GameSession` boundary. It owns the mutable gameplay/editor/campaign state and the state-transition policy, while `main.cpp` remains responsible for process/platform/bootstrap and presentation composition.

`GameSession` must not own Vulkan resources, `Camera`, `Window`, `RendererFacade`, or presentation pipelines. It receives input state and viewport dimensions as data/services and exposes the resulting runtime state needed by the entry point.

This is intentionally **not** an `Application` class. The application composition boundary remains deferred until the remaining bootstrap responsibilities have been measured and their ownership graph is clearer.

## Invariants to preserve

- fixed-step simulation remains unchanged;
- state transitions preserve current user-visible behavior;
- editor return state remains correct;
- campaign streaming order and offsets remain unchanged;
- run recording semantics remain unchanged;
- reset semantics remain unchanged;
- renderer initialization/shutdown ordering remains valid;
- Vulkan device idle/shutdown remains deterministic;
- no new global mutable state is introduced;
- no RenderSnapshot contract is introduced by this extraction.

## Scope

### Includes

- create `GameSession` as a pure runtime/domain orchestration boundary;
- move stateful gameplay/editor/campaign objects and related state transitions out of `main.cpp`;
- keep input and presentation dependencies narrow and explicit;
- preserve existing menu click/navigation, pause/credits/editor transitions, streaming and completion behavior;
- add direct unit tests for extracted transition/session invariants where practical;
- update architecture and roadmap documentation with the resulting ownership model.

### Out of scope

- `Application` class;
- gameplay changes;
- physics tuning;
- `RenderSnapshot` migration;
- graphics bootstrap redesign;
- input-system redesign;
- save/schema redesign;
- adaptive difficulty;
- performance optimisation without profiling.

## Dependency map

```text
main.cpp
  ├── GraphicsRuntime ──→ Window/Vulkan/RendererFacade
  ├── PresentationRuntime ──→ presentation resources
  ├── InputManager
  ├── KeyBindings
  ├── Camera
  └── GameSession
          ├── GameStateMachine
          ├── CampaignRuntime
          ├── Level
          ├── PhysicsWorld
          ├── SimulationOrchestrator
          ├── Player
          └── EditorSession
```

Consumers affected: `main.cpp`, runtime tests and any future runtime code that accesses player/level/state through the entry point. No renderer API consumer is intentionally changed in this tranche.

## Validation target

The refactor must preserve current executable behavior and all existing tests. Additional tests should target `GameSession` state transitions and reset/streaming behavior without coupling tests to graphics implementation details.

### First validation failure

CI run `33026463141` (`Tests`, run #743) failed during `make game -j2` in the `game-build` step.

Observed diagnostic:

```text
main.cpp:210:54: error: no member named 'formatElapsed' in namespace 'logic'
```

Classification: compile failure.  
Confirmed cause: `main.cpp` stopped including `Logic/RunHistory.h` while moving persistence policy into `GameSession`; `logic::formatElapsed` is still declared by that header and is intentionally retained in the entry-point completion message.  
Fix: restore the explicit `Logic/RunHistory.h` include; no behavioral redesign is required.

The source-size checks in the same run passed, so this failure was not caused by the size gate.

### Revalidation after fix

After restoring the include, the following runs completed successfully on the corrected branch commit:

- `Tests` run #746 / workflow run `33026510334` — **success**;
- `Sanitizers` run #8 / workflow run `33026512992` — **success**.

Both workflows completed their build/test paths, source-size checks, headless Vulkan selection, campaign validation and cleanup successfully.

## Definition of Ready

- [x] objective and scope defined;
- [x] normative documents consulted;
- [x] current ownership graph inspected;
- [x] affected consumers identified;
- [x] validation strategy defined;
- [x] exit criteria defined.

## Definition of Done

- [x] `GameSession` owns the agreed runtime/session responsibilities;
- [x] `main.cpp` no longer owns those session rules;
- [x] behavior-preservation tests pass;
- [x] Linux/Clang/headless Vulkan CI remains green;
- [x] campaign validation remains green;
- [x] architecture/roadmap/WP reflect the final ownership model;
- [x] no `Application` monolith or RenderSnapshot migration is introduced unintentionally.

## Evidence / references

- `docs/ROADMAP.md` — 9.6 order and gate;
- `docs/ARCHITECTURE.md` — target architecture;
- `docs/PROJECT_MANAGEMENT.md` — WBS/dependency protocol;
- `Game/Graphics/GraphicsRuntime.h` — existing graphics ownership boundary;
- `Game/Graphics/PresentationRuntime.h` — existing presentation ownership boundary;
- `Game/Core/GameStateMachine.h` — existing state boundary;
- `Game/Logic/CampaignRuntime.h` — existing campaign boundary;
- CI run `33026463141` — concrete compile failure and diagnostic;
- workflow run `33026510334` — corrected normal validation;
- workflow run `33026512992` — corrected sanitizer validation.

## Alterações durante execução

```text
Discovery:
The runtime/session ownership was coherent, but main.cpp still mixed it with process and presentation composition.

Decision:
Extract GameSession, not Application.

Failure discovered during validation:
Missing explicit RunHistory declaration after the extraction.

Correction:
Restored Logic/RunHistory.h include in main.cpp.

Validation:
Tests #746 and Sanitizers #8 both completed successfully.
```

## Fecho

**Resultado:** concluído.  
**Critério de saída:** the first main-loop session boundary is merged only after behavior-preservation CI and sanitizer evidence are green.  
**Dívida residual:** further `main.cpp` decomposition (bootstrap/configuration and frame composition), Windows CI, Vulkan lifecycle/queue evidence and the general RenderSnapshot boundary remain open.
