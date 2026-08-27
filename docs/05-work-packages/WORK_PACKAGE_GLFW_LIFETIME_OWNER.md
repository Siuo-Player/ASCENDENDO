# Work Package — GLFW lifetime owner

## Identificação

**Roadmap:** `9.6 Base Engineering Gate`  
**Subsystem:** `Graphics / Platform lifecycle`  
**Work Package:** `9.6 GLFW lifetime owner`  
**Branch:** `fix/9-6-glfw-lifetime-owner`  
**PR:** `<to be created>`

## Objetivo

Establish exactly one explicit owner for process-wide GLFW initialization/termination and remove the current split responsibility between `main.cpp` and `gfx::Window`.

## Descoberta / evidência

`main.cpp` creates `GlfwRuntime`, which calls `glfwInit()` and later `glfwTerminate()`. `gfx::Window::create()` also called `glfwInit()`, and `gfx::Window::destroy()` called `glfwTerminate()`.

`GraphicsRuntime` already owns `Window` and controls the destruction order of the graphics resource graph. Consequently, GLFW lifetime was duplicated across two layers.

The code path is normally benign because GLFW initialization is idempotent and the outer `GlfwRuntime` is destroyed after `GraphicsRuntime`, but the ownership contract is ambiguous and the second `glfwTerminate()` has no resource ownership associated with it. More importantly, `main.cpp` also calls GLFW APIs directly (`glfwGetPrimaryMonitor`, `glfwSetWindowTitle`) outside `Window`, so the initialization boundary should be explicit at process level.

## Decisão arquitetural

Keep GLFW process lifetime in a single small RAII owner at the application/process boundary (`main.cpp`), and make `gfx::Window` a pure window wrapper that assumes GLFW is already initialized.

`Window` may remain responsible for creating/destroying the `GLFWwindow*`, but it must not initialize or terminate the GLFW library itself.

This preserves the current `main → GraphicsRuntime → Window` ownership graph while making process-global lifetime explicit and preventing nested library ownership.

## Alternatives considered

1. Let `Window` remain the sole GLFW owner and move monitor discovery out of `main` — rejected because GLFW APIs are used by `main` before and during graphics composition, making implicit initialization too easy to violate.
2. Keep both owners because the calls are currently idempotent — rejected because duplicated process-global lifetime is not an explicit ownership contract and complicates future reuse/tests.
3. Introduce a generic `PlatformRuntime` abstraction — rejected for now; one library with one explicit owner does not justify another abstraction layer.

## Scope

### Inclui

- remove `glfwInit()` / `glfwTerminate()` from `gfx::Window`;
- keep the existing process-level RAII owner in `main.cpp`;
- document the precondition that `Window::create()` requires GLFW to be initialized;
- validate graphics startup, tests and headless Vulkan after the change;
- validate failure cleanup paths where `GraphicsRuntime::init()` returns false.

### Não inclui

- input abstraction redesign;
- window resizing;
- Vulkan queue redesign;
- swapchain recreation redesign;
- `Application` class;
- renderer changes.

## Dependency map

```text
main/process
  └── GlfwRuntime (sole GLFW library lifetime owner)
        └── GraphicsRuntime
              └── Window (GLFWwindow resource only)
                    └── Vulkan surface creation
```

Consumers: `main.cpp`, `GraphicsRuntime`, `Window`, any future code constructing `Window` directly.  
Validation dependencies: GLFW headers/library, existing Linux headless workflow, Windows dependency strategy remains separate.

## Invariants

- GLFW is initialized before any `glfw*` API is used;
- exactly one process-level owner calls `glfwTerminate()`;
- `GLFWwindow` is destroyed before GLFW is terminated;
- Vulkan surface/device/instance destruction remains before window destruction through the existing member destruction order;
- no runtime behavior changes are intended;
- stub builds remain compile-safe when GLFW is unavailable.

## Riscos

| Risco | Probabilidade | Impacto | Mitigação | Estado |
|---|---|---|---|---|
| A direct `Window` user relies on implicit GLFW initialization | baixa | médio | make the precondition explicit and search all consumers before merge | mitigado by repository search |
| Failure cleanup terminates GLFW before Vulkan/window resources are released | baixa | alto | keep process owner alive until the GraphicsRuntime scope has fully destructed | mitigado by current scope ordering |
| Stub build behavior changes | baixa | baixo | preserve no-op `Window` methods under `!GLFW_AVAILABLE` | aberto até CI |

## Validation

- source-size checker;
- Linux normal tests;
- Linux ASan/UBSan;
- headless Vulkan build/startup;
- campaign validation;
- inspect the failure paths of `GraphicsRuntime::init()` to ensure the process-level GLFW owner remains alive until all graphics members have been destroyed.

## Definition of Ready

- [x] ownership problem reproduced from current source;
- [x] consumers searched;
- [x] alternatives recorded;
- [x] implementation scope bounded;
- [x] validation defined.

## Definition of Done

- [ ] only process-level RAII owner initializes/terminates GLFW;
- [ ] `Window` only owns `GLFWwindow*`;
- [ ] all builds/tests pass;
- [ ] failure-path cleanup is validated;
- [ ] docs/architecture/roadmap/tech debt synchronized;
- [ ] PR merged.

## Alterações durante execução

```text
Implementation check:
A first edit removed nested GLFW init/terminate from Window, but introduced a source typo in the GLFW window hint constant (`GLFW.RESIZABLE`).

Classification:
Local compile-time defect introduced by the branch implementation; not a CI/infrastructure failure.

Correction:
The implementation was corrected to use `GLFW_RESIZABLE` before validation.
```

## Fecho

**Estado:** em execução.  
**Próxima decisão:** validate the single-owner lifetime and cleanup ordering; if green, integrate the tranche and continue to the remaining Vulkan lifecycle evidence.
