# OSTER CONFLICT — R14 MODEL INTEGRATION HISTORY

Цей файл є коротким людським журналом R14. Git commit history залишається технічною історією змін.

## 2026-08-20 — старт R14

- Створено робочу гілку `feat/r14-production-models` від актуального `main` commit `fbe66f7502f0cf6ecc621bba575c1e1b35e7e76b`.
- Перечитано `OSTER_CONFLICT_PROJECT_CONTEXT.md` та застосовано правила: не перебудовувати проєкт з нуля, не ламати multiplayer/gameplay, працювати етапами, перевіряти exact `/Game/...` paths, розділяти source/compile/runtime/visual verification.
- Створено `R14_PRODUCTION_MODELS_TZ.md` у корені гілки.
- Почато Stage 0: інвентаризація моделей і runtime-прив'язок.
- Підтверджено наявні категорії Content: AK-47, R13 weapon assets, QuantumCharacter, SampleAnimationPack, AdvancedVillagePack, Modular_Rural_Cabin, PN_FoliageCollection, TileableForestRoad, VehicleVarietyPack та інші scene/Fab assets.
- Підтверджено, що `AOCPickupGunTruck` уже має runtime-підключення production HMMWV та окремого M2 visual на turret/barrel pivot. Якщо HMMWV asset недоступний, клас використовує pickup fallback.
- Підтверджено production source pipeline для HMMWV, M2 та BTR-4. Mk19 subtree прибирається під час HMMWV import, а M2 імпортується окремо.
- Підтверджено: BTR-4 source поки development-only через непідтверджену redistribution license.
- Підтверджено: current first-person weapon presentation має generic ADS/recoil/reload offsets, але explicit fire/reload sequences зараз під'єднані лише для AK-47. Це перший великий борг Stage 1.
- Підтверджено: current character production subsystem використовує одну QuantumCharacter body/arms базу для кількох faction profiles; R14 має розвести команди/класи візуально.
- Підтверджено VehicleVarietyPack assets: hatchback, pickup, SUV, sports car та box truck. Активний civilian vehicle code використовує sports car/hatchback/SUV, не box truck.

## Поточна точка роботи

`Stage 0 — inventory/contracts`: IN PROGRESS.

Наступний технічний блок: повний weapon asset matrix, grip/FP/TP animation coverage, потім runtime exclusion/cleanup непридатних vehicle models.
