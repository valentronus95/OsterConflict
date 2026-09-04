# Asset Status

Canonical inventory for Oster Conflict / PASS45. This file tracks every asset pack, model family, donor source, external UE sample/project, and user-added import that is currently evidenced by the canonical branch, prior local `git status`, Content Browser screenshots, explicit user import/download reports, or existing project audits.

## Rules

- This file is the single current asset inventory. New observed imports are added here immediately.
- A directory/root row covers **all files recursively under that import root**, including meshes, materials, textures, animations, sounds and Blueprints. We do not create thousands of meaningless rows for each texture file.
- Nothing may disappear from this inventory silently. If a resource is replaced, the old owner is marked `RETIRED/REMOVE` and then physically removed when safe; Git history is rollback.
- `✅` = the fact stated in that column is fully verified.
- `🟡` = present/known or wired, but exact local package identity or required UE 5.8 runtime acceptance is still pending.
- `❌` = not yet integrated, missing, blocked, or explicitly scheduled for retirement.
- `Git` = verified on canonical PASS45 branch. `LOCAL` = evidenced by local git-status/Content Browser/user import. `REPORT` = explicitly reported by the user but its exact ignored local package path is not remotely enumerable.
- GitHub cannot enumerate ignored local `.uasset` payloads on the user's `C:` drive. Those assets stay in this ledger from local evidence instead of being falsely declared absent.

## Vehicles and mounted weapons

| Present | Integration | Asset / pack | Evidence / current owner |
|---|---|---|---|
| ✅ | 🟡 | HMMWV UA | `Content/Production/Vehicles/HMMWV/SM_HMMWV_UA`; `AOCHMMWVGunTruck`; UE acceptance pending |
| ✅ | 🟡 | M2 Browning .50 | `Content/Production/Weapons/M2/SM_M2_Browning`; mounted on HMMWV and gun-truck paths; UE acceptance pending |
| ✅ LOCAL | 🟡 | BTR-4 Bucephalus | local-only `Content/Production/Vehicles/BTR4/`; source `BTR4_Bucephalus.fbx`; `AOCBTR`; UE acceptance pending |
| ✅ | 🟡 | Vehicle Variety Pack | `Content/VehicleVarietyPack/`; exact pickup mesh used by `AOCPickupGunTruck`; UE acceptance pending |
| ✅ | 🟡 | Pickup + M2 gun truck | `AOCPickupGunTruck`; pickup shell + production M2; UE acceptance pending |
| 🟡 REPORT | ❌ | Additional user-added pickup model | user-reported Fab pickup; exact local package identity still to be reconciled against VehicleVarietyPack/current Content |

## Firearms and launchers

