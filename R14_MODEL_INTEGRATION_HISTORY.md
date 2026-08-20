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
- Перевірено equip path у `AOCWeaponBase`: `ApplyInventoryPresentation` досі містить legacy `FVector(38,12,-14)`. Presentation subsystem уже замінює його profile-значенням, але перенесення profile безпосередньо в equip path залишено окремим follow-up, щоб не переписувати великий базовий weapon-файл без фактичної компіляції.

## 2026-08-20 — anti-armor launcher production visual

- Stage 0 виявив уже наявний raw asset `Content/Raw/R13/Weapons/Kenney/rocketlauncherModern.obj` та вже імпортований `/Game/R13/Weapons/rocketlauncherModern`.
- Перевірено `LICENSE_KENNEY_CC0.txt`: Kenney Weapon Pack прямо ліцензований Creative Commons Zero (CC0), дозволене personal/educational/commercial use.
- `AOCAntiArmorLauncher` (`OC_RPG1`) отримав production visual `/Game/R13/Weapons/rocketlauncherModern.rocketlauncherModern` без зміни projectile/damage/network gameplay logic.
- Старі source-only static proxy components ховаються тільки після успішного створення та реєстрації production visual, щоб failure path не робив launcher невидимим.
- Launcher додано до canonical automation та runtime weapon validation. Попередній `OC_RPG1 = MISSING` gate прибрано.

## 2026-08-20 — code-level animation coverage matrix

- Додано `OCWeaponAnimationProfiles.h/.cpp` як одну кодову матрицю authored Fire/Reload coverage для всіх 11 реалізованих weapon ID.
- `OC_AR1` містить перевірені canonical object paths `AK-47_Fire_W` і `AK-47_Reload_W`.
- Інші weapon rows не отримали вигаданих generic paths: відсутні authored animations зберігаються як явні порожні paths і залишаються `MISSING` у R14 requirements.
- Для `OC_SG1` (Remington 870) та `OC_LMG1` (M249) кодова матриця окремо фіксує `bRequiresArticulatedWeapon=true`, бо поточні production visuals static і не можуть чесно відтворювати pump/belt/magazine mechanics як цілісний mesh.
- Додано `OCWeaponPresentationProfileTests.cpp`: UE automation перевіряє 11/11 grip/animation profile declarations, відсутність NaN у base transforms, loadability усіх оголошених animation paths, canonical AK Fire/Reload paths і skeleton compatibility з production AK mesh.
- Додано `.github/workflows/r14-weapon-profile-contracts.yml`, щоб source-CI не дозволяв тихо втратити weapon row, AK path або articulated requirement.
- `R14_WEAPON_ANIMATION_REQUIREMENTS.md` і `R14_MODEL_REGISTRY.md` синхронізовано з новою code-level матрицею.
- На прохання користувача локальний Windows UE 5.8 прогін не запускається після кожної паралельної гілки/локації. Compile/runtime/visual/cook перевірку перенесено в один консолідований ноутбучний validation pass після подальшого доопрацювання.

## 2026-08-20 — Stage 2 character production audit

- Підтверджено чотири runtime faction archetypes: `UASpecialUnit`, `MaskedFighters`, `USRangers`, `Insurgents`.
- Підтверджено, що всі 4 faction-профілі зараз використовують одну production body `/Game/QuantumCharacter/Mesh/SKM_QuantumCharacter` і ті самі FP arms `/Game/QuantumCharacter/Mesh/Modules/SKM_Arms`. Тому вони не позначаються production-distinct лише через різні назви.
- Підтверджено authoritative gameplay roles у `EOCPlayerRole`: Rifleman, Medic, Engineer, Support.
- Підтверджено фактичну role→GearClass логіку: Engineer/Support = Heavy, Medic = Standard, Rifleman = seeded 35% Light / 65% Standard.
- Додано `OCCharacterProductionProfiles.h/.cpp` як code-level registry faction, role та modular production contracts. Усі faction unique flags і role unique flags за замовчуванням `false` до реальної visual approval.
- Аудит `QuantumCharacter/Mesh` та `Mesh/Modules` виявив 10 корисних modular candidates: no-head body, arms, head, beige vest, drops, holster, jeans, back patch, rolled-up blue shirt, beige cap.
- `SKM_QuantumCharacter_NoHead` зареєстровано тільки як audited candidate, не як runtime-active replacement.
- Додано `OCCharacterProductionProfileTests.cpp`: UE automation перевіряє 4 factions, 4 authoritative roles, shared body/arms truth, role gear mapping і 10 audited modules з skeleton/material/LOD validation.
- Додано `.github/workflows/r14-character-model-contracts.yml` та компактний додатковий `.github/workflows/r14-character-audit-extension.yml` для source-level character contracts.
- Створено `R14_CHARACTER_MODEL_REQUIREMENTS.md` з окремим Definition of Done для faction/role differentiation.

