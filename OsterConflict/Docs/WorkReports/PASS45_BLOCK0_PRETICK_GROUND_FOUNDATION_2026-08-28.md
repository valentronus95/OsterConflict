# PASS45 Block 0 — pre-tick authored ground foundation

Date: 2026-08-28  
Branch: `fix/pass45-runtime-rejection-material-closure-20260826`  
Parent TZ: `PASS45_RUNTIME_RECOVERY_TZ.md`  
Execution plan: `PASS45_BLOCK_EXECUTION_PLAN.md`  
Status: **SOURCE CODED / LOCAL UE 5.8 RUNTIME PENDING**

## Problem found

`AOCWorldSectorOster` still creates the compact Ground from `/Engine/BasicShapes/Cube` and applies the Engine BasicShape tint during actor `BeginPlay()`. The existing `UOCAuthoredWorldSurfaceUpgradeSubsystem` then replaces Ground with `SM_Plane_1x1` + `M_Inst_Landscape` only after its delayed source-upgrade window. That leaves Block 0 dependent on a late cleanup of a prototype ground state.

## Correction

Added `UOCBlock0GroundFoundationSubsystem` as the Block 0 pre-tick Ground owner.

It:

- runs through `UWorldSubsystem::OnWorldBeginPlay` rather than a tick/timer delay;
- loads the tracked authored ground mesh `/Game/AdvancedVillagePack/Meshes/SM_Plane_1x1`;
- loads `/Game/AdvancedVillagePack/Materials/M_Inst_Landscape`;
- converts the existing compact Cube transform bounds-aware so the 960 x 940 m footprint is preserved;
- preserves the previous Ground top-Z;
- clears the BasicShape material override and binds the authored material;
- fails visibly if the exact Ground component/content/source assumptions are not satisfied;
- emits `PASS45_BLOCK0_PRETICK_GROUND_READY ... runtime_acceptance=0` only for the source postcondition.

The existing delayed world-surface upgrader remains authoritative for its other families and is idempotent for Ground when the exact authored mesh/material are already present, so it does not need to mutate Ground again.

## Verification

Added:

- `VERIFY_PASS45_BLOCK0_GROUND_FOUNDATION.py`;
- `.github/workflows/pass45-block0-ground-foundation.yml`.

The guard rejects timer/tick ownership in the new Ground path and requires the existing late Ground path to remain idempotent.

## Runtime status

This is not Block 0 acceptance. Local UE 5.8 still must prove:

- compile succeeds;
- first visible gameplay frame does not expose the old prototype Ground state;
- full territory ground/grass coverage is coherent;
- roads/sidewalks/buildings/path boundaries have no grass spill or gaps;
- the required five Block 0 screenshot intents pass;
- performance/LOD remains acceptable.
