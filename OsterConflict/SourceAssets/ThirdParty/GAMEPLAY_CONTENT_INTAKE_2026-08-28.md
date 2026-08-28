# Oster Conflict — gameplay content intake

Date: 2026-08-28
Branch: `content/free-gameplay-assets-intake-20260828-r2`
Base: `fix/pass45-runtime-rejection-material-closure-20260826@25e7d3e86b9f94edd137aba21995c0d3d5d66bcc`
Parent execution contract: `PASS45_GAMEPLAY_CONTENT_INTAKE_TZ_2026-08-28.md`
Mandatory gate launcher: `OsterConflict/Scripts/RUN_PASS45_GAMEPLAY_CONTENT_GATE.cmd`

Purpose: acquire missing presentation/audio/animation donor content without replacing Oster Conflict runtime architecture or pretending that downloaded bytes are production-ready.

## Rules

1. Raw downloads live under `OsterConflict/SourceAssets/ThirdParty/Gameplay`.
2. Existing `/Game/QuantumCharacter/Mesh/Modules/SKM_Arms` remains canonical until a retarget/runtime pass proves a replacement is better.
3. UE 5.8 supplies Character Movement, Chaos, PhysicsAsset, Niagara, IK/Control Rig and spatial audio. No third-party framework replaces those systems.
4. Network sources are pinned to immutable commits or direct upstream URLs. Known hashes are verified; every acquired local file receives a SHA-256 receipt.
5. Ambiguous licenses remain candidates and are not promoted to shipping content.
6. No downloader silently overwrites production `.uasset` files.
7. `DOWNLOADED` is not a completion state. The mandatory gate in `PASS45_GAMEPLAY_CONTENT_INTAKE_TZ_2026-08-28.md` controls acquisition, migration, integration, optimization and runtime acceptance.

## Automated intake

`OsterConflict/Scripts/FETCH_GAMEPLAY_CONTENT_58.cmd` acquires:

- `Ayush-Mohanty/FPS-Arms-3D@c80a452e680c5a27c9e936176eb17e446869529a` — FP arms / AK-74M animation donor. Repository MIT; nested AK animation attribution file is retained and must be honored if shipped.
- `petroulacl/fps-asset-kit@a19b7458a593598211c95ec46ef4eb4b6d1f94d7` — `sfx/` and `weapons/` candidate source only. Source README asserts CC0 provenance; exact selected shipping files must be re-verified.
- `yegors/hard-lines@7c1f90b0295030e60cf9ea731371156466ddb181` — two CC0 12-gauge report candidates plus four CC0 concrete footsteps, with source provenance retained.
- `yashimosh/border-run@38d6afa442a06bbd414f21ac05c0f1b4d08bd705` — six CC0 engine RPM loops as generic vehicle-audio prototype material, not claimed to be factual HMMWV/BTR recordings.
- OpenGameArt `Forest_Ambience.mp3` — verified SHA-256 `9850AA1D0D5D66BD9C5DAF8BB77C6D852E01F2F4DE22F283BD5621E8BED13B75`.
- OpenGameArt `fire.wav` — verified SHA-256 `85CA0CC60D0C037FFF8B185E31AD1FCDBDA6CE45EEE17C3EE1318D1B8F59E330`.

The script writes `LOCAL_CONTENT_RECEIPT.csv` for exact acquired bytes.

## Fab / separate-project intake

Fab content added directly to Oster Conflict remains local until committed/pushed from the workstation. Full-project products such as `SuperSimpleFPSPack`, `GameAnimationSample` and `InteractionSystem2` must be inventoried before migration because Unreal package paths and dependencies can collide. Do not copy `.uasset` files blindly into arbitrary subfolders.

Run `OsterConflict/Scripts/RUN_PASS45_GAMEPLAY_CONTENT_GATE.cmd` after switching to this branch. It runs acquisition, local inventory and the source-level PASS45 content verifier. The generated inventory is the factual input for the migration/import pass.

## Still not solved by downloading random packs

- FP arms visibility / owner visibility / camera attachment / AnimBP / transforms.
- Per-weapon ADS calibration and grip IK.
- Remington pump, M249 feed/charge and other exact moving-part animation.
- Authored Niagara smoke/muzzle/impact/explosion presentation.
- Exact final firearm/vehicle audio identity.
- Runtime acceptance of any imported asset.
