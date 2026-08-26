# Work Package — Pipeline initialization failure safety

## Scope

Harden the graphics `Pipeline::init()` failure paths without changing frame-loop behaviour or the #61 `GraphicsRuntime` wiring.

## Observation

`Pipeline::init()` acquires resources incrementally:

```text
shader modules
    ↓
pipeline layout
    ↓
graphics pipeline
```

Several failure returns occur after one or more Vulkan objects have already been created. The current implementation does not guarantee that all acquired objects are released on every failure path, and `m_ctx` can remain non-null even when initialization fails.

## Required invariant

After any `Pipeline::init(...)` return of `false`:

```text
m_initialized == false
m_pipeline   == VK_NULL_HANDLE
m_layout     == VK_NULL_HANDLE
m_ctx        == nullptr
```

No shader module created during the failed attempt may remain alive.

After a successful initialization the existing public behaviour remains unchanged.

## Design choice

Use local RAII for temporary shader modules and a single failure cleanup path for the pipeline-owned persistent resources. Do not introduce a general graphics resource manager.

This keeps ownership local to `Pipeline` and makes failure safety explicit.

## Evidence

The existing renderer integration test already exercises successful pipeline initialization. This work package adds failure-path reasoning and cleanup invariants without requiring changes to CI configuration.

## Non-goals

- no change to `GraphicsRuntime` ownership;
- no change to `RendererCore` frame orchestration;
- no change to shader selection or pipeline semantics;
- no CI workflow changes.
