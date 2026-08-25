# Pass45 generic visual owner retirement — 2026-08-25

Status: **CODED_UNTESTED / RUNTIME REJECTED**

This corrective continuation follows the 2026-08-25 Pass45 runtime rejection. The visible AdvancedVillagePack residential/fence presentation and unreferenced rural/recovered showcase layers are not accepted Oster production content.

## Retired runtime owners

Physically removed from current source:

- `OCWorldAssetModelsSubsystem.h/.cpp` — runtime owner that spawned the generic decorator and hid semantic world proxies;
- `OCAssetModelDecorator.h/.cpp` — mass placement of AdvancedVillagePack houses/fences plus rural shed/props rejected by runtime evidence;
- `OCRecoveredEnvironmentSubsystem.h/.cpp` — already-inert unverified recovered environment owner;
- `OCRecoveredBuildingDetailsSubsystem.h/.cpp` — already-inert recovered unfinished-building detail owner;
- `OCRecoveredRoadsidePropsSubsystem.h/.cpp` — already-inert orphaned roadside/construction prop owner.

## Retired stale CI

- `VERIFY_OSTER_WORLD_MODELS_PASS.py` — historical verifier explicitly required the rejected generic decorator and AdvancedVillagePack residential/fence families;
- `.github/workflows/oster-world-models-pass.yml` — historical workflow for that stale contract.

`VERIFY_PASS45_STALE_RUNTIME_RETIREMENT.py` now locks the rejected generic world/decorator owners out of current source. Git history remains the archive; compiled no-op compatibility shells are not retained.

## Current visual ownership

The semantic `AOCWorldSectorOster` baseline remains authoritative where no accepted photo/georeference-backed replacement exists. This is intentional fail-visible recovery, not a claim that the baseline is final visual fidelity.

## Open content truth

BTR-4 and remaining weapon material/texture closure is not declared complete. Current source expects `/Game/Production/...` assets, while the repository tree does not currently expose the expected `Content/Production/Vehicles/BTR4` payload. No generated grey/white material substitution is authorized. Missing authored dependencies remain `CONTENT GAP` until verified content exists and local UE runtime proves them.

## Acceptance

Source deletion does not equal runtime acceptance. Required next proof remains local UE 5.8 build/playtest showing:

- rejected generic fence/house/tower/shack visuals absent;
- no black-world regression;
- Museum/Culture House/Silpo remain spatially correct;
- BTR/weapon authored materials are non-default where payload exists;
- no vehicle teleport regression;
- normal gameplay respects the Pass45 fullscreen/60 FPS recovery contract.
