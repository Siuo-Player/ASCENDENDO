# ASCENDENDO — Runtime-root independence validation

**Roadmap:** Gate 9.6 — Base Engineering Gate  
**Subsystem:** Runtime / Process bootstrap  
**Status:** VALIDATED / BOUNDED  
**Date:** 2026-08-28

## Property investigated

For the current implementation, `RuntimePaths::fromProcess(nullptr)` should derive its executable-relative resource root independently of the process current working directory.

## Evidence

The executable currently resolves `RuntimePaths` through platform process-path mechanisms before the `argv[0]` fallback. PR #105 added an integration-level unit case that calls `RuntimePaths::fromProcess(nullptr)` from two different temporary current working directories and compares:

- executable root;
- assets root;
- levels root;
- campaign file;
- player sprite path.

The test also confirms that user-data paths remain rooted in the explicit user-data root rather than the repository `Development` tree.

## CI result

The same change passed all three required checks:

- Linux / Clang / C++20 / Headless Vulkan — success;
- Linux / Clang / ASan + UBSan / Headless Vulkan — success;
- Windows / Clang / C++20 — success.

The Linux and sanitizer test summaries reported 230/230 test cases and 1318/1318 assertions. Windows also completed the full suite and campaign validation and produced the standard Windows evidence artifact.

## Interpretation

The evidence validates the scoped property under the tested process-path behavior and the supported CI environments.

It does **not** establish:

- a universal packaging/deployment policy;
- independence of every conceivable `argv[0]` fallback scenario;
- relocatability of an arbitrary installed/bundled distribution;
- behavior for unsupported platforms not represented by the current implementation/CI matrix.

## Decision

No production change is justified. The current process-root resolution is sufficiently evidenced for the Gate's bounded requirement. Future packaging work may define a stronger resource-root abstraction separately if product/deployment requirements demand it.

## Provenance

- PR #105 — `test(9.6): verify runtime path process root ignores CWD`;
- commit merged to `main`: `bec96c323876108bf28021692b5bc9f1ea4fa949`;
- `Game/Core/RuntimePaths.cpp`;
- `Tests/Unit/test_runtime_paths.cpp`;
- `docs/05-work-packages/WORK_PACKAGE_RUNTIME_ROOT_INDEPENDENCE_2026-08-28.md`.
