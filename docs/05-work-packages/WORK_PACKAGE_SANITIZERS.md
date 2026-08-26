# Work Package — ASan/UBSan CI coverage

## Roadmap

`9.6 Base Engineering Gate` → P2 transversal / CI evidence

## Objective

Create an explicit GitHub Actions job that instruments both the test objects and the production `Game` library with AddressSanitizer and UndefinedBehaviorSanitizer.

## Discovery

The current `Makefile` defines Linux debug flags with ASan/UBSan, but `GAME_OBJS` are compiled through `CXXFLAGS_REL` while test objects use the debug configuration. Therefore the existing `make tests` path does not guarantee that all production Game code linked into the test binary is sanitizer-instrumented.

## Scope

### Includes

- a separate sanitizer workflow on Ubuntu;
- Clang/C++20;
- headless Vulkan dependencies and deterministic software ICD;
- KiB source-size checks;
- sanitizer flags applied to both Game and test compilation/linking through Make variable overrides;
- leak detection and fail-fast sanitizer configuration.

### Does not include

- changing `.github/workflows/tests.yml`;
- changing the Makefile;
- Windows CI;
- hardware/GPU matrix;
- fuzzing or property-based testing.

## Decision

Use a separate workflow so sanitizer evidence is independently observable. Override the Makefile's release variables at invocation time instead of changing normal build semantics.

This preserves the current build model while proving the missing property: sanitizer instrumentation reaches production Game objects used by the tests.

## Alternatives considered

1. Change `CXXFLAGS_REL` globally to include sanitizers — rejected because release/game builds should not implicitly inherit debug instrumentation.
2. Reuse `make tests` unchanged — rejected because it does not prove that `GAME_OBJS` are sanitizer-instrumented.
3. Add a sanitizer build mode to the Makefile now — deferred; useful later if multiple sanitizer jobs need the same configuration.

## Risks

| Risk | Probability | Impact | Mitigation | State |
|---|---|---|---|---|
| Vulkan/GLFW under ASan exposes environment noise | medium | medium | keep deterministic LVP + Xvfb and classify failures | open |
| Link flags differ between platform packages | low | medium | Linux-only job, same toolchain as current baseline | mitigated |
| Sanitizer runtime reports an existing defect | medium | high | fix in the same branch when reproducible | open |

## Validation

- sanitizer workflow completes successfully;
- build and test commands return zero;
- ASan leak detection remains enabled;
- UBSan halts on runtime UB;
- campaign validation still passes;
- the normal `Tests` workflow remains unchanged.

## Definition of Done

- [ ] sanitizer job is observable in GitHub Actions;
- [ ] production Game objects are built with ASan/UBSan in the sanitizer job;
- [ ] all sanitizer tests pass;
- [ ] result is documented in the PR;
- [ ] no normal build flags were changed.

## Next decision

Windows CI remains a separate evidence item because the repository currently does not contain the expected vendored `external/glfw/lib-vc2022` binaries.
