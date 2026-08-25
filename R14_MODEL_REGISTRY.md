# OSTER CONFLICT — R14 GAME ASSET REGISTRY

Оновлено: 2026-08-25
Гілка: `main` / Pass45 recovery current truth

Мета реєстру: одна точка правди для production-візуалів, їхніх `/Game/...` шляхів і стану інтеграції. Тут фіксуються тільки перевірені в репозиторії або коді дані.

## Weapon visuals

| Asset | Canonical path | Mesh | Стан |
|---|---|---|---|
| AK-47 | `/Game/AK-47/Mesh/SKM_AK-47` | Skeletal | runtime code wired; explicit fire/reload presentation exists; Pass45 exact material/texture runtime validation pending |
| MP5 | `/Game/R13/Weapons/Stein/MP5/SKM_MP5` | Skeletal | file/path verified; runtime code wired; Pass45 exact material/texture runtime validation + animation coverage pending |
| M1911 | `/Game/R13/Weapons/Stein/1911/SKM_1911` | Skeletal | runtime code wired; Pass45 exact material/texture runtime validation + animation coverage pending |
| M700 | `/Game/R13/Weapons/Stein/M700/SKM_M700` | Skeletal | runtime code wired; Pass45 exact material/texture runtime validation + animation coverage pending |
| Remington 870 | `/Game/Production/Weapons/Remington870/SM_Remington870` | Static | **CONTENT GAP / NOT READY**: canonical production `.uasset` is absent from current repository content; runtime code points to this path but falls back when load fails; strict Pass45 11/11 weapon gate must remain FAIL until a legitimate authored asset is added and validated |
| M249 | `/Game/Production/Weapons/M249/SM_M249` | Static | **CONTENT GAP / NOT READY**: canonical production `.uasset` is absent from current repository content; runtime code points to this path but falls back when load fails; strict Pass45 11/11 weapon gate must remain FAIL until a legitimate authored asset is added and validated |
| M14 | `/Game/R13/Weapons/Stein/M14/SKM_M14` | Skeletal | runtime code wired; Pass45 exact material/texture runtime validation + animation coverage pending |
| MAC-10 | `/Game/R13/Weapons/Stein/Mac10/SKM_Mac10` | Skeletal | runtime code wired; Pass45 exact material/texture runtime validation + animation coverage pending |
| TEC-9 | `/Game/R13/Weapons/Stein/Tec9/SKM_Tec9` | Skeletal | runtime code wired; Pass45 exact material/texture runtime validation + animation coverage pending |
| Lever Action .45-70 | `/Game/R13/Weapons/Stein/LeverAction/SKM_LeverAction` | Skeletal | runtime code wired; Pass45 exact material/texture runtime validation + animation coverage pending |
| Anti-Armor Launcher (`OC_RPG1`) | `/Game/R13/Weapons/rocketlauncherModern` | Static | Kenney CC0 source + imported uasset verified; runtime production visual wired; Pass45 exact material/texture runtime validation + grip/visual validation pending |
| M2 Browning visual | `/Game/Production/Weapons/M2/SM_M2_Browning` | Static | source/import path verified; mounted by gun-truck runtime code; Pass45 authored material/runtime validation pending |

Pass45 repository audit on 2026-08-25 confirmed that `OsterConflict/Content/Production/Weapons` is absent from the current Git tree and that no repository file supplies the canonical Remington 870 or M249 production mesh under another production path. `AOCWeapon_Shotgun` and `AOCWeapon_LMG` still request the canonical paths above and deliberately keep their gameplay fallback visible when load fails. Therefore those two rows are content blockers, not source-code READY items. No generated grey/default substitute is accepted as closure.

`OCProductionWeaponRuntimeValidationSubsystem` now emits the exact acceptance chain for every implemented weapon: weapon class -> canonical mesh -> material slot -> authored material asset -> runtime material asset -> used texture dependencies. `BasicShapeMaterial`, `DefaultMaterial`, `WorldGridMaterial` and `_defaultMat` are explicit placeholder failures. `VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py` requires `SUMMARY=11/11 production weapon classes PASS`, `materialGaps=0`, `unexpectedOverrides=0` and no `placeholder=1`; this intentionally prevents Pass45 runtime acceptance while either missing production weapon remains unresolved.

Current first-person presentation provides generic ADS, recoil and reload offsets. Explicit model animation sequences are currently wired for AK-47 only. R14 therefore treats all other rows as incomplete until their compatible animation coverage and hand alignment are validated.

R14 has a code-level animation coverage registry in `OCWeaponAnimationProfiles.h/.cpp`. All 11 implemented weapon IDs are declared there. Only `OC_AR1` currently owns verified Fire/Reload object paths. Missing authored animations are represented by intentionally empty paths instead of invented or silent generic mappings. `OC_SG1` and `OC_LMG1` additionally carry an explicit articulated-weapon requirement because the target Remington 870 and M249 production visuals are static meshes when those assets become available.