| Present | Integration | Asset / identity | Evidence / current state |
|---|---|---|---|
| ✅ | 🟡 | AK-47 with animations | `Content/AK-47/`; runtime weapon/animation path exists; UE acceptance pending |
| ✅ LOCAL | 🟡 | AK-74M | `Content/ak-74m/`; `IMP_AK74M`; exact local model bridge |
| ✅ LOCAL | 🟡 | AR-15 | `Content/ar15-rifle/`; `IMP_AR15` |
| ✅ LOCAL | 🟡 | M4A1 | `Content/assault-rifle-m4a1/`; `IMP_M4A1` |
| ✅ LOCAL | 🟡 | Colt M1911 | `Content/colt-m1911/`; local exact-model path plus existing pistol identity |
| ✅ | 🟡 | Raw Stein M1911 source | `Content/Raw/R13/Weapons/SteinClassicWeapons/WeaponsPack/1911/`; historical source/reimport path |
| ✅ LOCAL | 🟡 | Makarov PM | `Content/makarov-pistol/`; `IMP_MAKAROV` |
| ✅ LOCAL | 🟡 | FN Ballista | `Content/fn-ballista-sniper-rifle/`; `IMP_BALLISTA` |
| ✅ LOCAL | 🟡 | Kar98k | `Content/kar98k-free-model/`; `IMP_KAR98K` |
| ✅ LOCAL | 🟡 | Shotgun / Remington path | `Content/shotgun/`; current Remington/manual-action integration still awaits final visual acceptance |
| ✅ LOCAL | 🟡 | Thompson / Tommy Gun | `Content/tommy-gun/`; `IMP_TOMMY` |
| ✅ LOCAL | 🟡 | M72 LAW | `Content/law-light-anti-tank-weapon-m72/`; `IMP_M72`; projectile launcher gameplay wired |
| ✅ LOCAL | 🟡 | RPG-26 | `Content/rpg-26-grenade-launcher-low-poly/`; `IMP_RPG26`; projectile launcher gameplay wired |
| 🟡 REPORT | 🟡 | New Fab RPG added 2026-09-04 | distinct gameplay identity `IMP_FAB_RPG`; Fab-only exact-model resolver prevents RPG-26 substitution; UE visual/runtime acceptance pending |
| 🟡 REPORT | 🟡 | AKS-74U | distinct gameplay identity `IMP_AKS74U`; local/Fab exact-token visual bridge and sandbox catalog spawn wired; UE visual/runtime acceptance pending |
| 🟡 REPORT | 🟡 | Revolver | distinct gameplay identity `IMP_REVOLVER`; dedicated `Revolver` mechanical action metadata, Fab exact-token visual resolver and sandbox catalog entry wired; UE visual/runtime acceptance pending |
| 🟡 REPORT | ❌ | Additional pistol pack | user-reported Fab import; reconcile against M1911/Makarov/Revolver before creating another gameplay identity |
| 🟡 REPORT | ❌ | FPS Weapon Bundle | user-reported Fab import; exact contained weapon inventory still needs local package enumeration |
| ✅ | 🟡 | M14 | existing gameplay/imported weapon identity; final UE acceptance pending |
| ✅ | 🟡 | MAC-10 | existing gameplay/imported weapon identity; final UE acceptance pending |
| ✅ | 🟡 | TEC-9 | existing gameplay/imported weapon identity; final UE acceptance pending |
| ✅ | 🟡 | Lever Action | existing gameplay/imported weapon identity; final manual-action visual calibration pending |
| ✅ | 🟡 | MP5 / core SMG | existing core gameplay identity; exact visual acceptance pending |
| ✅ | 🟡 | M700 / core sniper | existing core gameplay identity; bolt calibration/acceptance pending |
| ✅ | 🟡 | M249 / core LMG | existing core gameplay identity; exact production model gap remains where applicable |
| ✅ | 🟡 | Core assault rifle | existing core gameplay identity; exact production visual validation pending |
| ✅ | 🟡 | Core pistol | existing core gameplay identity; exact production visual validation pending |
| ✅ | 🟡 | Core shotgun | existing core gameplay identity; exact production visual validation pending |
| ✅ | 🟡 | Core anti-armor launcher | existing `OC_RPG1` identity; remains distinct from M72, RPG-26 and the new Fab RPG |

## Grenades

| Present | Integration | Asset | Evidence / current state |
|---|---|---|---|
| ✅ LOCAL | 🟡 | Frag grenade A | one of the two user-downloaded explosive grenade models; Fab resolver supports distinct frag visuals; exact package name recorded when locally visible |
| ✅ LOCAL | 🟡 | Frag grenade B | second distinct explosive grenade model; must remain visually distinct from Frag A |
| ✅ LOCAL | 🟡 | M18 smoke grenade | user-downloaded Fab smoke grenade; smoke gameplay/VFX path exists; exact model acceptance pending |
| ✅ LOCAL | 🟡 | Flash grenade | local `Content/Fab/Flash_Grenade/` evidence including `flash.uasset` / FlashGrenade assets; exact runtime visual acceptance pending |
| ✅ | ❌ | Legacy R13 grenade body | `/Game/R13/Weapons/grenade`; old shared body only; retire as active shared visual after all four new models pass UE validation |

