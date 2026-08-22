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

## Next model work in this branch

1. Run source-contract verification for the newly integrated house/fence/tree assets.
2. Audit landmark ownership and model completeness for Museum, Silpo, Culture House, Stadium and College.
3. Continue replacing repeated generic residential presentation without changing unverified geography.
4. Audit road/sidewalk presentation for raised/convex geometry and duplicate visual owners.
5. Audit visible blockout geometry and z-fighting after the model-owner pass.
6. Runtime-accept the complete world/model pass in UE before merging to `main`.

## Merge rule

Do not merge this branch to `main` merely because source checks are green. Final merge requires a coherent UE runtime model/map acceptance pass.