`OCWeaponPresentationProfileTests.cpp` validates declaration coverage for the same 11 weapon IDs, finite first-person profile transforms, loadability of any declared animation path, canonical AK Fire/Reload paths and AK weapon-skeleton compatibility. `.github/workflows/r14-weapon-profile-contracts.yml` protects the source-level profile matrix in GitHub CI. Actual UE 5.8 compile/runtime/visual validation remains pending and is part of the consolidated Pass45 acceptance rather than something source CI may infer.

Current equipped-weapon attachment historically relied on one shared camera-space base transform for every weapon class (`X=38, Y=12, Z=-14`, zero rotation). R14 routes each implemented weapon ID through its own explicit `FOCFirstPersonWeaponProfile` in `UOCFirstPersonWeaponPresentationSubsystem`. All profiles intentionally preserve the legacy baseline and remain `UNCALIBRATED` until the exact mesh is visually approved in UE 5.8; no fake per-weapon coordinates are being guessed.

`AOCWeaponBase::ApplyInventoryPresentation` still contains the legacy equip-time transform. The profile subsystem corrects presentation afterward; direct equip-path profile wiring remains tracked for a later safe source pass or the consolidated Windows UE compile gate. This is explicit technical debt, not considered completed.

## Characters

### Current faction production state

All four runtime faction archetypes are declared separately, but all still share the same production body and first-person arms. R14 therefore keeps `bFactionUniqueBody=false` and `bFactionUniqueArms=false` until actual differentiated assets/material sets are wired and visually approved.

| Faction | Current body | Current FP arms | Production-distinct |
|---|---|---|---|
| UA Special Unit | `/Game/QuantumCharacter/Mesh/SKM_QuantumCharacter` | `/Game/QuantumCharacter/Mesh/Modules/SKM_Arms` | no |
| Masked Fighters | `/Game/QuantumCharacter/Mesh/SKM_QuantumCharacter` | `/Game/QuantumCharacter/Mesh/Modules/SKM_Arms` | no |
| US Rangers Style | `/Game/QuantumCharacter/Mesh/SKM_QuantumCharacter` | `/Game/QuantumCharacter/Mesh/Modules/SKM_Arms` | no |
| Insurgents | `/Game/QuantumCharacter/Mesh/SKM_QuantumCharacter` | `/Game/QuantumCharacter/Mesh/Modules/SKM_Arms` | no |

### Current gameplay-role visual state

Authoritative gameplay roles already exist in `EOCPlayerRole` and are replicated by `AOCPlayerState`. R14 records their actual current visual mapping without changing gameplay/loadout rules.

| Role | Current GearClass | Current uniqueness |
|---|---|---|
| Rifleman | Standard, seeded Light variant allowed | `bRoleUniqueVisual=false` |
| Medic | Standard | `bRoleUniqueVisual=false` |
| Engineer | Heavy | `bRoleUniqueVisual=false` |
| Support | Heavy | `bRoleUniqueVisual=false` |

Engineer and Support therefore still collide visually as the same Heavy class. Medic can also overlap Standard Rifleman. Stage 2 cannot close until those roles become visually readable without changing their authoritative gameplay behavior.

### Audited QuantumCharacter production candidates

| Asset/module | Canonical path | Type | Runtime now |
|---|---|---|---|
| Main body | `/Game/QuantumCharacter/Mesh/SKM_QuantumCharacter` | Skeletal | yes |
| No-head modular body | `/Game/QuantumCharacter/Mesh/SKM_QuantumCharacter_NoHead` | Skeletal | no, audited candidate |
| First-person arms | `/Game/QuantumCharacter/Mesh/Modules/SKM_Arms` | Skeletal | yes |
| Head | `/Game/QuantumCharacter/Mesh/Modules/SKM_Head` | Skeletal | no, audited candidate |
| Vest | `/Game/QuantumCharacter/Mesh/Modules/SKM_Bulletproof_Bege` | Skeletal | yes |
| Drops/pouches | `/Game/QuantumCharacter/Mesh/Modules/SKM_Drops_1_Bege` | Skeletal | yes |
| Holster | `/Game/QuantumCharacter/Mesh/Modules/SKM_Holster_Hard_Bege` | Skeletal | yes |
| Jeans | `/Game/QuantumCharacter/Mesh/Modules/SKM_Jeans` | Skeletal | no, audited candidate |
| Back patch | `/Game/QuantumCharacter/Mesh/Modules/SKM_Patch_Back` | Skeletal | no, audited candidate |
| Rolled-up blue shirt | `/Game/QuantumCharacter/Mesh/Modules/SKM_Shirt_RolledUp_Blue` | Skeletal | no, audited candidate |
| Beige cap | `/Game/QuantumCharacter/Mesh/Modules/SM_Cap_Bege` | Static | yes |

