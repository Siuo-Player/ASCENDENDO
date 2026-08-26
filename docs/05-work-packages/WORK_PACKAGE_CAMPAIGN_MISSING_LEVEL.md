# Work Package — Campaign missing-level failure semantics

## Observation

`Level::appendFromFile()` returns the unchanged Y offset when a level file cannot be opened. `CampaignRuntime` must not interpret that result as a successful consumption of the campaign entry.

## Required invariant

For a missing level entry:

```text
loadInitialLevel() == false
or
streamNextLevel() == false

campaign index unchanged
spawn Y unchanged
Level content not advanced by the missing entry
```

A successful step remains one consumed campaign entry followed by the returned next spawn position.

## Design choice

Keep the failure decision at the `CampaignRuntime` boundary by checking the referenced path before delegating to the existing `Level` parser. Do not change `Level::appendFromFile()` semantics in this tranche because its fixed-height return contract is used elsewhere and broader malformed-content semantics belong to validation work.

## Scope

- missing-file detection for initial load and streaming;
- regression tests for non-consuming failure;
- no changes to CampaignID;
- no UGC/web validation;
- no CI workflow changes;
- no Application abstraction.
