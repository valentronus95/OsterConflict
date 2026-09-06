# PASS 45 — Gate E reference-driven residential runtime closure

Date: 2026-08-26
Branch: `fix/pass45-runtime-rejection-material-closure-20260826`
PR: #94
Source milestone: `994225835c755f11ad202821ee141f6d1de0e78d`
Source CI: **40/40 current PR workflows PASS**
Runtime truth: **RUNTIME REJECTED 2026-08-25 / local UE 5.8 retest pending**

## Root gap

The existing Pass45 residential retirement removed the known primary source owners (`AOCEnterableHouse`, `BuildResidentialBlocks()` and the generic Krushelnytska private-house/fence generator), but that source fact alone could not prove that another actor, imported mesh or late startup owner did not resurrect the rejected generic village/tower/shack presentation in the final gameplay world.

## Current Gate E implementation

`UOCReferenceDrivenResidentialValidationSubsystem` is a validation-only `UWorldSubsystem` for the normal `OsterConflict_Runtime` gameplay world.

It runs one observation after the startup window (`2.0 s`) and never destroys, hides, recolors, relocates, replaces or spawns world content.

The runtime scan rejects non-zero instances for retired generic component families:

- `Buildings`;
- `ResidentialRoofs`;
- `ResidentialDetails`;
- `WoodFences`;
- `MetalFences`;
- `LightSheetFences`.

It also rejects actor/static-mesh identities carrying the currently rejected presentation families/tokens:

- `AdvancedVillagePack`;
- `OCEnterableHouse`;
- `DarkTower` / `SteepRoofTower` / `SteepRoof`;
- `Shack`;
- `Tower`.

A broad token match is intentionally fail-visible: if a future reference-approved asset legitimately contains one of those names, it must be audited and explicitly reconciled rather than silently exempted.

## Runtime evidence contract

Success:

`PASS45_REFERENCE_DRIVEN_RESIDENTIAL_RUNTIME_READY`

Failure:

`PASS45_REFERENCE_DRIVEN_RESIDENTIAL_RUNTIME_FAIL`

The strict Pass45 evidence verifier now requires READY and rejects FAIL. Automated PASS records:

`REFERENCE_DRIVEN_RESIDENTIAL_RUNTIME_CONTRACT=PASS`

This evidence is part of the same `START_HERE.cmd -> 2. ПОВНИЙ RUNTIME-ТЕСТ` chain and cannot be satisfied by a separate optional launcher.

## Source guardrails

`VERIFY_PASS45_REFERENCE_DRIVEN_RESIDENTIAL_RETIREMENT.py` now verifies both the historical source retirement and the new runtime validator. It fails if the validator becomes a mutating repair owner or if Gate E is detached from strict evidence.

`RUN_ALL_VERIFY.py` includes this verifier, so `Source verification` protects the contract.

At source milestone `994225835c755f11ad202821ee141f6d1de0e78d`, all **40/40** current PR workflow runs completed successfully, including:

- `Source verification`;
- `Pass 45 reference-driven residential retirement`;
- `Pass 45 strict runtime acceptance harness`;
- `Runtime recovery Pass 45`;
- the current historical regression matrix.

## Acceptance boundary

This closes the **source/runtime-evidence wiring** for Gate E only. It does not claim that the rejected dark tower/shack or generic fence/house family has factually disappeared from the user's machine.

Gate E remains `CODED_UNTESTED` until a local UE 5.8 run produces `PASS45_REFERENCE_DRIVEN_RESIDENTIAL_RUNTIME_READY` and the visual playtest/screenshot confirms no rejected generic residential/tower/shack presentation is visible.