`OCCharacterProductionProfiles.h/.cpp` is the code-level source of truth for current faction, role and module contracts. `OCCharacterProductionProfileTests.cpp` validates the 4 factions, 4 authoritative roles, shared body/arms state and 10 audited modular candidates. `R14_CHARACTER_MODEL_REQUIREMENTS.md` defines the Stage 2 acceptance criteria.

### Material/texture audit

The existing QuantumCharacter content does **not** currently provide four ready-made faction color/camo sets. Verified examples are singular variants such as:

- `/Game/QuantumCharacter/Materials/M_Bulletproof_Bege`
- `/Game/QuantumCharacter/Materials/M_Drops_Tactical_Bege`
- `/Game/QuantumCharacter/Materials/M_Holster_Hard_Bege`
- `/Game/QuantumCharacter/Materials/M_Cap_Bege`
- `/Game/QuantumCharacter/Materials/M_Shirt_RolledUp_Blue`
- `/Game/QuantumCharacter/Materials/M_Jeasn`
- `/Game/QuantumCharacter/Materials/M_Patches`

The corresponding inspected Bulletproof/Drops texture folders expose beige base-color plus normal/ORM maps, while the rolled-up shirt exposes one blue base-color plus normal/ORM maps. Therefore existing material names alone are insufficient to truthfully mark four factions production-distinct. New licensed material variants, controlled material instances, or additional modular character assets are still required and must be visually approved before runtime promotion.

## Vehicles

| Asset | Canonical path | Стан |
|---|---|---|
| Ukrainian HMMWV | `/Game/Production/Vehicles/HMMWV/SM_HMMWV_UA` | runtime code wired; exact authored material/runtime validation pending |
| BTR-4 Bucephalus | `/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus` | runtime code wired; repository-safe generated fallback now carries explicit `M_BTR4_OC_Authored` PBR material and material-slot binding; local UE 5.8 import/runtime validation pending; user-selected FBX remains development-only until redistribution license is verified |
| Pickup | `/Game/VehicleVarietyPack/Meshes/SM_Pickup` | verified fallback for gun-truck class; dedicated production armed-pickup visual pending |
| Sedan/sports car | `/Game/VehicleVarietyPack/Meshes/SM_SportsCar` | active civilian style |
| Hatchback | `/Game/VehicleVarietyPack/Meshes/SM_Hatchback` | active civilian style |
| SUV | `/Game/VehicleVarietyPack/Meshes/SM_SUV` | active civilian style |
| Box truck | removed from R14 branch | intentionally excluded from R14 vehicle set |

Source/license notes:

- HMMWV source metadata requires attribution.
- M2 source licensing metadata is inconsistent; attribution is preserved until re-verification.
- BTR-4 local user-selected FBX source has no verified redistribution license and remains development-only; the repository-safe generated external game visual is separately authored in-project and still requires local UE visual acceptance.
- Kenney Weapon Pack source includes `LICENSE_KENNEY_CC0.txt`, explicitly Creative Commons Zero (CC0); the imported `rocketlauncherModern` is acceptable for the R14 production launcher visual.

## Environment / interiors

Verified top-level packs currently present for audit/reference include:

- `AdvancedVillagePack` — **runtime-rejected as generic Oster presentation in Pass45; current generic decorator/owner was physically retired**
- `Modular_Rural_Cabin`
- `PN_FoliageCollection`
- `TileableForestRoad`
- `Scene_RoadsideConstruction`
- `Scene_UnfinishedBuilding`
- `Fab`

Environment model acceptance fields to fill during stages 4–5: exact asset path, materials/textures, collision, LOD/Nanite, foliage cull distance, streaming policy, light/exposure compatibility, runtime use and visual verification. Presence in `Content` is not permission to promote a pack into current runtime presentation.

## Animation sources

- `SampleAnimationPack`: rifle idle and ADS idle currently used only as compatible fallback; checked Rifle directory contains no named Fire/Reload assets.
- `AK-47/Animations`: explicit fire/reload sequences currently wired and registered in `OCWeaponAnimationProfiles.cpp`.
- R13 Stein weapon directories: skeletal weapon meshes/accessories are present, but dedicated weapon Fire/Reload animation assets were not found in the checked weapon folders.
- `QuantumCharacter/Demo/Animations`: current body idle/walk/run/fall source used by production character subsystem.
- `OCWeaponAnimationProfiles`: authoritative R14 code matrix for declared per-weapon Fire/Reload coverage; empty path means coverage is still missing.

## Required final fields for every production row

Each asset must end R14 with: source/license, canonical path, mesh type, skeleton if applicable, materials/textures, collision/PhysicsAsset, scale/orientation, sockets, first-/third-person visibility, animation/AnimBP/retarget state, LOD/Nanite, streaming, owning runtime class, compile status, runtime status, visual status and cook/package status.
