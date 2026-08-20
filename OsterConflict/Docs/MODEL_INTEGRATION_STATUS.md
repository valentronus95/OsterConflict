# Oster Conflict - Production model integration status

Updated: 2026-08-20

This file separates states that must not be confused:
1. a candidate/source exists outside the repository;
2. source bytes are physically present in Git/LFS;
3. Unreal production `.uasset` content has been generated;
4. runtime code is prepared to load the canonical asset;
5. Unreal compile/automation/visual validation has actually passed.

## Runtime-integrated production assets

### Environment
- AdvancedVillagePack: houses, trees, street lights, bridge, well through `OCAssetModelDecorator`.
- Modular_Rural_Cabin: pines, fences, power poles and rural props through `OCAssetModelDecorator`.
- TileableForestRoad: `SM_Forest_Path` on rural outer routes.
- PN_FoliageCollection: landmark vegetation plus dense HISM ground-cover across the playable runtime sector with hard-surface filtering.
- Scene_UnfinishedBuilding: wall/pillar shell plus floor, upper floor, stairs, alternate wall modules and authored windows at the recovered construction site.
- Scene_RoadsideConstruction: wheelbarrow, gravel, cable wheel, shovel and toolbox, scoped to the unfinished-building site.
- R13.7 Museum: photo-driven replacement model plus source cleanup/runtime validation.

### Weapons already backed by repository assets
- Assault rifle: animated `AK-47/Mesh/SKM_AK-47` skeletal production visual.
- Pistol: `R13/Weapons/Stein/1911/SKM_1911`.
- SMG: `R13/Weapons/Stein/MP5/SKM_MP5`.
- Sniper rifle: `R13/Weapons/Stein/M700/SKM_M700`.
- Explicit restored variants: M14, MAC-10, TEC-9 and Lever Action, each with its own class/tuning and actual R13 skeletal mesh.
- Sandbox automatically exposes a separate restored-weapon rack for M14, MAC-10, TEC-9 and Lever Action only in Sandbox mode so normal matches are not polluted with test pickups.
- Source-only/proxy geometry remains as a safe fallback and pickup-collision path when a production mesh cannot load.

### Next weapon production contracts already wired in code
- `AOCWeapon_LMG` is now the M249 gameplay class and automatically prefers canonical `/Game/Production/Weapons/M249/SM_M249.SM_M249`.
- `AOCWeapon_Shotgun` is now the Remington 870 gameplay class and automatically prefers canonical `/Game/Production/Weapons/Remington870/SM_Remington870.SM_Remington870`.
- Both hooks use a static production visual while preserving the existing weapon actor, authoritative gameplay tuning and fallback/pickup collision.
- These two canonical assets are **not yet physically present in Git/LFS or Content**. The code hook is ready; the model bytes/import are still pending.

### First-person weapon presentation
- QuantumCharacter `SKM_Arms` is the first-person arms mesh.
- A local presentation subsystem keeps hands and the active weapon together during recoil, ADS convergence and reload motion.
- The AK uses its own imported `AK-47_Fire_W` and `AK-47_Reload_W` sequences when their skeleton matches the AK skeletal mesh.
- SampleAnimationPack rifle idle/ADS poses are applied only when their skeleton matches the active arms skeleton and no authored AnimBlueprint is already authoritative for the arms component.
- Production-weapon single-node fire/reload fallback animations likewise do not replace an authored weapon AnimBlueprint.
- Weapon/arms base transforms and transient single-node animation state are restored on weapon switch, drop/no-weapon state and vehicle transition, preventing ADS/recoil/reload offsets from leaking into later equipment states.
- Reload presentation clamps duration to a safe positive minimum before calculating the visual arc.
- This is a presentation layer only. Server fire rate, ammo, reload completion and damage remain authoritative in existing gameplay code.

### Characters
- QuantumCharacter production third-person body.
- QuantumCharacter first-person arms.
- Light/Standard/Heavy equipment mapping uses cap, vest, drops and holster modules.
- QuantumCharacter's own Idle/Walk/Run/Fall sequences are selected by movement state only when animation and body skeletons match.
- Existing faction/role gameplay state remains authoritative; production art is presentation-only.

