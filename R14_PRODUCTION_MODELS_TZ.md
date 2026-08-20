# OSTER CONFLICT — R14 PRODUCTION MODELS ТЗ

Дата старту: 2026-08-20
Робоча гілка: `feat/r14-production-models`
База: `main` @ `fbe66f7502f0cf6ecc621bba575c1e1b35e7e76b`

## 1. Мета

Перевести Oster Conflict від набору proxy/greybox-візуалів до контрольованого production-набору моделей без руйнування наявної multiplayer/gameplay логіки.

Гілка R14 є єдиною робочою гілкою для поточного циклу інтеграції моделей. Кожен логічний етап виконується окремими комітами. `main` не чіпати до завершення перевірки етапу.

## 2. Обов'язкові правила

- Перед підключенням asset перевіряти точний `/Game/...` шлях.
- Не видаляти робочу gameplay/network логіку заради заміни візуалу.
- Unreal binary/source assets вести через Git LFS відповідно до `.gitattributes`.
- Не комітити `Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/`, `.vs/`.
- Не називати asset готовим, доки не розрізнено стани: source present, imported, compile verified, runtime verified, visual verified, packaged/cook verified.
- Для зовнішніх asset обов'язково фіксувати джерело та ліцензію. Невідома ліцензія = development-only, без права вважати asset фінальним.
- Не тягнути пакети масово без потреби. Цільове залізо вимагає контролю VRAM/RAM, LOD/Nanite, streaming і soft references.
- Production-гілка не повинна повертати `Cube`, `Sphere`, `Cylinder` або інші BasicShapes як фінальний вид зброї, персонажа чи транспорту.

## 3. Етап 0 — інвентаризація та контракти

Статус: IN PROGRESS

1. Зібрати реєстр усіх наявних weapon/character/vehicle/environment/interior asset-ів.
2. Для кожного зафіксувати: Unreal path, тип mesh, skeleton, animations, materials/textures, collision/PhysicsAsset, LOD/Nanite, license/source, runtime owner/class, статус перевірки.
3. Виявити відсутні або сумнівні asset-и до завантаження нових.
4. Не дублювати те, що вже є у проєкті.
5. Ввести blacklist для непридатних транспортних моделей. Вантажівки не включати до активного бойового/цивільного vehicle pool R14.

Критерій завершення: реєстр не має невідомих активних runtime-моделей.

## 4. Етап 1 — зброя, найменший завершений production-контур

Пріоритет №1.

### 4.1 Моделі

Перевірити та довести всі реалізовані weapon-класи, включно з уже знайденими:

- AK-47
- MP5
- M1911
- M700
- Remington 870
- M249
- M14
- MAC-10
- TEC-9
- Lever Action .45-70
- M2 Browning як vehicle/deployable weapon
- anti-armor launcher та інші weapon-класи, що є в коді/weapon rack

Для кожної зброї:

- production mesh замість proxy;
- правильний масштаб;
- правильна орієнтація;
- окрема перевірка first-person та third-person;
- socket/attach point до рук;
- muzzle socket/trace origin;
- magazine/bolt/slide/pump/lever рухомі частини, якщо модель їх підтримує;
- collision тільки там, де вона потрібна gameplay;
- materials/PBR без загублених текстур;
- тіні та owner visibility без дублювання моделі в камері.

### 4.2 Руки та хват

Для кожної зброї перевірити:

- права рука на grip/trigger;
- ліва рука на handguard/pump/foregrip;
- пальці не проходять крізь mesh;
- зброя не проходить крізь камеру/тіло;
- hip pose;
- ADS pose;
- sprint/low-ready за наявності;
- crouch/jump/landing сумісність;
- переключення зброї не накопичує transform offsets.

Якщо skeleton зброї та animation skeleton несумісні, не форсувати sequence. Виконати retarget або додати сумісний animation set.

### 4.3 Стрільба

Для кожної зброї:

- fire animation;
- recoil animation/camera recoil;
- muzzle flash;
- muzzle smoke за потреби;
- shell/ejection FX, якщо модель/калібр це передбачає;
- tracer відповідно до режиму/типу боєприпасу;
- impact FX за surface type;
- синхронізований shot sound;
- animation notify для shot/muzzle/ejection, де це доцільно;
- multiplayer перевірка: authoritative shot не залежить від cosmetic animation.

### 4.4 Перезарядка

Для кожної зброї:

- reload animation;
- magazine swap або weapon-specific reload;
- shotgun shell-by-shell/pump logic, якщо реалізується;
- bolt/charging handle/slide/lever animation;
- timing gameplay reload узгоджений з animation notify;
- interrupt/switch/death/vehicle transitions не залишають animation state завислим.

Критерій завершення етапу 1: кожна зброя з weapon rack має production mesh, коректний хват, fire/reload presentation і проходить runtime visual check.

## 5. Етап 2 — персонажі

1. Перевірити наявний `QuantumCharacter` як технічну production-базу, не вважати один body mesh готовим набором команд.
2. Побудувати окремі visual profiles для команд/factions.
3. Усередині команд розбити зовнішність по gameplay-класах, які реально визначені в коді. Не вигадувати класи до перевірки enum/data.
4. Для кожного профілю: body, FP arms, materials, uniform/gear, helmet/headgear, vest, holster, backpack/pouches, PhysicsAsset, sockets.
5. Перевірити animation blueprint/retarget: idle, walk, run, crouch, jump/fall/land, aim offsets, weapon poses, fire/reload, death/ragdoll.
6. First-person arms мають бути окремо контрольовані від third-person body.
7. Командні відмінності не повинні змінювати gameplay collision або hit zones без окремої причини.

