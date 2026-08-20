# Oster Conflict - Production model integration status

Updated: 2026-08-20

This file separates four states that must not be confused:
1. a source/download exists outside the repository;
2. a source asset is present in repository history/current checkout;
3. Unreal production `.uasset` content has been generated;
4. the runtime actually loads/uses that production asset.

## Runtime-integrated production assets

### Environment
- AdvancedVillagePack: houses, trees, street lights, bridge, well through `OCAssetModelDecorator`.
- Modular_Rural_Cabin: pines, fences, power poles and rural props through `OCAssetModelDecorator`.
- TileableForestRoad: `SM_Forest_Path` on rural outer routes.
- PN_FoliageCollection: landmark vegetation plus dense HISM ground-cover across the playable runtime sector with hard-surface filtering.
- Scene_UnfinishedBuilding: wall/pillar shell plus floor, upper floor, stairs, alternate wall modules and authored windows at the recovered construction site.
- Scene_RoadsideConstruction: wheelbarrow, gravel, cable wheel, shovel and toolbox, scoped to the unfinished-building site.
- R13.7 Museum: photo-driven replacement model plus source cleanup/runtime validation.

### Weapons
- Assault rifle: animated `AK-47/Mesh/SKM_AK-47` skeletal production visual.
- Pistol: `R13/Weapons/Stein/1911/SKM_1911`.
- SMG: `R13/Weapons/Stein/MP5/SKM_MP5`.
- Sniper rifle: `R13/Weapons/Stein/M700/SKM_M700`.
- Explicit restored variants: M14, MAC-10, TEC-9 and Lever Action, each with its own class/tuning and actual R13 skeletal mesh.
- Sandbox automatically exposes a separate restored-weapon rack for M14, MAC-10, TEC-9 and Lever Action so they can be tested without mislabeling existing LMG/shotgun slots.
- Source-only geometry remains only as a safe fallback/pickup collision path when an imported production mesh cannot load.

### First-person weapon presentation
- QuantumCharacter `SKM_Arms` is the first-person arms mesh.
- A local presentation subsystem keeps hands and the active weapon together during recoil, ADS convergence and reload motion.
- The AK uses its own imported `AK-47_Fire_W` and `AK-47_Reload_W` sequences when their skeleton matches the AK skeletal mesh.
- SampleAnimationPack rifle idle/ADS poses are applied to first-person arms only when their skeleton matches the active arms skeleton; incompatible animation is skipped rather than forced.
- This is a presentation layer only. Server fire rate, ammo, reload completion and damage remain authoritative in existing gameplay code.

### Characters
- QuantumCharacter production third-person body.
- QuantumCharacter first-person arms.
- Light/Standard/Heavy equipment mapping uses cap, vest, drops and holster modules.
- QuantumCharacter's own Idle/Walk/Run/Fall sequences are selected by movement state only when animation and body skeletons match.
- Existing faction/role gameplay state remains authoritative; production art is presentation-only.

### Vehicles
- VehicleVarietyPack Hatchback, SUV and SportsCar are used by civilian vehicle styles.
- VehicleVarietyPack Pickup remains the fallback gun-truck visual.
- `AOCPickupGunTruck` now prefers canonical `/Game/Production/Vehicles/HMMWV/SM_HMMWV_UA` and mounts `/Game/Production/Weapons/M2/SM_M2_Browning` on the existing gameplay turret pivot when those assets exist.
- `AOCBTR` now prefers canonical `/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus` as a visual shell while retaining the existing 8-wheel suspension, physics, armor and turret gameplay.
- Production vehicle replacement clears component material overrides with `EmptyOverrideMaterials()` so the imported mesh keeps its authored material slots instead of inheriting fallback visual overrides.
- `Scripts/import_production_vehicle_assets.py` provides deterministic UE 5.8 import paths/names and removes the HMMWV Mk19 scene subtree before importing the HMMWV. `IMPORT_PRODUCTION_VEHICLES_UE58.cmd` is the Editor command-line launcher.
- `UOCProductionVehicleRuntimeValidationSubsystem` performs delayed PIE/runtime validation of HMMWV, M2 and BTR-4 asset availability, usable bounds/material slots and actual use on spawned gun-truck/BTR actors. Fallbacks remain functional, but validation reports a failure until production assets are really active.

## Binary ingest safety

`INGEST_UPLOADED_MODELS_AND_IMPORT.cmd` is locked to `feat/import-hmmwv-btr4-m2` and refuses to start with unrelated tracked local changes. It performs fetch/switch/fast-forward safety checks before touching production binaries.

After Unreal import, `VERIFY_PRODUCTION_MODEL_INGEST.cmd` verifies before the generated asset commit:
- HMMWV GLB, M2 GLB, BTR-4 FBX and all six required BTR textures exist as real local binaries, not tiny unsmudged pointer files;
- canonical HMMWV, M2 and BTR-4 `.uasset` outputs exist;
- the corresponding extensions resolve to the Git LFS filter;
- source binaries already committed in `HEAD` are stored as LFS pointers;
- generated Unreal assets staged in the index are stored as LFS pointers.

The ingest exits with failure and does not commit generated Unreal assets if this verification gate fails.

## Source files received in the current work session but not yet physically stored by the GitHub connector

The conversation upload contains:
- Ukrainian HMMWV Mk19 GLB;
- separate M2 Browning GLB;
- user-selected BTR-4E Bucephalus FBX plus six textures.

The current GitHub connector can mutate repository text/code/branches/PRs but does not expose a local binary/LFS upload operation. Therefore runtime paths, validation and import automation are integrated, but these new source bytes and the generated `/Game/Production/...` `.uasset` files must not be reported as present in GitHub until a real local LFS push/import has occurred.

## Restored content intentionally not scattered blindly into runtime

- Slavic Medieval Town Lite kit is available, but its asset folders are mostly generic `Cube`, `Cube_001`, etc. Without visual Content Browser inspection, blind placement would damage location fidelity rather than improve it.
- Remaining Scene_UnfinishedBuilding and RoadsideConstruction modules are available for future location-specific dressing; they are not randomly scattered through photographed Oster locations.

## Assets still genuinely missing

- A verified dedicated modern production LMG mesh for the current LMG class.
- A verified dedicated modern production shotgun mesh for the current Shotgun class.
- A verified exact Silpo production building mesh/photo-driven final replacement.

## Source/license caveat

The uploaded BTR-4 FBX contains a source/authoring path referring to a GTA San Andreas BTR-4E Bucephalus mod and no license file was included in the upload. It can be used for development integration, but redistribution/public release should wait until the original source/license is verified.

## Validation boundary

GitHub can verify repository state, asset paths, validation code and source integration. It cannot replace an Unreal Engine 5.8 compile/PIE visual run. Imported mesh orientation, authored sockets, material dependencies, animation retargeting, M2 muzzle alignment, hand placement and final vehicle scale must be visually checked in UE before declaring the art pass final.