### Vehicles: runtime contracts prepared, binary ingest still pending
- VehicleVarietyPack Hatchback, SUV and SportsCar are used by civilian vehicle styles.
- VehicleVarietyPack Pickup remains the fallback gun-truck visual.
- `AOCPickupGunTruck` prefers canonical `/Game/Production/Vehicles/HMMWV/SM_HMMWV_UA` when the asset exists.
- The uploaded HMMWV Mk19 subtree is removed before import and the remaining source scene is rotated into the Unreal vehicle axis convention before Interchange import.
- Production HMMWV fitting uses a 465 x 216 x 275 cm visual envelope and grounds the imported mesh bottom to the fallback wheel plane (`Z=-86 cm`) instead of vertically centering the shell.
- HMMWV turret pivot is moved to the source Mk19 mount area when the production HMMWV is active.
- The separate production M2 uses canonical `/Game/Production/Weapons/M2/SM_M2_Browning`, attaches to `BarrelPivot` so the visible gun follows both yaw and pitch, preserves the authored receiver/mount pivot and moves the authoritative muzzle point to the production barrel location.
- `AOCBTR` prefers canonical `/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus` while retaining the existing 8-wheel suspension, physics, armor and turret gameplay.
- The BTR-4 visual shell is fitted to 776 x 293 x 300 cm and grounded to the 8-wheel suspension plane (`Z=-98 cm`) instead of being centered vertically around the physics body.
- Production vehicle replacement clears component material overrides with `EmptyOverrideMaterials()` so imported meshes keep their authored material slots.
- `UOCProductionVehicleRuntimeValidationSubsystem` performs delayed PIE/runtime validation of HMMWV, M2 and BTR-4 asset availability, usable bounds/material slots and actual use on spawned gun-truck/BTR actors. Fallbacks remain functional, but validation reports a failure until production assets are really active.
- The current BTR-4 FBX is a combined visual shell. Its visible authored turret is not yet separated into a movable mesh, so turret presentation still requires a real UE visual check before merge.

## UE 5.8 vehicle production import contract

`Scripts/import_production_vehicle_assets.py` provides deterministic paths/names, removes the HMMWV Mk19 scene subtree and wraps the remaining HMMWV glTF scene in a canonical source rotation before import.

For HMMWV/M2 GLB import it uses UE 5.8 Interchange with the current controls:
- `InterchangeCombineStaticMeshesBehavior.ALL` instead of the deprecated `combine_static_meshes` boolean;
- `InterchangeForceMeshType.IFMT_STATIC_MESH` so rigid GLB hierarchy is not misclassified;
- baked meshes, static-mesh import enabled, skeletal import disabled and imported collision disabled because gameplay collision remains authoritative.

BTR-4 FBX intentionally stays on the legacy FBX static-mesh importer because the project needs deterministic combined static-mesh output.

Each import task must report the canonical object in `imported_object_paths`; merely finding an old asset at the destination is not accepted as success. The Python script writes `Saved/ProductionAssetImportCache/production_import_success.txt` only after all three canonical vehicle/M2 assets were actually created/updated and saved. `IMPORT_PRODUCTION_VEHICLES_UE58.cmd` deletes any old sentinel before launch and refuses to print PASS unless the new sentinel contains all three canonical paths.

## Binary ingest safety

`INGEST_UPLOADED_MODELS_AND_IMPORT.cmd` is locked to `feat/import-hmmwv-btr4-m2` and refuses to start with unrelated tracked local changes. It performs fetch/switch/fast-forward safety checks before touching production binaries.

The ingest accepts either:
- the original `моделі.zip` layout with a nested BTR archive; or
- normalized `OsterConflict_vehicle_assets_ready.zip` with direct HMMWV GLB, M2 GLB, BTR-4 FBX and the six required BTR textures.

After Unreal import, `VERIFY_PRODUCTION_MODEL_INGEST.cmd` verifies before the generated asset commit:
- HMMWV GLB, M2 GLB, BTR-4 FBX and all six required BTR textures exist as real local binaries, not tiny unsmudged pointer files;
- canonical HMMWV, M2 and BTR-4 `.uasset` outputs exist;
- corresponding extensions resolve to the Git LFS filter;
- source binaries already committed in `HEAD` are stored as LFS pointers;
- generated Unreal assets staged in the index are stored as LFS pointers.

The ingest exits with failure and does not commit generated Unreal assets if this verification gate fails.

## UE 5.8 local validation gate

`Source/OsterConflict/Private/Tests/OCProductionModelTests.cpp` defines `OsterConflict.ProductionModels.CanonicalAssets` automation coverage for the HMMWV, M2 and BTR-4 canonical assets. It checks asset existence, usable bounds, material slots and render LODs, then writes `production_automation_success.txt` only after full PASS.

