# OSTER CONFLICT — WORLD / MODELS PASS

Date: 2026-08-22  
Branch: `feat/oster-world-models-pass-20260822`  
Base: current `main`  
Overall status: **IN_PROGRESS / CODED_UNTESTED**

## Current project focus

Gameplay design is intentionally deferred. Team rules, game modes, balance, class logic and the final match concept are not acceptance targets for this branch.

This branch is limited to the physical Oster world:

- map layout and verified geographic anchors;
- landmark buildings and recognizable Oster locations;
- residential houses and yards;
- production 3D models;
- materials and textures;
- vegetation;
- fences, utility objects and street furniture;
- road/sidewalk visual geometry;
- removal of visible blockout/proxy presentation where a real model already exists;
- runtime visual integration and model ownership.

No map or landmark should be moved merely to make an invented gameplay concept convenient.

## Ownership rule

One site or model family must have one player-facing presentation owner. Existing owners are extended or corrected instead of stacking another decorative subsystem over the same location.

Source-side asset presence is not runtime proof. A model remains `CODED_UNTESTED` until UE runtime confirms scale, pivot, materials, LFS hydration, placement and absence of duplicate/proxy rendering.

## Residential model audit

The existing player-facing residential owner is `AOCAssetModelDecorator`, spawned by `UOCWorldAssetModelsSubsystem`.

Before this pass it used only two full house meshes across the city:

- `SM_House_Var01`
- `SM_House_Var02`

The same two meshes were alternated through Solomii Krushelnytskoi Street and all generic residential blocks. This is the direct source-side reason for the repeated/cloned-house appearance.

The repository already contains authored AdvancedVillagePack variation assets that were not being used:

- `SM_House_Var01_Extra01` … `SM_House_Var01_Extra08`
- `SM_House_Var02_Extra`
- `SM_Tree_Var04`
- `SM_Tree_Var05`
- `SM_Fence_Var01`
- `SM_Fence_Var02`
- `SM_Fence_Var03`

These files are Git LFS managed; repository pointer presence alone does not prove the playtest machine has hydrated payloads.

## Changes already coded

### Residential houses

`AOCAssetModelDecorator` now keeps the existing two authored base house families but layers the matching authored `Extra` mesh at the exact same transform. It does not pretend an accessory mesh is a standalone house.

House selection is deterministic rather than simple A/B parity. The result is multiple facade/silhouette configurations while the existing collision cores remain aligned.

Large positional jitter was deliberately avoided because primitive building cores still own collision. Variation comes from authored model details, restrained yaw and restrained proportions rather than moving visuals metres away from collision.

The authored enterable-house gap on Solomii Krushelnytskoi remains intact.

Status: **CODED_UNTESTED**

### Residential fences

Three AdvancedVillagePack fence families were added alongside the existing old rural fence. Yard fence selection is deterministic instead of one fence repeated everywhere.

Status: **CODED_UNTESTED**

### Trees

AdvancedVillagePack tree variants 04 and 05 are now integrated, increasing broadleaf selection from three to five families. Existing pine families remain.

Status: **CODED_UNTESTED**

### Enterable-house interior and yard props

`AOCEnterableHouse` was still constructing almost all ordinary household dressing from `/Engine/BasicShapes/Cube.Cube` even though the checked-in `Modular_Rural_Cabin` pack already contains suitable real props.

The existing house owner now integrates these authored meshes when available:

- `Old_Sofa`
- `Wooden_Table_Small`
- `Plastic_Chair`
- `Office_Chair`
- `Refrigerator_Old`
- `Wooden_Crate`
- `Metal_Barrel`
- `Wheel_Barrow`
- `Fence_Old_1_2m`
- `Side_Shed`

A shared bounds-fitting helper calculates uniform scale from the actual mesh bounds and grounds the result at the requested local floor/yard height. This avoids hard-coding an assumed import scale for each pack asset.

Dining furniture, work-corner furniture, sofa, refrigerator, clutter, yard fence and backyard shed now prefer the authored model. Cube fallback remains only where the corresponding real asset cannot be loaded or where no checked-in authored model exists yet, such as the current TV/monitor/PC/laptop and some cabinets.

Cosmetic furniture/electronics are `NoCollision` in this model pass so that hidden or partially replaced blockout geometry cannot leave invisible obstacles. Building shell/interior collision, interactive door, windows, light and yard gate remain owned by the existing house systems.

The visible debug label `S08 ENTERABLE HOUSE` is now hidden in game.

Status: **CODED_UNTESTED**

## Landmark model audit

The current source audit found an important distinction between **photo-inspired runtime construction** and an actual Oster-specific production mesh.

### Museum

The current museum presentation stack contains extensive photo-model/detail subsystems, but its main architecture is still assembled largely from Engine Cube/Cylinder primitives plus selected reusable roof/window/detail meshes. No checked-in exact Oster Museum production source model was found under `SourceAssets/Production`.

### Silpo

The current Silpo photo-model stack is also primarily runtime-built geometry plus materials/detail layers. No checked-in exact Oster Silpo production source model was found under `SourceAssets/Production`.

### Culture House

The current Culture House photo-model owner is runtime Cube/Cylinder architecture with photo-inspired proportions/materials. No checked-in exact Oster Culture House production source model was found under `SourceAssets/Production`.

### Production source tree

`OsterConflict/SourceAssets/Production` currently contains production source families for **Vehicles** and **Weapons**, but no checked-in `World`/`Buildings` family containing exact Oster landmark meshes.

Therefore these landmark items are currently an **asset-source gap**, not merely an integration bug. The project must not falsely relabel the existing runtime primitive construction as an exact production mesh.

Status: **AUDITED / ASSET GAP CONFIRMED / RUNTIME PRESENTATION STILL EXISTS**

## Source verification

The dedicated `VERIFY_OSTER_WORLD_MODELS_PASS.py` source contract now covers:

- residential model ownership;
- all nine newly used house detail assets;
- added tree and fence families;
- collision-aligned house placement;
- preserved Krushelnytskoi enterable-house gap;
- real enterable-house prop asset presence and runtime paths;
- bounds-based prop fitting;
- removal of invisible cosmetic furniture/electronics collision;
- preservation of interactive door/window/light/gate ownership.

A green source run does not mean the models are visually approved in UE.

## Next model work in this branch

1. Audit the current landmark stacks for duplicate/blockout layers and remove only confirmed redundant visual owners.
2. Continue replacing repeated generic residential presentation without changing unverified geography.
3. Expand the enterable-house shell/roof/material presentation using authored building modules without covering its real openings.
4. Audit road/sidewalk presentation for raised/convex geometry and duplicate visual owners.
5. Audit visible blockout geometry and z-fighting after the model-owner pass.
6. Prepare an explicit production-world asset intake path for future exact Oster Museum/Silpo/Culture/other building meshes instead of pretending they already exist.
7. Runtime-accept the complete world/model pass in UE before merging to `main`.

## Merge rule

Do not merge this branch to `main` merely because source checks are green. Final merge requires a coherent UE runtime model/map acceptance pass.
