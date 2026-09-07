# PASS45 weapon/material closure continuation — 2026-08-26

Status: **CODED_UNTESTED / runtime remains rejected until a new local UE 5.8 playtest**.

Active work began from `main` `69f0f8005ffc4518fcb413a6202eb3e51c21fd1f` after the R14 verifier forward-port. The material problem was not treated as a colour-repair problem. The current work separates stale imported assets, authored source dependencies, explicit content gaps and rendered runtime acceptance.

## Findings

1. Existing Stein raw source folders contain authored external PNG textures beside each FBX, but the imported `/Game/R13/Weapons/Stein/*` folders did not reliably contain the corresponding texture assets.
2. `import_textures=True` on FBX intake therefore was not sufficient proof that external authored PNG dependencies were imported/bound.
3. Production vehicle normal intake previously skipped all work when HMMWV/M2/BTR `.uasset` files merely existed. A stale BTR uasset could therefore survive after the source-side authored-material contract changed.
4. The general production vehicle importer treated missing local BTR FBX as a gap instead of invoking the repository-safe BTR authored GLB fallback already present in source.
5. The cumulative weapon validator mixed absent exact Remington870/M249 production payload with material failure on repository-available canonical assets.

## Source corrections

- Added `Scripts/pass45_reimport_stein_weapon_materials.py`:
  - imports committed PNG textures before each Stein FBX;
  - reimports materials/textures;
  - rejects `DefaultMaterial`, `BasicShapeMaterial`, `WorldGridMaterial`, `_defaultMat` and missing slots;
  - uses UE 5.8 `MaterialEditingLibrary.get_material_used_textures` to require actual texture dependencies;
  - writes only editor-import evidence, never runtime READY.
- Added `PASS45_REIMPORT_STEIN_WEAPON_MATERIALS_UE58.cmd` and cached `TRY_PASS45_STEIN_WEAPON_MATERIALS_UE58.cmd` with revision sentinel `PASS45_STEIN_MATERIAL_CLOSURE_20260826_R1`.
- `VALIDATE_PRODUCTION_MODELS_UE58.cmd` now performs the Stein corrective import and gates repository-available canonical weapons separately from absent exact production payload.
- `import_production_vehicle_assets.py` now has revision `PASS45_MATERIAL_CLOSURE_20260826_R1` and uses:
  - local BTR FBX + authored materials/textures when available;
  - otherwise the repository-safe generated BTR GLB with explicit `M_BTR4_OC_Authored` PBR material.
- `verify_production_vehicle_fresh_load.py` verifies current revision, non-placeholder slots and BTR source/material provenance.
- `TRY_PRODUCTION_VEHICLES_UE58.cmd` no longer treats uasset existence as freshness. It requires the current revision/fresh-load sentinels before skipping reimport.
- `VERIFY_PASS45_WEAPON_MATERIAL_DEPENDENCY_AUDIT.py` is part of `RUN_ALL_VERIFY.py`.

## Remaining factual acceptance

- local UE 5.8 editor reimport succeeds;
- normal `START_HERE.cmd` route uses the current imported material state;
- rendered rack shows authored appearance for every repository-available required weapon;
- BTR-4 renders without the white/default artifact;
- Remington870/M249 exact production payload remains `CONTENT GAP` until real payload exists; no synthetic READY claim;
- Pass45 runtime status remains **RUNTIME REJECTED / CODED_UNTESTED** until the next accepted playtest.