## Characters, hands and animation

| Present | Integration | Asset / pack | Evidence / current state |
|---|---|---|---|
| ✅ | 🟡 | Quantum Modular Character / QuantumCharacter | `Content/QuantumCharacter/`; arms/modules available; FP/runtime integration not fully accepted |
| ✅ LOCAL | 🟡 | FPSArms3D donor | `SourceAssets/ThirdParty/Gameplay/FPSArms3D/`; first-person arms donor |
| ✅ LOCAL | 🟡 | FPSAssetKit donor | `SourceAssets/ThirdParty/Gameplay/FPSAssetKit/`; donor inventory retained locally |
| ✅ | 🟡 | Sample Animation Pack / Free Animation Pack | `Content/SampleAnimationPack/`; animations present, runtime use only partially accepted |
| 🟡 LOCAL PROJECT | ❌ | Game Animation Sample | separate local UE project; inventory/integration into OsterConflict not complete |
| 🟡 REPORT | ❌ | Additional Fab character packs | user reported several character packs; exact package names must be appended when locally enumerated |

## Buildings and world packs

| Present | Integration | Asset / pack | Evidence / current state |
|---|---|---|---|
| ✅ | 🟡 | Advanced Village Pack | `Content/AdvancedVillagePack/`; world/building content present; selected runtime use only |
| ✅ LOCAL | 🟡 | Five-story post-Soviet apartment building | `Content/fivestory-building-appartament-of-post-soviet/`; building bridge exists; final correct placement pending |
| ✅ | 🟡 | Modular Rural Cabin | `Content/Modular_Rural_Cabin/`; available for Oster housing; final placement pending |
| 🟡 REPORT | ❌ | Modular Urban Houses | user-added pack; exact local root must be reconciled and buildings placed intentionally |
| 🟡 REPORT | ❌ | Unfinished / construction building pack | previously visible/imported building content; exact current package root pending reconciliation |
| ✅ | 🟡 | Open World Demo Collection / KiteDemo | `Content/KiteDemo/`; foliage/trees used, ground/rocks only partially integrated |
| ✅ LOCAL | ❌ | Deko Matrix Demo | `Content/Deko_MatrixDemo/`; imported locally, not a current production world owner |
| 🟡 REPORT | ❌ | City Streets Props / city-street environment content | user-added pack; exact local root/runtime use pending |

## Street props, fortifications and surfaces

| Present | Integration | Asset / pack | Evidence / current state |
|---|---|---|---|
| ✅ | 🟡 | Mega Street Props Pack | `Content/Mega_Street_Props_Pack/`; park bench/fence and selected props wired; full pack not blanket-spawned |
| ✅ | 🟡 | Street Props Pack Vol.1 | `Content/Street_Props_Pack_V1/`; available, selected runtime use pending/partial |
| 🟡 REPORT | 🟡 | Street Props Pack Vol.2 | user-added pack; selected meshes are already referenced through current Street Props content, exact root reconciliation pending |
| ✅ LOCAL | 🟡 | Military Trenches Barrier Sandbag | `Content/Fab/Megascans/3D/Military_Trenches_Barrier_Sandbag_Canvas_Square_01_yd0kbfl/High/`; runtime bridge written, UE validation pending |
| ✅ LOCAL | 🟡 | Military Trenches Pile Sandbag | `Content/Fab/Megascans/3D/Military_Trenches_Pile_Sandbag_Canvas_01_yd0tae2/High/`; runtime bridge written, UE validation pending |
| ✅ LOCAL | 🟡 | PO-2 fence | `Content/po-2-fence/`; available local fence model |
| ✅ LOCAL | 🟡 | Pripyat chain-link fence | `Content/pripyat-chainlink-fence/`; available local fence model |
| ✅ LOCAL | 🟡 | Pripyat light poles | `Content/pripyat-light-poles/`; street-pole runtime bridge exists, UE validation pending |
| ✅ LOCAL | 🟡 | Telephone pole scene | `Content/telephone-pole-scene/`; road-side infrastructure bridge exists, UE validation pending |
| 🟡 REPORT | ❌ | Additional chain-link fence pack | user-reported Fab import; reconcile exact root against Pripyat/PO-2 packs |
| 🟡 REPORT | ❌ | Additional fences pack | user-reported Fab import; exact root/integration pending |
| 🟡 REPORT | 🟡 | Rubble pack | strict `rubble` local resolver now feeds decorative debris into the existing team-base trench setpiece owner; UE visual/runtime acceptance pending |
| ✅ | 🟡 | Sidewalk 01 | tracked `Scene_RoadsideConstruction/.../SM_Urb_Roa_Sidewalk_01`; current world-surface owner replaces source Sidewalks while preserving topology; UE acceptance pending |
| 🟡 REPORT | ❌ | Realistic Asphalt Material PBR | separate user-added surface pack; exact local root/material cutover pending |
| 🟡 REPORT | ❌ | Tileable Pine Forest Road | user-added road/surface pack; exact local root/runtime placement pending |
| ✅ | 🟡 | Roadside Construction | tracked `Content/Scene_RoadsideConstruction/`; `SM_Urb_Roa_Asphalt_01` and `SM_Urb_Roa_Sidewalk_01` are current road/sidewalk runtime assets; UE acceptance pending |
| 🟡 REPORT | ❌ | Free Furniture Pack | user-added furniture pack; exact local root and interior placement pending |

