# Work Package — Level clear metadata

## Finding
`Level::clear()` must reset all model state it owns. The current implementation clears only accumulated platforms, leaving `name`, `hasFlag` and `flagBounds` stale.

## Contract
After `Level::clear()`:
- `platformCount() == 0`;
- `name == "Sem Nome"`;
- `hasFlag == false`;
- `flagBounds` is default initialized.

## Scope
- reset metadata in `Level::clear()`;
- add regression test;
- no collision, campaign, parser, or rendering changes.

## Evidence
The stale metadata is observable through the public `Level` fields immediately after `clear()`.
