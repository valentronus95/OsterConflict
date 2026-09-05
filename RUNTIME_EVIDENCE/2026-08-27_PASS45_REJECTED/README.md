# PASS45 runtime evidence — REJECTED — 2026-08-27

Status: **RUNTIME REJECTED**

Source branch under test: `fix/pass45-runtime-rejection-material-closure-20260826`

This evidence supersedes the 2026-08-26 visual verdict for any conflicting runtime-readiness claim. Source/CI success does not override these observed UE 5.8 failures.

## User-observed factual failures

- AK-family first-person visual appeared briefly (about one second) and then disappeared.
- Multiple expected production models/materials were absent or visually degraded; some skins/material presentation was missing.
- Museum / Culture-House area still presented overlapping or blockout-like building composition rather than clearly separated accepted landmarks.
- HMMWV M2/Browning presentation was visibly misaligned/floating above the intended roof mount.
- BTR-4 presentation was visibly sideways/overturned and therefore failed orientation/vehicle presentation acceptance.
- Overall rendered world/material/LOD quality remained visibly prototype-level and unacceptable for Gate K.

## Root causes/source corrections landed after this rejection

- Weapon fallback/presentation now validates a real renderable mesh rather than accepting the `OC_ProductionWeaponVisual` tag alone.
- Exact M2 uses the authored receiver/mount pivot; rejected bounds recenter / longest-axis guessing is forbidden.
- BTR authored fallback GLB now explicitly maps internal +Z-up coordinates to glTF +Y-up, and the production import contract revision forces a fresh R3 import instead of reusing a sideways R2 asset.
- Culture House structural presentation was migrated from Engine BasicShape construction to committed modular building assets.
- Museum structural presentation was migrated from Engine BasicShape construction to committed modular building assets; missing authored content fails closed rather than rendering a Cube fallback.

## Acceptance state

These corrections are **SOURCE-CODED / RUNTIME RETEST REQUIRED**. They are not runtime acceptance.

Gate K remains OPEN. PR #94 remains OPEN / UNMERGED. A current-head local UE 5.8 run through `START_HERE.cmd -> 2. ПОВНИЙ RUNTIME-ТЕСТ` plus direct visual inspection is still required.
