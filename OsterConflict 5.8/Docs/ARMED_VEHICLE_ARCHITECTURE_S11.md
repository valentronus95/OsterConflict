# Armed Vehicle Architecture — S11

## Класи

`AOCVehicleBase`
: S10 фізика, driver possession, камери, health, wreck.

`AOCArmedVehicleBase`
: двомісний бойовий транспорт. Додає gunner seat, turret state, turret ammo/reload, server fire loop та team crew rules.

`AOCPickupGunTruck`
: легка машина з mounted MG. Звичайний ballistic damage type.

`AOCBTR`
: броньований прототип. Корпус приймає тільки `UOCAntiArmorDamageType`.

`AOCPickupGunTruckSpawnPoint`, `AOCBTRSpawnPoint`
: persistent respawner subclasses з наперед заданим vehicle class.

## Чому gunner не Possess-ить сам vehicle Pawn
Unreal PlayerController одночасно володіє одним Pawn. Vehicle Pawn уже потрібен водієві для W/S/A/D та vehicle camera. Тому gunner залишається на своєму Character, який є owned actor цього клієнта, але переходить у vehicle-gunner state: movement/collision/weapon presentation вимикаються, а mouse/fire inputs маршрутизуються до бойової машини через server RPC на Character.

Це дає дві незалежні input streams:

`Driver PlayerController -> AOCVehicleBase -> movement`

`Gunner PlayerController -> AOCCharacter RPC -> AOCArmedVehicleBase -> turret`

## Server authority
Клієнт gunner не передає результат попадання. Він передає лише відносні yaw/pitch та fire-held/reload intent. Сервер:
1. перевіряє `Requester == GunnerCharacter`;
2. перевіряє наявність Driver;
3. перевіряє стан gunner та vehicle;
4. обмежує yaw/pitch;
5. відраховує ammo;
6. задає RPM;
7. виконує line trace;
8. викликає `ApplyPointDamage`.

## Damage classes
- `UOCBallisticDamageType` — піхотна зброя та mounted MG;
- `UOCVehicleCannonDamageType` — важка vehicle turret зброя;
- `UOCAntiArmorDamageType` — контракт для RPG/одноразового launcher у S12.

S11 BTR intentionally filters hull damage so ballistic, vehicle-cannon та collision damage не можуть знищити корпус. Це gameplay rule згідно з MASTER-TZ, а не симуляція реальної бронепробивності.

## Crew team
`OccupantTeam` задається першим crew member. Поки хоча б один член екіпажу залишається всередині:
- інший seat дозволений тільки гравцю цієї команди;
- server friendly-fire rule блокує damage по своїй occupied armed vehicle;
- коли техніка повністю порожня, team reset у `None` і її може зайняти інша сторона.

## Turret replication
Реплікуються gameplay states, а не mouse input history:
- GunnerCharacter;
- OccupantTeam;
- TurretYaw;
- TurretPitch;
- TurretAmmoInMagazine;
- TurretReserveAmmo;
- bTurretReloading.

Клієнти відтворюють turret transform з replicated yaw/pitch.
