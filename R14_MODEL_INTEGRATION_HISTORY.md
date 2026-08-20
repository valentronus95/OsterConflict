# OSTER CONFLICT — R14 MODEL INTEGRATION HISTORY

Цей файл є коротким людським журналом R14. Git commit history залишається технічною історією змін.

## 2026-08-20 — старт R14

- Створено робочу гілку `feat/r14-production-models` від актуального `main` commit `fbe66f7502f0cf6ecc621bba575c1e1b35e7e76b`.
- Перечитано `OSTER_CONFLICT_PROJECT_CONTEXT.md` та застосовано правила: не перебудовувати проєкт з нуля, не ламати multiplayer/gameplay, працювати етапами, перевіряти exact `/Game/...` paths, розділяти source/compile/runtime/visual verification.
- Створено `R14_PRODUCTION_MODELS_TZ.md` у корені гілки.
- Створено `R14_MODEL_REGISTRY.md` як єдину таблицю production asset paths/status.
- Почато Stage 0: інвентаризація моделей і runtime-прив'язок.
- Підтверджено наявні категорії Content: AK-47, R13 weapon assets, QuantumCharacter, SampleAnimationPack, AdvancedVillagePack, Modular_Rural_Cabin, PN_FoliageCollection, TileableForestRoad, VehicleVarietyPack та інші scene/Fab assets.
- Підтверджено, що `AOCPickupGunTruck` уже має runtime-підключення production HMMWV та окремого M2 visual на turret/barrel pivot. Якщо HMMWV asset недоступний, клас використовує pickup fallback.
- Підтверджено production source pipeline для HMMWV, M2 та BTR-4. Mk19 subtree прибирається під час HMMWV import, а M2 імпортується окремо.
- Підтверджено: BTR-4 source поки development-only через непідтверджену redistribution license.
- Підтверджено: current first-person weapon presentation має generic ADS/recoil/reload offsets, але explicit fire/reload sequences зараз під'єднані лише для AK-47. Це перший великий борг Stage 1.
- Підтверджено: current character production subsystem використовує одну QuantumCharacter body/arms базу для кількох faction profiles; R14 має розвести команди/класи візуально.
- Підтверджено VehicleVarietyPack assets: hatchback, pickup, SUV, sports car та box truck. Активний civilian vehicle code використовує sports car/hatchback/SUV, не box truck.

## 2026-08-20 — перший cleanup/validation pass

- Видалено `OsterConflict/Content/VehicleVarietyPack/Meshes/SM_Truck_Box.uasset` з R14. Інші потрібні vehicle assets пакета не зачіпалися.
- Розширено `OsterConflict.ProductionModels.CanonicalAssets`: automation тепер перевіряє не лише HMMWV/M2/BTR-4, а також armed pickup visual, Remington 870, M249, AK-47, MP5, M1911, M700, M14, MAC-10, TEC-9, Lever Action, QuantumCharacter body та first-person arms.
- Для skeletal meshes тест додатково перевіряє usable bounds, skeleton, material slots і render LOD.
- `INGEST_UPLOADED_MODELS_AND_IMPORT.cmd` переведено зі старої `feat/import-hmmwv-btr4-m2` на активну `feat/r14-production-models`, щоб локальний ingest не перемикав роботу назад у застарілу гілку.
- Створено Draft PR #15 `R14 production models integration` як довготривалий review/CI контейнер. Merge заборонений до завершення R14 verification gates.
- Перший PR contract run #44 впав, бо workflow ще очікував стару назву branch у ingest contract. Workflow виправлено, а не обійдено.
- Production model workflow тепер підтримує R14 push/PR paths та перевіряє R14 branch lock і розширений skeletal model contract.
- Повторний run #46 (`32397545168`) завершився SUCCESS по всіх contract steps.

## 2026-08-20 — animation inventory для Stage 1

- `SampleAnimationPack/Animations` містить `Rifle`, `Unarmed`, `Door` набори.
- Перевірений `Rifle` набір має rifle idle/ADS/walk animation assets, але в поточному каталозі не знайдено named `Fire` або `Reload` assets.
- Тому `SampleAnimationPack` не можна чесно вважати повним fire/reload рішенням для всіх weapon models.
- AK-47 залишається єдиною зброєю з explicit model fire/reload sequences, уже підключеними в `UOCFirstPersonWeaponPresentationSubsystem`.
- Для MP5/M1911/M700/M14/MAC-10/TEC-9/Lever Action та static Remington/M249 потрібен окремий compatible animation/retarget pass; для static meshes Remington/M249 також потрібно вирішити, чи замінювати їх на skeletal production meshes для рухомих деталей.
- Створено `R14_WEAPON_ANIMATION_REQUIREMENTS.md` з weapon-by-weapon матрицею потрібних fire/reload/bolt/pump/lever/belt/arms animation assets та acceptance gate для нового animation pack.

