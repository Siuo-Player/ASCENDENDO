# Work Package — Render failure exit semantics

## Roadmap

`9.6 Base Engineering Gate`

## Invariant

A fatal `RendererFacade::drawFrame()` failure must produce a non-zero process exit status.

```text
drawFrame() == false
    -> diagnostic
    -> shutdown via RAII
    -> return non-zero
```

Normal user-driven shutdown remains exit code `0`.

## Decision

Do not introduce a new runtime class for this. The entry point is already the correct owner of the process exit policy; renderer failure is an error outcome, not a new subsystem.

## Status

Implemented in the development branch, pending PR review/CI validation.

## Scope

- preserve existing renderer error diagnostic;
- return non-zero on fatal presentation failure;
- preserve RAII destruction order;
- no gameplay behavior changes;
- no CI workflow changes.
