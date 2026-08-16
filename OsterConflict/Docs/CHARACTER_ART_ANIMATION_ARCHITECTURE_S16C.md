# Character Art / Animation Architecture — S16C

## 1. Фракції

| ID | Напрямок | Базова палітра | Принцип |
|---|---|---|---|
| UA Special Unit | сучасний український tactical look | olive / multicam | без копіювання конкретної чинної форми або емблем 1:1 |
| Masked Fighters | темний нерегулярний tactical/civilian mix | black / charcoal / muted | балаклави, chest rigs, легше спорядження |
| US Rangers Style | contemporary US-style infantry reference | OCP / coyote | узагальнений ranger-style art direction |
| Insurgents | цивільно-польовий mix | brown / gray / olive | різні куртки, робочий/польовий одяг, нерегулярне спорядження |

Одночасно в матчі грають 2 сторони. Default: TeamOne = UA Special Unit, TeamTwo = Masked Fighters.

## 2. Мережеве правило

Gameplay не залежить від mesh. Сервер авторитетно призначає:

`Team -> FactionArchetype -> AppearanceSeed`

`PlayerState` реплікує faction/seed. Клієнт будує presentation з того самого seed. Це дозволяє людям та AI використовувати одну систему, а reconnect/join-in-progress отримує той самий canonical state.

## 3. DataAsset profile

`UOCCharacterVisualProfile` містить посилання на:

- ThirdPersonBodyMesh
- FirstPersonArmsMesh
- ThirdPersonAnimClass
- FirstPersonAnimClass
- helmet / vest / backpack mesh banks
- Fire / Reload / Revive / Downed / Death montages
- sockets

Production art можна замінювати без зміни network/gameplay C++.

## 4. Animation Blueprint contract

`UOCCharacterAnimInstance` віддає:

- Speed2D / VerticalSpeed / DirectionDegrees
- InAir / Crouched / Sprinting
- Aiming / Reloading / HasWeapon
- Downed / Dead / Reviving
- InVehicle / VehicleGunner
- AimPitch / AimYaw

Рекомендований final graph:

`Locomotion State Machine -> UpperBody weapon layer -> AimOffset -> Additive hit reaction -> IK -> ragdoll transition`

Locomotion states:

`Idle / Walk / Run / Sprint / Crouch / Jump-Fall-Land / Downed Crawl / Vehicle Hidden-Seated / Dead`

One-shot дії Fire / Reload / Revive мають йти montage slots поверх locomotion, де це дозволяє стан.

## 5. First-person / third-person split

- Third-person body: OwnerNoSee.
- FPS arms: OnlyOwnerSee, no shadow by default.
- Weapon presentation лишається окремим actor framework.
- У production pass зброя повинна мати окремі FP/TP sockets і pose offsets.

## 6. IK / retargeting

Production вимога:

- один preferred humanoid skeleton family;
- IK Rig для source і target;
- IK Retargeter для сторонніх ліцензованих animation libraries;
- hand IK для стабільного хвату зброї;
- foot IK / ground alignment для сходів, ґрунту й нерівних дворів;
- окремі stance offsets для crouch/downed.

## 7. Source-only proxy

До появи assets компонент створює простий proxy silhouette з Engine BasicShapes. Це не art target. Він потрібен для:

- видимості ботів і remote players;
- перевірки faction replication;
- перевірки camera/owner visibility;
- тесту vehicle / hit / downed / corpse systems.

## 8. Performance budget

- одна базова skeleton family бажана для більшості playable characters;
- modular gear не повинен створювати десятки окремих replicated actors;
- LOD/skin cache/Nanite rules визначаються після імпорту реальних meshes;
- remote animation tick rate та significance optimization профілюються в S18.
