# Work Package — EditorRenderer concrete Camera include

**Issue:** #137

## Context

PR #136 exposed a compilation failure in `Game/Graphics/EditorRenderer.cpp`: the implementation instantiates `Camera`, while `ShapeRenderer.h` only provides a forward declaration.

## Decision

Include `Graphics/Camera.h` directly in `EditorRenderer.cpp`.

## Scope

- add the direct include;
- no behavior changes;
- no changes to `Camera` or renderer ownership.

## Validation

Run Linux normal, Linux ASan/UBSan and Windows required workflows.