Критерій завершення: кожна команда і кожен реальний клас має стабільний production visual profile, що коректно реплікується.

## 6. Етап 3 — транспорт

### 6.1 Дозволений основний набір

- HMMWV
- HMMWV + M2 Browning
- BTR-4 «Буцефал» після підтвердження ліцензії фінального source
- armed pickup / pickup з кулеметом
- обмежений набір доречних civilian vehicles

### 6.2 Заборонений напрям

- випадкові вантажівки та GTA-подібний vehicle clutter не включати до active spawn pools;
- не видаляти пакет фізично, якщо в ньому є потрібні sedan/hatchback/SUV/pickup assets;
- прибирати вантажівки з runtime references/spawn selections, а не ламати спільний asset pack.

### 6.3 HMMWV + Browning

Наявний контракт `/Game/Production/Vehicles/HMMWV/SM_HMMWV_UA` + `/Game/Production/Weapons/M2/SM_M2_Browning` зберегти.

Перевірити:

- M2 закріплений на turret/barrel pivot;
- yaw/pitch візуалу відповідають authoritative turret aim;
- muzzle trace походить із фактичного ствола;
- gunner camera/seat не перетинає mesh;
- reload/ammo/fire FX M2;
- vehicle collision, mass, wheel/ground alignment;
- damage/health не залежать від replacement mesh.

### 6.4 Armed pickup

- використати наявний `AOCPickupGunTruck` як gameplay-клас;
- production pickup mesh має бути окремо зафіксований у реєстрі;
- кулемет, mount, gunner position, camera, muzzle та collision перевірити окремо;
- HMMWV не повинен бути єдиною production-підміною для pickup-класу після появи затвердженого pickup asset.

Критерій завершення: усі активні vehicle spawn classes мають конкретну дозволену production-модель; truck clutter відсутній.

## 7. Етап 4 — трава, дерева, небо, середовище

Використати та перевірити вже наявні `AdvancedVillagePack`, `PN_FoliageCollection`, `TileableForestRoad` та інші доречні пакети перед пошуком нових.

### 7.1 Рослинність

- grass varieties;
- дерева, характерні для Остра та радянських/післявоєнних насаджень;
- кущі;
- roadside/yard vegetation;
- wind response;
- foliage collision тільки де потрібна;
- LOD/Nanite/cull distances;
- density budget без перевантаження RTX 3050 Laptop.

### 7.2 Небо та освітлення

- production sky/atmosphere;
- Directional Light;
- SkyLight;
- volumetric clouds/fog у розумному бюджеті;
- day lighting без black/white exposure regressions;
- menu/background scene і gameplay scene не повинні конфліктувати експозицією.

## 8. Етап 5 — будівлі та інтер'єри

1. Перевірити `Modular_Rural_Cabin`, `AdvancedVillagePack`, наявні scene packs та Fab assets.
2. Визначити набори: житлові будинки, господарські споруди, гаражі, паркани, ворота, міські props.
3. Інтер'єри: столи, шафи, дивани, кухня, холодильник, ПК/ноутбук, світильники, побутові props.
4. Двері/ворота/світло повинні використовувати наявну interaction logic, а не бути новими несумісними системами.
5. Interior meshes: collision, lightmaps/Lumen suitability, material instances, draw calls, streaming.
6. Не заповнювати кожну кімнату однаковим prefab-набором.

Критерій завершення: доступна тестова будівля з повним production interior pass без proxy-кубів.

## 9. Етап 6 — технічна чистота asset pipeline

Для всіх категорій:

- стабільна naming convention;
- canonical `/Game/Production/...` для імпортованих зовнішніх production asset-ів;
- source-файли окремо від `/Content`;
- attribution/license manifest;
- redirector cleanup;
- duplicate detection;
- material instance reuse;
- texture size budget;
- LOD/Nanite policy;
- collision complexity policy;
- PhysicsAsset для skeletal meshes;
- sockets/IK/retarget assets;
- soft references/streaming для важких world assets;
- відсутність hard reference на development-only asset у shipping path.

## 10. Етап 7 — перевірки

Після кожної великої категорії:

1. Static/source contract check.
2. UE Editor compile.
3. Game/Client compile.
4. Server compile там, де toolchain дозволяє.
5. Asset load validation.
6. Listen-server runtime test.
7. First-person visual test.
8. Third-person/remote player visual test.
9. Vehicle enter/exit/fire test.
10. Cook/package validation.
11. Log scan: missing assets, skeleton mismatch, duplicate object/class, material errors.
12. Performance pass: frame time, VRAM/RAM, draw calls/foliage density.

## 11. Definition of Done

R14 готовий до merge лише коли:

- усі активні weapon classes мають production visual;
- зброя коректно тримається в руках і має fire/reload presentation;
- персонажі розведені по командних/класових visual profiles;
- HMMWV + M2 та armed pickup перевірені;
- випадкові вантажівки не спавняться;
- активний vehicle pool має production meshes;
- трава/дерева/небо/інтер'єри підключені з performance policy;
- немає фінальних BasicShape proxy в бойовому runtime;
- усі зовнішні asset-и мають перевірений license status;
- compile/runtime/visual/cook стани зафіксовані окремо;
- regression не ламає multiplayer, bots, UI, map startup та build scripts.