## Vegetation and terrain

| Present | Integration | Asset / pack | Evidence / current state |
|---|---|---|---|
| ✅ | 🟡 | KiteDemo foliage + trees | `Content/KiteDemo/Environments/Foliage` + trees; already used by world logic, integrated acceptance still part of final UE pass |
| ✅ | 🟡 | KiteDemo ground tiles + rocks | present; only partial integration |
| ✅ | 🟡 | PN Foliage Collection | `Content/PN_FoliageCollection/`; integrated fallback/world vegetation path |
| ✅ | 🟡 | Megaplant Library | `Content/Megaplant_Library/`; candidate/partial use |
| ✅ LOCAL | 🟡 | Megaplant Tree Ginkgo | `Content/Megaplant_Library/Tree_Ginkgo/`; local import |
| 🟡 REPORT | ❌ | Temperate Vegetation: Foliage Collection | user-added vegetation pack; exact current local root/integration pending |

## UI and interaction

| Present | Integration | Asset / project | Evidence / current state |
|---|---|---|---|
| ✅ LOCAL | 🟡 | Easy Crosshair / CrosshairFreePack | `Content/CrosshairFreePack/`; HUD bridge written, UE validation pending |
| 🟡 LOCAL PROJECT | ❌ | SuperSimpleFPSPack | separate local project/sample; exact useful HUD/content inventory still pending |
| 🟡 LOCAL PROJECT | ❌ | InteractionSystem / InteractionSystem2 | separate local project; current Oster doors/gates/lights remain authoritative; only useful non-duplicate content should be reused |

## VFX and audio

| Present | Integration | Asset / source | Evidence / current state |
|---|---|---|---|
| ✅ | 🟡 | Fire / Explosion VFX | `Content/Fire_EXP_Vol01_Free/`; wired; included in final UE acceptance |
| ✅ | 🟡 | Smoke VFX | `Content/PotaVFX_Smoke/`; wired; included in final UE acceptance |
| ✅ LOCAL | 🟡 | HardLines shotgun audio | `SourceAssets/ThirdParty/Gameplay/HardLines/`; mechanical/weapon audio donor |
| ✅ LOCAL | 🟡 | HardLines footsteps | same donor root; final routing/audibility pending |
| ✅ LOCAL | 🟡 | BorderRun vehicle engine audio | `SourceAssets/ThirdParty/Gameplay/BorderRun/`; vehicle audio donor |
| 🟡 LOCAL | 🟡 | Forest_Ambience.mp3 | known local ambient source; final SoundWave/runtime acceptance pending |
| 🟡 LOCAL | 🟡 | fire.wav | known local source; final SoundWave/runtime acceptance pending |
| ✅ | 🟡 | AK fire/reload audio | existing weapon audio path; final integrated audibility acceptance pending |
| ✅ LOCAL | ❌ | OpenGameArt donor content | `SourceAssets/ThirdParty/Gameplay/OpenGameArt/`; inventory/use only where provenance and exact purpose are valid |

