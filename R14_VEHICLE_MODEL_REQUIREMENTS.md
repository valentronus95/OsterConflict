# OSTER CONFLICT — R14 VEHICLE MODEL REQUIREMENTS

Оновлено: 2026-08-20
Гілка: `feat/r14-production-models`

Цей документ фіксує Stage 3: HMMWV + M2, armed pickup + M2, BTR-4 та активний civilian vehicle set. Основне правило: різні машини не можуть проходити production validation шляхом тихої підміни однієї моделі іншою.

## 1. Combat vehicle identities

### HMMWV + M2

- Runtime class: `AOCHMMWVGunTruck`.
- Shared authoritative gameplay/network base: `AOCPickupGunTruck` / `AOCArmedVehicleBase`.
- Canonical shell: `/Game/Production/Vehicles/HMMWV/SM_HMMWV_UA`.
- Canonical mounted weapon: `/Game/Production/Weapons/M2/SM_M2_Browning`.
- HMMWV subclass explicitly selects HMMWV visual through `ShouldUseHMMWVProductionVisual() == true`.
- Normal combat-fleet count/distribution is preserved through the existing legacy spawn-point reference, which now resolves to the explicit HMMWV class.

### Armed pickup + M2

- Runtime class: `AOCPickupGunTruck`.
- Canonical current shell: `/Game/VehicleVarietyPack/Meshes/SM_Pickup`.
- Canonical mounted weapon: `/Game/Production/Weapons/M2/SM_M2_Browning`.
- Base class explicitly keeps `ShouldUseHMMWVProductionVisual() == false`.
- Explicit spawn type: `AOCProductionPickupGunTruckSpawnPoint`.
- It is **not** added as an extra normal-match vehicle yet, so Stage 3 model work does not silently alter combat vehicle count/balance before UE visual validation.

### BTR-4 Bucephalus

- Runtime class: `AOCBTR`.
- Canonical shell: `/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus`.
- Existing physics/suspension/armor/weapon logic remains authoritative; production asset is a visual shell.
- Development-only until redistribution/source license is verified.

## 2. Current spawn compatibility

`AOCGameMode::SpawnCombatVehicleFleet()` currently creates one legacy gun-truck spawn and one BTR-class spawn per side. R14 does not increase that count during the source-only model pass.

The old class name `AOCPickupGunTruckSpawnPoint` is temporarily retained because existing map/GameMode code already references it. Its `VehicleClass` now resolves to `AOCHMMWVGunTruck`, preserving current HMMWV fleet behavior while the new explicit spawn classes are introduced.

After the consolidated UE visual pass, the legacy spawn-point name can be migrated in a controlled follow-up without mixing that rename into model integration.

## 3. Runtime validation contract

`UOCProductionVehicleRuntimeValidationSubsystem` now treats vehicle identities separately:

- `AOCHMMWVGunTruck` must visibly use the HMMWV canonical mesh;
- plain `AOCPickupGunTruck` must visibly use `SM_Pickup`;
- every armed gun truck must visibly use the tagged production M2;
- `AOCBTR` must visibly use the BTR-4 canonical mesh;
- HMMWV, pickup, M2 and BTR-4 assets must have usable bounds/materials.

A HMMWV may no longer satisfy the pickup validation row and a pickup may no longer satisfy the HMMWV row.

## 4. Source/license state

| Asset | Source state | R14 release state |
|---|---|---|
| Ukrainian HMMWV | source metadata identifies Sketchfab `42manako`, CC-BY-4.0 | usable with attribution preserved |
| M2 Browning | source metadata identifies `britdawgmasterfunk`; title says CC0 but downloaded GLB metadata was observed as CC-BY-4.0 | keep attribution; license must be re-verified before final release |
| BTR-4 | no license file; FBX source path references GTA San Andreas BTR-4E mod | development-only until redistribution license/source verified |
| VehicleVarietyPack pickup | existing project Content asset | source/package license must stay documented with the project asset provenance before release |

No R14 code is allowed to convert an unresolved license into a “PASS” label merely because the mesh loads.

## 5. Model acceptance per combat vehicle

Each vehicle must pass:

1. exact canonical mesh loads;
2. correct scale and forward axis;
3. wheel/ground alignment;
4. no shell intersection with physical collision body;
5. driver camera not inside opaque geometry;
6. third-person camera clearance;
7. M2 mount pivot visually sits on roof/bed mount;
8. turret yaw + barrel pitch move visible M2 and authoritative muzzle together;
9. muzzle trace starts at visible barrel end;
10. driver/gunner enter/exit remains network-authoritative;
11. source-only placeholder geometry is hidden only after production shell succeeds;
12. vehicle damage/wreck/respawn logic is unchanged;
13. dedicated server remains independent from cosmetic mesh availability;
14. cook/package includes required assets;
15. source/license/attribution status recorded.

## 6. Civilian vehicles

Active runtime civilian set remains:

- `/Game/VehicleVarietyPack/Meshes/SM_SportsCar`;
- `/Game/VehicleVarietyPack/Meshes/SM_Hatchback`;
- `/Game/VehicleVarietyPack/Meshes/SM_SUV`.

The box truck was removed from the R14 branch because it is not part of the intended vehicle set. R14 must not reintroduce miscellaneous truck models simply to increase visual variety.

## 7. Stage 3 Definition of Done

Stage 3 closes only when:

- HMMWV and armed pickup are separate runtime identities and validate against different shells;
- HMMWV + M2 mount alignment is visually approved;
- armed pickup + M2 mount alignment is visually approved;
- BTR-4 shell is visually/physically approved;
- BTR-4 redistribution rights are resolved or the asset is replaced before distributable release;
- M2 license discrepancy is resolved or attribution/license handling is explicitly approved;
- vehicle collision, cameras, turret/muzzle, enter/exit, damage, respawn and multiplayer behavior pass;
- Windows UE 5.8 compile/runtime/visual/cook PASS in the consolidated laptop run;
- registry/history reflect the actual final state.