`VALIDATE_PRODUCTION_MODELS_UE58.cmd` performs the real local gate:
1. build `OsterConflictEditor Win64 Development`;
2. run the production model automation test in `UnrealEditor-Cmd` and export the report;
3. require the explicit success sentinel for HMMWV/M2/BTR-4;
4. launch the standalone Sandbox without frontend for visual inspection of HMMWV scale/orientation/grounding, M2 mount/pitch/muzzle, BTR-4 shell/materials/grounding and first-person weapon presentation.

Repository CI does **not** replace this UE run.

## Repository-level CI contract

`.github/workflows/production-model-contracts.yml` runs on this feature branch and relevant pull-request changes. It validates Python syntax and guards:
- current UE 5.8 Interchange settings;
- HMMWV source-axis preprocessing and production grounding;
- M2 `BarrelPivot`/muzzle contract;
- BTR-4 grounding;
- canonical vehicle paths;
- normalized and nested archive ingest layouts;
- LFS rules and verifier/sentinel handshake;
- runtime validator files;
- FPS presentation transition/AnimBlueprint protections;
- M249 and Remington 870 canonical runtime hooks;
- UE automation/local-validator contracts.

This CI only catches repository-level regressions that can be checked on a normal GitHub runner.

## Source files available outside GitHub LFS in the current work session

The current work session has both the original and normalized vehicle source archives. The normalized archive contains:
- Ukrainian HMMWV Mk19 GLB;
- separate M2 Browning GLB;
- BTR-4E Bucephalus FBX;
- six required BTR textures.

The GitHub connector can mutate repository text/code/branches/PRs but does not expose a binary Git LFS upload operation. Therefore these source bytes and generated `/Game/Production/...` vehicle `.uasset` files must **not** be reported as present in GitHub until a real LFS push/import has occurred on a machine with UE 5.8.

## Verified next weapon source candidates, not yet downloaded into Git/LFS

### M249 LMG candidate
- Source: `https://sketchfab.com/3d-models/m249-9b4c19512e6749248704ff5a5b6d4421`
- Author: Chipotle0303.
- Sketchfab currently lists it as downloadable under Creative Commons Attribution.
- Listed geometry: 37.5k triangles / 19k vertices; Blender + Substance Painter; game-ready intent.
- Target canonical asset after import: `/Game/Production/Weapons/M249/SM_M249`.
- Status: source candidate verified; bytes/import/UE visual check pending.

### Remington 870 shotgun candidate
- Source: `https://sketchfab.com/3d-models/remington-870-pbrgr-26e37df4c38c4e8a9da8adeb4b66bff6`
- Author: tris.blend (`tris09`).
- Sketchfab currently lists it as downloadable under Creative Commons Attribution.
- Listed geometry: 4.4k triangles / 2.4k vertices; PBR, game-ready, explicitly described as Unreal-compatible and ready for animation.
- Target canonical asset after import: `/Game/Production/Weapons/Remington870/SM_Remington870`.
- Status: source candidate verified; bytes/import/UE visual check pending.

## Restored content intentionally not scattered blindly into runtime

- Slavic Medieval Town Lite kit is available, but its asset folders are mostly generic `Cube`, `Cube_001`, etc. Without visual Content Browser inspection, blind placement would damage location fidelity rather than improve it.
- Remaining Scene_UnfinishedBuilding and RoadsideConstruction modules are available for future location-specific dressing; they are not randomly scattered through photographed Oster locations.

## Asset still genuinely unsourced

- Exact/photo-driven Silpo production building replacement.

## Source/license caveat

The uploaded BTR-4 FBX contains a source/authoring path referring to a GTA San Andreas BTR-4E Bucephalus mod and no license file was included in the upload. It can be used for development integration, but redistribution/public release should wait until the original source/license is verified.

The M249 and Remington 870 candidates above are listed by Sketchfab as CC Attribution at the time of verification. Their attribution/license metadata must be retained with any downloaded source before distribution.

## Validation boundary

GitHub can verify repository state, asset paths, validation code and source integration. It cannot replace an Unreal Engine 5.8 compile/automation/visual run. Imported mesh orientation, authored pivots/sockets, material dependencies, animation retargeting, M2 muzzle alignment, hand placement and final vehicle/weapon scale must be visually checked in UE before declaring the art pass final.