## Miscellaneous props and donor roots

| Present | Integration | Asset / root | Evidence / current state |
|---|---|---|---|
| ✅ LOCAL | ❌ | Sardine can | `Content/konserva-sardines/`; imported prop, not yet intentionally placed |
| ✅ LOCAL | ❌ | Ukrainian cherry juice | `Content/ukrainian-cherry-juice-nash-sik/`; imported prop, not yet intentionally placed |
| ✅ | 🟡 | R13 content root | `Content/R13/`; existing weapons/world/runtime content, individual owners tracked above |
| ✅ | 🟡 | Raw content root | `Content/Raw/`; source/import staging content, must not become a duplicate runtime owner |
| ✅ LOCAL | 🟡 | BorderRun donor root | `SourceAssets/ThirdParty/Gameplay/BorderRun/`; covered recursively |
| ✅ LOCAL | 🟡 | FPSArms3D donor root | `SourceAssets/ThirdParty/Gameplay/FPSArms3D/`; covered recursively |
| ✅ LOCAL | 🟡 | FPSAssetKit donor root | `SourceAssets/ThirdParty/Gameplay/FPSAssetKit/`; covered recursively |
| ✅ LOCAL | 🟡 | HardLines donor root | `SourceAssets/ThirdParty/Gameplay/HardLines/`; covered recursively |
| ✅ LOCAL | 🟡 | OpenGameArt donor root | `SourceAssets/ThirdParty/Gameplay/OpenGameArt/`; covered recursively |

## Mandatory reconciliation queue

These items are already in the inventory and may not be forgotten merely because Git ignores their payload:

1. Record exact local package/model names for the **new Fab RPG**, AKS-74U, revolver and rubble mesh after UE exposes them, and enumerate the still-unresolved additional pistol, FPS Weapon Bundle and all additional character packs. The RPG, AKS-74U, revolver and rubble runtime bridges are already coded and no longer wait on exact folder names.
2. Resolve the exact local roots for Modular Urban Houses, Street Props Vol.2, City Streets Props, the separate Realistic Asphalt PBR pack, forest road, Free Furniture, Temperate Vegetation and the additional fence packs.
3. Bind the four new grenade models as **Frag A / Frag B / Smoke / Flash**, then retire the legacy shared R13 grenade visual from active use.
4. Finish intentional building/world placement. Importing a house pack into Content is not the same as placing the correct house on the Oster map.
5. Run one integrated UE 5.8 current-head acceptance after the broad asset batch is ready; only then promote runtime/visual/audio rows from `🟡` to `✅`.

## Current truth

- The asset inventory itself is now centralized here.
- All currently known user-added/imported roots and explicitly reported packs/models are represented, including ignored local content that GitHub cannot list directly.
- AKS-74U, the newly added Fab RPG and the revolver now have distinct gameplay IDs/catalog entries and strict local visual resolution. They remain `🟡` until UE 5.8 proves the local models selected and rendered correctly.
- The rubble pack is now an optional strict local asset in the existing trench-setpiece owner, not a second competing world subsystem.
- `Scene_RoadsideConstruction` is verified present in Git and already owns authored road/sidewalk meshes, so those rows are no longer falsely shown as unintegrated.
- This does **not** pretend that every local ignored `.uasset` has been byte-enumerated remotely. Exact local identities that Git cannot see remain visibly marked `LOCAL/REPORT` until factual reconciliation.
- Formal PASS45 progress remains separate from asset-integration progress.
