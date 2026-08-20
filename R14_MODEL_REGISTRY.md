# OSTER CONFLICT — R14 GAME ASSET REGISTRY

Оновлено: 2026-08-20
Гілка: `feat/r14-production-models`

Мета реєстру: одна точка правди для production-візуалів, їхніх `/Game/...` шляхів і стану інтеграції. Тут фіксуються тільки перевірені в репозиторії або коді дані.

## Weapon visuals

| Asset | Canonical path | Mesh | Стан |
|---|---|---|---|
| AK-47 | `/Game/AK-47/Mesh/SKM_AK-47` | Skeletal | runtime code wired; explicit fire/reload presentation exists; visual validation pending |
| MP5 | `/Game/R13/Weapons/Stein/MP5/SKM_MP5` | Skeletal | file/path verified; runtime code wired; animation coverage pending |
| M1911 | `/Game/R13/Weapons/Stein/1911/SKM_1911` | Skeletal | runtime code wired; animation coverage pending |
| M700 | `/Game/R13/Weapons/Stein/M700/SKM_M700` | Skeletal | runtime code wired; animation coverage pending |
| Remington 870 | `/Game/Production/Weapons/Remington870/SM_Remington870` | Static | runtime code wired; animated production replacement/pass pending |
| M249 | `/Game/Production/Weapons/M249/SM_M249` | Static | runtime code wired; animated production replacement/pass pending |
| M14 | `/Game/R13/Weapons/Stein/M14/SKM_M14` | Skeletal | runtime code wired; animation coverage pending |
| MAC-10 | `/Game/R13/Weapons/Stein/Mac10/SKM_Mac10` | Skeletal | runtime code wired; animation coverage pending |
| TEC-9 | `/Game/R13/Weapons/Stein/Tec9/SKM_Tec9` | Skeletal | runtime code wired; animation coverage pending |
| Lever Action .45-70 | `/Game/R13/Weapons/Stein/LeverAction/SKM_LeverAction` | Skeletal | runtime code wired; animation coverage pending |
| Anti-Armor Launcher (`OC_RPG1`) | `/Game/R13/Weapons/rocketlauncherModern` | Static | Kenney CC0 source + imported uasset verified; runtime production visual wired; canonical/runtime automation added; grip/visual validation pending |
| M2 Browning visual | `/Game/Production/Weapons/M2/SM_M2_Browning` | Static | source/import path verified; mounted by gun-truck runtime code; visual validation pending |

Current first-person presentation provides generic ADS, recoil and reload offsets. Explicit model animation sequences are currently wired for AK-47 only. R14 therefore treats all other rows as incomplete until their compatible animation coverage and hand alignment are validated.

Current equipped-weapon attachment historically relied on one shared camera-space base transform for every weapon class (`X=38, Y=12, Z=-14`, zero rotation). R14 now routes each implemented weapon ID through its own explicit `FOCFirstPersonWeaponProfile` in `UOCFirstPersonWeaponPresentationSubsystem`. All profiles intentionally preserve the legacy baseline and remain `UNCALIBRATED` until the exact mesh is visually approved in UE 5.8; no fake per-weapon coordinates are being guessed.

`AOCWeaponBase::ApplyInventoryPresentation` still contains the legacy equip-time transform. The profile subsystem corrects presentation afterward; direct equip-path profile wiring is tracked as a follow-up after the first Windows UE compile gate to avoid broad base-weapon rewrites without compilation.

## Characters

| Asset/profile | Path/status | R14 requirement |
|---|---|---|
| QuantumCharacter body | `/Game/QuantumCharacter/Mesh/SKM_QuantumCharacter` | technical production base; faction/class differentiation pending |
| First-person arms | `/Game/QuantumCharacter/Mesh/Modules/SKM_Arms` | per-weapon grip/IK/animation validation |
| Vest | `/Game/QuantumCharacter/Mesh/Modules/SKM_Bulletproof_Bege` | team/class gear variation |
| Drops/pouches | `/Game/QuantumCharacter/Mesh/Modules/SKM_Drops_1_Bege` | team/class gear variation |
| Holster | `/Game/QuantumCharacter/Mesh/Modules/SKM_Holster_Hard_Bege` | socket/visibility validation |
| Cap | `/Game/QuantumCharacter/Mesh/Modules/SM_Cap_Bege` | headgear set expansion |
| UA Special Unit | runtime profile exists | unique team/class visual set pending |
| Masked Fighters | runtime profile exists | unique team/class visual set pending |
| US Rangers Style | runtime profile exists | unique team/class visual set pending |
| Insurgents | runtime profile exists | unique team/class visual set pending |

## Vehicles

| Asset | Canonical path | Стан |
|---|---|---|
| Ukrainian HMMWV | `/Game/Production/Vehicles/HMMWV/SM_HMMWV_UA` | runtime code wired; visual/runtime verification pending |
| BTR-4 Bucephalus | `/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus` | runtime code wired; development-only until source redistribution license is verified |
| Pickup | `/Game/VehicleVarietyPack/Meshes/SM_Pickup` | verified fallback for gun-truck class; dedicated production armed-pickup visual pending |
| Sedan/sports car | `/Game/VehicleVarietyPack/Meshes/SM_SportsCar` | active civilian style |
| Hatchback | `/Game/VehicleVarietyPack/Meshes/SM_Hatchback` | active civilian style |
| SUV | `/Game/VehicleVarietyPack/Meshes/SM_SUV` | active civilian style |
| Box truck | removed from R14 branch | intentionally excluded from R14 vehicle set |

Source/license notes:

- HMMWV source metadata requires attribution.
- M2 source licensing metadata is inconsistent; attribution is preserved until re-verification.
- BTR-4 source has no verified redistribution license and remains development-only.
- Kenney Weapon Pack source includes `LICENSE_KENNEY_CC0.txt`, explicitly Creative Commons Zero (CC0); the imported `rocketlauncherModern` is acceptable for the R14 production launcher visual.

## Environment / interiors

Verified top-level packs currently present and scheduled for R14 audit:

- `AdvancedVillagePack`
- `Modular_Rural_Cabin`
- `PN_FoliageCollection`
- `TileableForestRoad`
- `Scene_RoadsideConstruction`
- `Scene_UnfinishedBuilding`
- `Fab`

Environment model acceptance fields to fill during stages 4–5: exact asset path, materials/textures, collision, LOD/Nanite, foliage cull distance, streaming policy, light/exposure compatibility, runtime use and visual verification.

## Animation sources

- `SampleAnimationPack`: rifle idle and ADS idle currently used only as compatible fallback; checked Rifle directory contains no named Fire/Reload assets.
- `AK-47/Animations`: explicit fire/reload sequences currently wired.
- R13 Stein weapon directories: skeletal weapon meshes/accessories are present, but dedicated weapon Fire/Reload animation assets were not found in the checked weapon folders.
- `QuantumCharacter/Demo/Animations`: current body idle/walk/run/fall source used by production character subsystem.

## Required final fields for every production row

Each asset must end R14 with: source/license, canonical path, mesh type, skeleton if applicable, materials/textures, collision/PhysicsAsset, scale/orientation, sockets, first-/third-person visibility, animation/AnimBP/retarget state, LOD/Nanite, streaming, owning runtime class, compile status, runtime status, visual status and cook/package status.