## 2026-08-20 — character material/texture audit

- Перевірено `QuantumCharacter/Materials` і ключові texture folders.
- Наявні матеріали включають `M_Bulletproof_Bege`, `M_Drops_Tactical_Bege`, `M_Holster_Hard_Bege`, `M_Cap_Bege`, `M_Shirt_RolledUp_Blue`, `M_Jeasn`, `M_Patches` та базові body/head/arms materials.
- Bulletproof і Drops мають по одному beige base-color набору плюс normal/ORM; rolled-up shirt має один blue base-color плюс normal/ORM.
- Отже, у поточному Content немає чесних готових material/camo variants для 4 production-distinct factions. Це не маскується через випадкові tint-и або назви профілів.
- Для Stage 2 потрібні або нові ліцензовані material variants, або контрольовані material instances, або додаткові modular character assets. Конкретні faction colors/material assignments не затверджуються без UE visual pass.
- Під час реєстрового оновлення короткочасно було змінено регістр exact path для MAC-10/TEC-9; до подальшої роботи одразу відновлено канонічні `/SKM_Mac10` і `/SKM_Tec9` відповідно до runtime source.

## 2026-08-20 — ТЗ синхронізовано з реалізацією

- `R14_PRODUCTION_MODELS_TZ.md` оновлено фактичними Stage 0/Stage 1 результатами.
- У Definition of Done додано окрему вимогу: жоден weapon grip profile не може залишатися `UNCALIBRATED` перед merge.
- ТЗ прямо посилається на `R14_WEAPON_ANIMATION_REQUIREMENTS.md` і blocking headless weapon runtime gate.
- Stage 2 має окремий `R14_CHARACTER_MODEL_REQUIREMENTS.md`; загальний реєстр тепер містить фактичний faction/role/module/material status.

## CI / validation policy

- Stage 1 source gates були зелені після animation/profile infrastructure pass.
- Stage 2 додав character-specific source gates; їхній актуальний стан перевіряється на новому HEAD після синхронізації history/docs.
- Source CI не дорівнює UE runtime validation.
- На прохання користувача фактичний Windows UE 5.8 Editor compile, headless runtime, visual calibration і cook/package не запускаються після кожної паралельної локації/гілки. Вони виконуються одним консолідованим ноутбучним validation pass пізніше.

## Поточна точка роботи

`Stage 0 — inventory/contracts`: weapon/vehicle/content inventory сформований значною мірою; character inventory тепер деталізований до faction/role/modules/materials; environment/interior audit ще попереду.

`Stage 1 — weapons`: 11 canonical production visuals, runtime/headless validation infrastructure, per-weapon grip architecture та code-level animation coverage matrix готові на source-рівні. Відсутні authored animations і UE visual calibration залишаються відкритими.

`Stage 2 — characters`: source architecture/inventory/automation стартували й зафіксували реальний стан без фальшивої готовності. Чотири faction-профілі та чотири gameplay-ролі описані в коді, 10 modular candidates проаудитовані, але фактична faction/role visual differentiation ще потребує material/module art pass та пізньої UE visual validation.

PR #15 залишається Draft і не повинен merge-итись у `main` до завершення R14 verification gates.