## 2026-08-20 — R14 weapon runtime gate

- Додано `UOCProductionWeaponRuntimeValidationSubsystem`. Він не працює в нормальному gameplay і активується лише параметром `-ValidateProductionWeapons`.
- Validator створює transient тестові weapon actors далеко під playable world, перевіряє weapon id, canonical mesh path, tagged production component і відсутність видимого source-only fallback, записує `weapon_runtime_validation.txt`, після чого видаляє тестові actors.
- Додано headless режим `-ValidateProductionWeaponsHeadless`: після запису report/sentinel UE процес завершується. `VALIDATE_PRODUCTION_MODELS_UE58.cmd` тепер блокує подальший visual launch, якщо runtime weapon sentinel не створений.
- One-click validation тепер має 4 етапи: Editor build → canonical automation → headless weapon runtime gate → visual Sandbox.
- Додано окремий workflow `.github/workflows/r14-weapon-model-contracts.yml` для weapon-specific source contracts.

## 2026-08-20 — first-person grip architecture

- Знайдено системний дефект: усі weapon actors історично отримували один camera-space transform `X=38, Y=12, Z=-14`, zero rotation, незалежно від геометрії зброї.
- Додано `FOCFirstPersonWeaponProfile` та explicit profile matrix для всіх реалізованих weapon IDs: `OC_AR1`, `OC_SMG1`, `OC_PST1`, `OC_SNP1`, `OC_SG1`, `OC_LMG1`, `R13_M14`, `R13_MAC10`, `R13_TEC9`, `R13_LEVER4570`, `OC_RPG1`.
- `UOCFirstPersonWeaponPresentationSubsystem` тепер бере base grip, ADS, recoil і reload transforms із weapon profile.
- Профілі навмисно стартують зі старого baseline і `bGripCalibrated=false`. Координати конкретної зброї не вигадуються без фактичного UE 5.8 visual check.
- Runtime log явно попереджає `UNCALIBRATED` для weapon profile, доки exact mesh не пройшов visual approval.

## 2026-08-20 — anti-armor launcher production visual

- Stage 0 виявив уже наявний raw asset `Content/Raw/R13/Weapons/Kenney/rocketlauncherModern.obj` та вже імпортований `/Game/R13/Weapons/rocketlauncherModern`.
- Перевірено `LICENSE_KENNEY_CC0.txt`: Kenney Weapon Pack прямо ліцензований Creative Commons Zero (CC0), дозволене personal/educational/commercial use.
- `AOCAntiArmorLauncher` (`OC_RPG1`) отримав production visual `/Game/R13/Weapons/rocketlauncherModern.rocketlauncherModern` без зміни projectile/damage/network gameplay logic.
- Старі source-only static proxy components ховаються тільки після успішного створення та реєстрації production visual, щоб failure path не робив launcher невидимим.
- Launcher додано до canonical automation та runtime weapon validation. Попередній `OC_RPG1 = MISSING` gate прибрано.

## CI після Stage 1 infrastructure pass

- Production model integration contracts: SUCCESS.
- R14 weapon model contracts: SUCCESS після виправлення реєстру та launcher integration contracts.
- CI тут перевіряє source/contracts. Фактичний UE 5.8 Editor compile, headless runtime gate, grip visual validation і cook/package не позначаються PASS без Windows UE запуску.

## Поточна точка роботи

`Stage 0 — inventory/contracts`: основний weapon/vehicle/content inventory сформований; environment/character detailed inventory ще продовжується.

`Stage 1 — weapons`: canonical production visuals тепер оголошені для всіх 11 реалізованих weapon IDs; runtime/headless validation infrastructure та per-weapon grip architecture готові. Наступний підетап: UE 5.8 compile/runtime/visual calibration + підбір/імпорт сумісних fire/reload animation assets для всіх weapon rows, де вони позначені `MISSING`.
