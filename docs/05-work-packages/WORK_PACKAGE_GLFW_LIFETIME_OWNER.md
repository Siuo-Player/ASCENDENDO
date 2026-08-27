# Work Package — GLFW lifetime owner

## Identificação

**Roadmap:** `9.6 Base Engineering Gate`  
**Subsystem:** `Graphics / Platform lifecycle`  
**Work Package:** `9.6 GLFW lifetime owner`  
**Branch:** `fix/9-6-glfw-lifetime-owner`  
**PR:** `#73`

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
- validate failure cleanup paths where `GraphicsRuntime::init()` returns false;
- give the integration test process an explicit GLFW lifetime owner because integration tests instantiate `Window` directly.

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

integration test process
  └── GlfwTestRuntime (test-process library lifetime owner)
        └── direct Window/Vulkan integration fixtures
```

Consumers: `main.cpp`, `GraphicsRuntime`, `Window`, `Tests/test_runner.cpp`, `Tests/Integration/test_graphics_runtime.cpp`, integration tests that construct `Window` directly and any future code constructing `Window` directly.  
Validation dependencies: GLFW headers/library, existing Linux headless workflow, Windows dependency strategy remains separate.

## Invariants

- GLFW is initialized before any `glfw*` API is used;
- exactly one owner initializes/terminates GLFW within each process;
- `GLFWwindow` resources are destroyed before GLFW is terminated;
- Vulkan surface/device/instance destruction remains before window destruction through the existing member destruction order;
- no runtime behavior changes are intended;
- stub builds remain compile-safe when GLFW is unavailable.

## Riscos

| Risco | Probabilidade | Impacto | Mitigação | Estado |
|---|---|---|---|---|
| A direct `Window` user relies on implicit GLFW initialization | baixa | médio | make the precondition explicit and search all consumers before merge | resolved in repository consumers |
| Failure cleanup terminates GLFW before Vulkan/window resources are released | baixa | alto | keep process owner alive until the GraphicsRuntime scope has fully destructed | validated by scope ordering |
| Stub build behavior changes | baixa | baixo | preserve no-op `Window` methods under `!GLFW_AVAILABLE` | validated by existing compilation path |
| Integration tests relied on Window's implicit `glfwInit()` | confirmed | médio | initialize GLFW once in `Tests/test_runner.cpp` | corrected in branch |
| An integration test terminates the process-wide GLFW runtime midway through the suite | confirmed | médio | remove per-test `glfwInit()`/`glfwTerminate()` and rely on the test-process owner | corrected in branch |

## Validation

- source-size checker;
- Linux normal tests;
- Linux ASan/UBSan;
- headless Vulkan build/startup;
- campaign validation;
- inspect the failure paths of `GraphicsRuntime::init()` to ensure the process-level GLFW owner remains alive until all graphics members have been destroyed.

### First validation failure

CI normal workflow run `33026726288` failed in `make tests -j2`: 11 integration tests that instantiate `gfx::Window` failed at `win.create(...)` because `Window` no longer initializes GLFW implicitly.

The log also shows that `make game -j2` succeeded, source-size checks succeeded and the campaign validator was otherwise reached successfully. The failure was therefore a test-consumer contract issue, not an infrastructure failure.

Correction: `Tests/test_runner.cpp` now owns one `GlfwTestRuntime` for the test process under `GLFW_AVAILABLE`, providing the explicit initialization required by all direct window integration consumers.

### Second validation failure

The next CI normal and sanitizer executions still reported the same 11 `Window` creation failures. Repository inspection identified a remaining test-level owner in `Tests/Integration/test_graphics_runtime.cpp`: that test called `glfwInit()` and later `glfwTerminate()` itself.

Because doctest executes this test before other window consumers, its final `glfwTerminate()` invalidated the process-level test owner for the remainder of the suite. This was a confirmed lifecycle bug in the test composition, not a CI infrastructure failure.

Correction: the test no longer initializes or terminates GLFW. The single `GlfwTestRuntime` in `Tests/test_runner.cpp` now remains the only library lifetime owner for the entire test process.

## Definition of Ready

- [x] ownership problem reproduced from current source;
- [x] consumers searched;
- [x] alternatives recorded;
- [x] implementation scope bounded;
- [x] validation defined.

## Definition of Done

- [x] only a process-level owner initializes/terminates GLFW in production;
- [x] `Window` only owns `GLFWwindow*`;
- [x] integration tests have an explicit test-process GLFW owner;
- [ ] all builds/tests pass after final correction;
- [ ] failure-path cleanup is validated;
- [ ] docs/architecture/roadmap/tech debt synchronized;
- [ ] PR merged.

## Alterações durante execução

```text
Implementation check:
A first edit removed nested GLFW init/terminate from Window, but introduced a source typo in the GLFW window hint constant (`GLFW.RESIZABLE`).

Correction:
The implementation was corrected to use `GLFW_RESIZABLE` before validation.

Validation finding #1:
The first CI revalidation exposed 11 integration failures because direct Window consumers relied on Window's former implicit GLFW initialization.

Correction #1:
Added one explicit RAII `GlfwTestRuntime` to `Tests/test_runner.cpp`.

Validation finding #2:
A subsequent run still exposed the same failures because `Tests/Integration/test_graphics_runtime.cpp` independently called `glfwTerminate()` after its test.

Correction #2:
Removed the test-level GLFW initialization/termination so the test runner remains the sole process owner.
```

## Fecho

**Estado:** em revalidação.  
**Próxima decisão:** confirm the corrected normal and sanitizer workflows; only then close and merge the tranche.
