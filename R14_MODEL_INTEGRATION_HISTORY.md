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

## Поточна точка роботи

`Stage 0 — inventory/contracts`: IN PROGRESS.

`Stage 1 — weapons`: STARTED. Canonical mesh validation готовий; наступний підетап — weapon-by-weapon grip + compatible fire/reload animation coverage, починаючи з уже skeletal моделей.

Примітка: UE 5.8 compile/runtime/visual/cook ще не позначені PASS. GitHub CI тут перевіряє contracts/source structure, а фактичний Unreal validation виконується окремо через Windows UE 5.8 pipeline.
