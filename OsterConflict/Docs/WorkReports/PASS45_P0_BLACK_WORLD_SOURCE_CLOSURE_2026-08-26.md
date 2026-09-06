# PASS45 P0 BLACK-WORLD SOURCE CLOSURE — 2026-08-26

Status: **CODED_UNTESTED / RUNTIME REJECTED 2026-08-25 REMAINS AUTHORITATIVE**

Branch: `fix/pass45-runtime-rejection-material-closure-20260826`
PR: #94
Code milestone before documentation sync: `9ba84273d06a6c210c808ceacbe96c45466a3d73`
Source workflows at that milestone: **39/39 PASS**

## Root-cause audit

The accepted current daylight owner is `AOCVisualEnvironment` and is component-owned/replicated. The audited runtime path did not reveal a later camera/post-process owner that supersedes its exposure contract.

The rejected source combination was concrete rather than speculative:

- `SunLight->SetIntensity(4.0f)`;
- `r.DefaultFeature.AutoExposure=False`.

For UE 5.8 physical-light units this leaves the outdoor scene severely underlit and is a credible source cause for the rejected near-black world.

## Corrective renderer contract

The source contract is now intentionally paired:

- Directional Light: `120000 lux`;
- `r.DefaultFeature.AutoExposure=True`;
- `r.DefaultFeature.AutoExposure.ExtendDefaultLuminanceRange=True`.

No one element is accepted as an isolated tweak. Source CI rejects restoration of the 4-lux/disabled-exposure contract.

## World material ownership

`AOCWorldSectorOster` remains the accepted semantic owner for:

- `Ground`;
- `Roads`;
- `Sidewalks`.

The audit found no confirmed second late `SetMaterial` owner for those families.

`OCRuntimeAcceptancePass6Subsystem` was initially flagged by a broad static detector, but direct source inspection proved its `Roads/Sidewalks` references are for obsolete BASE instance removal while its `SetMaterial()` path is weapon-only. The detector was narrowed to local material-write context; runtime validation was not weakened.

## Runtime fail-visible validation

`UOCWorldGeometryStabilitySubsystem` now checks semantic material state at the 12-second baseline and through the 16/20-second stability samples.

Each tracked Ground/Roads/Sidewalks family must have:

- a component;
- a material;
- a `UMaterialInstanceDynamic`;
- a `Color` vector parameter.

Failure emits `PASS12_WORLD_GEOMETRY_STABILITY_FAIL` with a family-specific reason.

Successful source/runtime contract markers:

- `PASS45_DAYLIGHT_EXPOSURE_CONTRACT_READY`;
- `PASS45_WORLD_MATERIAL_BASELINE_READY`;
- `PASS12_WORLD_GEOMETRY_STABLE`;
- `PASS45_WORLD_MATERIAL_STABLE`.

## Strict acceptance linkage

`VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py` now requires the daylight and world-material stability evidence and forbids `PASS12_WORLD_GEOMETRY_STABILITY_FAIL`.

`VERIFY_PASS45_STRICT_RUNTIME_ACCEPTANCE_HARNESS.py` source-gates that linkage so the black-world validation cannot become an optional side test while the strict main route still passes.

## Source verification

At code head `9ba84273d06a6c210c808ceacbe96c45466a3d73`:

- `World geometry stability pass 12`: PASS;
- `Pass 45 strict runtime acceptance harness`: PASS;
- `Runtime recovery Pass 45`: PASS;
- `Source verification`: PASS;
- full current PR source matrix: **39/39 PASS**.

## Runtime truth

This report does not claim the visual defect is fixed in UE runtime.

Pass45 remains **RUNTIME REJECTED 2026-08-25** until `START_HERE.cmd -> 2. ПОВНИЙ RUNTIME-ТЕСТ` proves all of the following on the target UE 5.8 machine:

- no large black world/ground region;
- no blown-out white scene after the physical-light correction;
- readable Ground/Roads/Sidewalks;
- required Pass12/material markers present with no FAIL marker;
- direct screenshot evidence.
