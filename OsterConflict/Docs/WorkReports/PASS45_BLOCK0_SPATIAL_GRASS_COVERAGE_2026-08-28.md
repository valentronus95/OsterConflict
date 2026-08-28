# PASS45 Block 0 — spatial grass coverage gate

Date: 2026-08-28  
Branch: `fix/pass45-runtime-rejection-material-closure-20260826`  
Parent TZ: `PASS45_RUNTIME_RECOVERY_TZ.md`  
Execution plan: `PASS45_BLOCK_EXECUTION_PLAN.md`  
Status: **SOURCE CODED / LOCAL UE 5.8 RUNTIME PENDING**

## Gap found

The existing foliage runtime guard accepts only a very low total instance threshold after `OC_Block0FullMapGrassComplete`. That completion tag proves that the population cursor finished traversing the configured grid, but it does not prove that line traces actually produced grass across the approved 960 x 940 m playable area. A collision/content regression could therefore leave most of Oster bare while still satisfying the old total-count contract.

## Correction

Added `UOCBlock0FoliageCoverageValidationSubsystem` as validation-only evidence for Block 0.

It waits for the final dense-foliage completion tag and then inspects world-space transforms of the final `DenseGrass_*` HISM instances.

Acceptance requires:

- at least 12 of 16 coarse 4 x 4 map bins contain factual grass instances;
- every map quadrant contains at least two occupied bins;
- observed grass reaches within 20% of all four approved playable-area edges;
- the validator performs no world mutation or repair.

Success marker:

`PASS45_BLOCK0_SPATIAL_GRASS_COVERAGE_READY ... full_playable_distribution=1 mutation=0 runtime_acceptance=0`

Failure marker:

`PASS45_BLOCK0_SPATIAL_GRASS_COVERAGE_FAIL ... mutation=0 runtime_acceptance=0`

## Why this remains unaccepted

This closes a false-PASS path only. It does not prove visual density, grass quality, boundary cleanup, LOD quality or frame-time cost. Those still require the five direct Block 0 UE 5.8 screenshots and runtime observation defined by the execution plan.
