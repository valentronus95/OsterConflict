# PASS45 Block 0 — spatial grass coverage gate

Date: 2026-08-28  
Branch: `fix/pass45-runtime-rejection-material-closure-20260826`  
Parent TZ: `PASS45_RUNTIME_RECOVERY_TZ.md`  
Execution plan: `PASS45_BLOCK_EXECUTION_PLAN.md`  
Status: **SOURCE CODED / LOCAL UE 5.8 RUNTIME PENDING**

## Gap found

The existing foliage runtime guard previously accepted a very low total instance threshold after `OC_Block0FullMapGrassComplete`. That completion tag proves that the population cursor finished traversing the configured grid, but it does not prove that line traces actually produced grass across the approved 960 x 940 m playable area. A collision/content regression could therefore leave most of Oster bare while still satisfying the old total-count contract.

## Final correction

The first implementation used a standalone validation-only subsystem. That prototype was retired in the same work cycle because a second tick owner would duplicate full-HISM scans and, more importantly, could disagree with the existing strict `PASS10`/`PASS36` runtime acceptance path.

The final architecture integrates spatial validation directly into `UOCFoliageRuntimeGuardSubsystem`, which is already consumed by strict runtime acceptance.

After `OC_Block0FullMapGrassComplete`, the guard now inspects world-space transforms of final `DenseGrass_*` HISM instances and requires all of the following before `PASS36_LOWCPU_FOLIAGE_RUNTIME_READY` may emit:

- existing profile minimum grass instance count still passes;
- at least 12 of 16 coarse 4 x 4 map bins contain factual grass instances;
- every map quadrant contains at least two occupied bins;
- observed grass reaches within 20% of all four approved playable-area edges;
- dense grass collision remains disabled.

Spatial failure emits both:

`PASS45_BLOCK0_SPATIAL_GRASS_COVERAGE_FAIL ... full_playable_distribution=0 ...`

and the existing hard acceptance failure:

`PASS10_FOLIAGE_RUNTIME_FAIL reason=block0_spatial_grass_distribution_insufficient`

Spatial success emits:

`PASS45_BLOCK0_SPATIAL_GRASS_COVERAGE_READY ... full_playable_distribution=1 ...`

before the existing `PASS10_FOLIAGE_RUNTIME_READY` / `PASS36_LOWCPU_FOLIAGE_RUNTIME_READY` markers, now carrying `spatial_coverage=1` and edge/bin evidence.

## Ownership and performance

There is no second Block 0 foliage coverage tick subsystem. `OCFoliageRuntimeGuardSubsystem` remains the single strict validation owner, samples at 4 Hz during convergence, and stops after terminal success/failure.

## Why this remains unaccepted

This closes the false-PASS path only. It does not prove visual density, grass quality, boundary cleanup, LOD quality or frame-time cost. Those still require the five direct Block 0 UE 5.8 screenshots and runtime observation defined by the execution plan.