# Collision-order result — 2026-08-28

## Scope

This closes the 9.6 collision-order characterization performed in PR #100.

## Evidence

The test exercised the same overlapping platform geometry and initial body state with platform storage order permuted between A→B and B→A. It compared final position, velocity, and grounded state.

Validation completed successfully on all three merge-gate environments:

- Linux / Clang / C++20 / Headless Vulkan
- Linux / Clang / ASan + UBSan / Headless Vulkan
- Windows / Clang / C++20

Linux reported 229/229 test cases and 1311/1311 assertions passed. Windows produced the normal evidence artifact.

## Conclusion

No order-dependent divergence was observed for the exercised contact set.

This is evidence for the tested configuration only. It does not establish universal permutation invariance for every possible multi-contact configuration.

Therefore the production collision solver is unchanged and no general collision-order determinism property is marked complete.

## Remaining limitation

A broader determinism claim would require additional contact configurations selected from the actual collision contract. Such work is deferred unless a concrete requirement or counterexample justifies it.

## Provenance

- PR: #100
- Merge commit: `852ef928a6497d9f4054df6767d9ffe1966f6b5f`
- Tested head: `1fc98e11e2a9d5a4a58ba93e6e037e647515bfcd`
