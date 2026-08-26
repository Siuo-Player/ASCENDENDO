# Work Package — Runtime failure exit semantics

## Scope

Ensure fatal runtime/rendering failures do not return success from the process entry point.

## Observation

`main.cpp` breaks out of the frame loop when `RendererFacade::drawFrame()` fails, but execution can then continue to the normal shutdown message and return `0`.

## Required invariant

```text
fatal runtime failure
    ↓
process exit code != 0
```

Normal user-driven shutdown remains exit code `0`.

## Design choice

Track a local runtime failure flag at the entry-point boundary. The renderer remains responsible for reporting whether a frame failed; `main()` is responsible for translating that fatal condition into process semantics.

No exception framework or global error manager is introduced.

## Non-goals

- no CI changes;
- no renderer API redesign;
- no change to normal shutdown behaviour;
- no CampaignID changes.
