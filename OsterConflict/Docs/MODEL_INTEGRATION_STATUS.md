# Oster Conflict - Production model integration status

Updated: 2026-08-20

This file separates three different states that were previously easy to confuse:
1. an asset exists somewhere in repository history;
2. an asset is present on `main`;
3. an asset is actually loaded/used by the runtime game.

## Runtime-integrated production assets

### Environment
- AdvancedVillagePack: houses, trees, street lights, bridge, well through `OCAssetModelDecorator`.
- Modular_Rural_Cabin: pines, fences, power poles and rural props through `OCAssetModelDecorator`.
- TileableForestRoad: `SM_Forest_Path` on rural outer routes.
- PN_FoliageCollection: sparse landmark pass plus dense HISM ground-cover pass across the playable runtime sector with hard-surface filtering.
- Scene_UnfinishedBuilding: wall/pillar shell plus floor, upper floor, stairs, alternate wall modules and authored windows at the recovered construction site.
- Scene_RoadsideConstruction: wheelbarrow, gravel, cable wheel, shovel and toolbox, scoped to the unfinished-building site.
- R13.7 Museum: photo-driven replacement model plus source cleanup/runtime validation.

### Weapons
- Assault rifle: `AK-47/Mesh/SM_AK-47` replaces the source-only cube/cylinder proxy when loadable.
- Pistol: `R13/Weapons/Stein/1911/SKM_1911` production skeletal visual.
- SMG: `R13/Weapons/Stein/MP5/SKM_MP5` production skeletal visual.
- Sniper rifle: `R13/Weapons/Stein/M700/SKM_M700` production skeletal visual.
- Source-only weapon geometry remains as a fallback and/or pickup collision path if an imported asset cannot load.

### Characters
- QuantumCharacter production third-person body.
- QuantumCharacter first-person arms.
- Light/Standard/Heavy equipment mapping uses cap, vest, drops and holster modules.
- QuantumCharacter's own Idle/Walk/Run/Fall sequences are selected by movement state only when animation and body skeletons match.
- Existing faction/role gameplay state remains authoritative; production art is presentation-only.

### Vehicles
- VehicleVarietyPack Hatchback, SUV and SportsCar are used by civilian vehicle styles.
- VehicleVarietyPack Pickup replaces the source-only pickup body for the mounted gun truck while existing turret/gameplay physics remain authoritative.

## Restored to the active project but not blindly forced into runtime

- SampleAnimationPack: imported and available. Rifle/ADS sequences are not forced onto QuantumCharacter until skeleton/retarget compatibility is verified in Unreal Editor.
- R13 weapon meshes M14, Mac10, Tec9 and LeverAction are available, but no current gameplay weapon slot maps cleanly to all of them. They should be added as explicit weapon variants rather than mislabeled as LMG/shotgun/etc.
- Slavic Medieval Town kit remains available for selective location work; non-semantic mesh names require visual Content Browser inspection before placement.
- Remaining Scene_UnfinishedBuilding and RoadsideConstruction modules are available for future location-specific dressing.

## Assets still missing for requested production visuals

- No HMMWV/Humvee production mesh is present in the restored VehicleVarietyPack. A civilian pickup is not treated as a Humvee.
- No verified exact Silpo building production mesh is present. The exact Silpo replacement still requires a custom/photo-driven model from accessible references.
- No verified dedicated production LMG or modern shotgun mesh is mapped to the current LMG/Shotgun classes yet.

## Validation boundary

GitHub can verify repository state, asset paths and source integration. It cannot replace an Unreal Engine 5.8 compile/PIE visual run. Imported mesh orientation, authored sockets, material dependencies, animation retargeting and final placement scale must be visually checked in UE before declaring the art pass final.
