# OSTER CONFLICT — WORK LEDGER

> Постійний журнал того, **що просив користувач, що реально зроблено, що не дороблено, скільки разів вимога повторювалась, яким commit це змінювалось і чи підтверджено playtest**.

Оновлювати цей файл після кожного суттєвого робочого блоку.

## 1. Поточний контекст

- Repository: `valentronus95/OsterConflict`
- Active integration branch: `main`
- UE target: 5.8.x Windows
- Unreal project: `OsterConflict/OsterConflict.uproject`
- Current location ТЗ: `STADION_OSTER_TZ.md` + `OsterConflict/Docs/Locations/SILPO_OSTER_TZ.md`
- 2026-08-20 recovery: accidental R13 tree rollback on `main` was reversed without rewriting history; current source is again the R14 integration tree.
- Точний repeat count починаємо вести системно з цього ledger. Для старих вимог, які явно повторювалися до створення файлу, використовується `≥N`, а не вигадане точне число.
- **2026-08-21 runtime override:** останній пакет із 14 runtime-скрінів має пріоритет над старими optimistic/code-only записами. Поточний runtime не можна називати виправленим, verified або готовим до приймального тесту. Спочатку усуваються зафіксовані runtime-регресії, потім виконується UE 5.8 playtest current `main`.
- Окремий persistent evidence-файл для цього пакета: `RUNTIME_AUDIT_2026-08-21.md`.
- 2026-08-21 source recovery після runtime-аудиту: додані `LocationTest=1` rack, LFS preflight, weapon real-mesh fallback, vehicle-exit input recovery, tactical map, persistent landmark exclusion guard і legacy-blockout audit. Усе це лишається `IN_PROGRESS/CODED_UNTESTED` до runtime.
- Перший user build після цих source-змін: ownership verifier `PASS`, LFS hydration пройшла, UHT пройшов, але UE build зупинився на `C4458` у `OCTacticalMapSubsystem.cpp` через локальну змінну `Slot`. Виправлено commit `bb7d49b5d265d13947dc98651c28ab7a7e7a5ae0`; повторний build ще не виконаний.
- Для користувача канонічний вхід тепер один: `START_HERE.cmd`. Version-named `RUN_*.cmd` — внутрішні helper scripts, нові user-facing launcher-файли на кожну версію не створювати. Спрощено commit `161591dc8f4a3e1953d224c762c8acc33b18970f`.
- Другий user launch після спрощення `START_HERE.cmd` був зупинений ще до UE build самим `VERIFY_R14_MAIN_LOCATION_OWNERSHIP.py`: verifier помилково вимагав текстові маркери `CURRENT MAIN`, `R14`, `R14.7` у version-neutral user launcher. Це не regression локацій. Verifier виправлено commit `bc106688ae1c7d764b86912c3bfccbd1799ddd6e`: тепер він перевіряє canonical `START_HERE.cmd` та правильні internal routes, а не номер версії у user-facing назві. Повторний локальний запуск pending.

## 2. Правила журналу

Для кожної вимоги зберігати:
- `ID`;
- коротку вимогу;
- `Repeat` — скільки разів користувач повертався до незакритої вимоги;
- `Status`;
- що фактично зроблено;
- останній relevant commit;
- що ще залишилось;
- результат build/playtest.

**Критичне правило:** якщо користувач повторив вимогу, яка раніше була названа «готовою», але playtest її спростував, статус повертається з `VERIFIED` у `IN_PROGRESS` або `CODED_UNTESTED`, а repeat count збільшується.

**Runtime-over-code rule:** якщо runtime-скрін, лог або playtest показує дефект, запис про committed/coded fix не є доказом виправлення. Для такого пункту використовується `IN_PROGRESS` до нового runtime-підтвердження.

## 3. Активні вимоги

| ID | Вимога | Repeat | Status | Фактичний стан / що залишилось |
|---|---|---:|---|---|
| LOC-MUSEUM-001 | Музей має бути окремим правильним landmark, не змішаним з Будинком культури/Сільпо | ≥3 | IN_PROGRESS | Persistent exclusion guard доданий (`dc07e098...`, `c248578a...`), але legacy R13.7 museum pipeline все ще має late cleanup ~4.95s і rebuild ~5.10s. Потрібен UE 5.8 `LocationTest=1` runtime-перегляд після 8+ секунд. |
| LOC-STADIUM-001 | Стадіон Остер має бути окремим hard-georeferenced site, узгодженим із музеєм і реальними картами | ≥2 | CODED_UNTESTED | Реалізацію зі `stadion-oster` контрольовано перенесено в `main`: canonical WGS84 anchor `50.949360, 30.884660`, окремий authoritative owner, modern field/sport zones, вхідна стела, дерева, стежки, будинки й локальні паркани. Structural stadium/ownership verifiers проходять. Потрібні UE 5.8 build/playtest, перевірка Z/collision/flicker і візуальне підтвердження. |
| LOC-TERRAIN-001 | За музеєм має бути спуск вниз, нижче хати | ≥2 | CODED_UNTESTED | Доданий окремий collision terrain + lower residential district; terrain swap зроблено fail-safe. Потрібен build/playtest і корекція форми/висоти за фото/місцевістю. |
| LOC-SILPO-001 | Сільпо має бути на своїй локації, окремо від музею | ≥3 | IN_PROGRESS | Photo-driven Silpo owner є; persistent exclusion guard доданий (`dc07e098...`, `c248578a...`). Runtime separation ще не підтверджений. Потрібен `LocationTest=1` retest. |
| LOC-WATERTOWER-001 | Водонапірна вежа біля Сільпо має бути окремим правильним landmark | 1 | TODO | Потрібно знайти/підтвердити фото та географічну прив'язку, перевірити наявні assets або створити/адаптувати модель. Не починати до закриття runtime-регресій поточного пріоритету. |
| LOC-CULTURE-001 | Будинок культури має стояти на своїй локації, не в музеї | ≥3 | IN_PROGRESS | Окремий current owner + persistent exclusion guard є (`dc07e098...`, `c248578a...`). Runtime доказу після нових змін немає. Потрібен current-main `LocationTest=1` playtest. |
| LOC-COLLEGE-001 | Коледж/технікум не повинен стояти у вигаданій точці | ≥2 | IN_PROGRESS | Неправильні College Facade/Access runtime layers раніше вимикались. Після основного runtime cleanup потрібно перевірити, чи інший legacy blockout не повертає їх або іншу geometry. |
| LOC-PARK-001 | Парк не повинен стояти у вигаданій точці | ≥2 | IN_PROGRESS | Неправильні park canopy/dressing/furniture layers раніше вимикались. Після основного runtime cleanup потрібен аудит усіх legacy placement passes. |
| VIS-FLICKER-001 | У грі не повинно все мерехтіти/перебудовуватись після входу | ≥2 | IN_PROGRESS | One-shot cleanup замінено persistent startup/late-spawn guard, але source-аудит знайшов старий museum cleanup ~4.95s + rebuild ~5.10s. Це ще треба прибрати/консолідувати після runtime підтвердження фактичного прояву. |
| UI-MENU-001 | Головне меню не повинно смикатись при вході/hover | ≥2 | CODED_UNTESTED | Прибрані повторні layout repair таймери 0.30/0.72; залишено один стартовий repair. Потрібен повторний playtest. |
| UI-TRAVEL-001 | Після START не має бути сірого старого меню на декілька секунд | ≥2 | CODED_UNTESTED | Loading/deployment presentation було перероблено на current `main`, але остаточного user runtime підтвердження немає. Потрібен повторний playtest через normal frontend → TEAM path. |
| GAME-WEAPONS-001 | Біля фактичного spawn має лежати вся реалізована зброя для тесту | ≥2 | IN_PROGRESS | `LocationTest=1` rack тепер прив'язаний до реально possessed/deployed pawn, містить 11 pickup classes і suppress-ить legacy world pickups у цьому diagnostic path. Source coded; потрібен UE runtime pickup-test усіх 11. |
| VIS-FP-001 | Прибрати кулі/прямокутники/debug geometry біля зброї | ≥2 | IN_PROGRESS | Доданий real-R13 fallback для M249/M1911/MAC-10/Remington (`ffa01282...`, `9c00e43b...`). Exact production paths для M249/Remington відсутні, тому production verification OPEN. Потрібен runtime. |
| ASSET-BTR-001 | BTR має використовувати production vehicle asset, без видимої fallback-коробки | 1 | IN_PROGRESS | Exact `/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus` asset у repo не знайдений; source fallback лишається primitive/proxy. Production fix ще не зроблений, runtime verification неможлива до появи intended asset. |
| ASSET-CHARACTER-001 | Гравець/боти мають використовувати production character/skin assets | 1 | IN_PROGRESS | QuantumCharacter body/arms є через Git LFS. Launcher тепер виконує `git lfs pull` і блокує тест на tiny/unhydrated critical assets (`54b8c2cd...`). Runtime binding ще не підтверджений. |
| GAME-VEHICLE-INPUT-001 | Після виходу з авто повністю відновлюється керування персонажем | 1 | IN_PROGRESS | Доданий transition recovery (`30cae92a...`, `7c4d9138...`): clear mappings, reset move/look ignore, `GameOnly`, rebuild local mappings після vehicle→character possession. Потрібен повторний enter→drive→exit runtime test. |
| UI-TACTICAL-MAP-001 | `M` відкриває tactical map і не конфліктує з іншою дією | 1 | IN_PROGRESS | Active conflict знайдено: `M` був `DeployTrap`. Trap перенесено на `V`, tactical map отримала `M` (`a6d31480...`, `d9d36c1...`). Перший build виявив `C4458 Slot` compile blocker; виправлено `bb7d49b5...`. Потрібен повторний build і runtime open/close/focus test. |
| VIS-HOUSES-001 | Використовувати вже наявні реальні моделі хат замість кубів | ≥2 | CODED_UNTESTED | `SM_House_Var01/02` повернуті як real-mesh layer; нижня зона за музеєм теж використовує real houses. Потрібен playtest масштабу/grounding після legacy-blockout audit. |
| VIS-FENCES-001 | Використовувати реальні паркани замість видимих кубів | 1 | IN_PROGRESS | `SM_Fence_Var01..04` підключені, proxy collision збережена невидимою. Виявлено неправильний stretching до 12× — треба переробити на tiled sections після критичних runtime fixes. |
| VIS-STREETLIGHT-001 | Підключати готові environment assets, які вже лежать у Content | 1 | CODED_UNTESTED | `AdvancedVillagePack/SM_StreetLight` підключений до road network; виключена музейна valley zone. Commit `401971b0...`. Потребує runtime перевірки після ownership cleanup. |
| LEGACY-BLOCKOUT-001 | Не дозволяти старому blockout непомітно перекривати current locations/assets | 1 | IN_PROGRESS | Створено `LEGACY_BLOCKOUT_AUDIT_2026-08-21.md` (`b137bee6...`). Confirmed high-risk: R13.7 museum 4.95/5.10 late mutation та raw fixed recovered-building site `(-69000,64500,0)`. Масове видалення не робити без runtime evidence. |

## 4. Важливі вже виконані технічні фікси

| ID | Результат | Status | Commit / примітка |
|---|---|---|---|
| BUILD-UE58-001 | UE 5.8 compile blocker `UObject/CoreUObjectDelegates.h` виправлено на правильний include | VERIFIED BUILD | Старіший user build: `Result: Succeeded`. Початковий fix commit `37a3a6d5...`. Новий recovery build має окремий tactical-map blocker, див. нижче. |
| BUILD-TACTICAL-MAP-001 | `C4458 declaration of Slot hides class member` у tactical map | CODED_UNTESTED | User build 2026-08-21 відтворив 7 compile errors у `OCTacticalMapSubsystem.cpp`; усі локальні `Slot` перейменовані на descriptive canvas-slot names. Commit `bb7d49b5d265d13947dc98651c28ab7a7e7a5ae0`. Повторний build pending. |
| CRASH-OBJECTNAME-001 | Crash `Cannot replace existing object of a different class` для `R13_BusStationConcrete` | VERIFIED RUNTIME FOR REPORTED CRASH | Конфлікт MaterialInstanceDynamic/ISM імен розділено; аналогічні конфлікти також перевірялись в інших photo-model systems. Після змін користувач зміг увійти в gameplay замість попереднього crash. |
| GAME-WEAPON-MAIN-001 | Forward-port spawn-relative weapon rack logic у `main` | MERGED TO MAIN, RUNTIME REGRESSED | Початковий `main` commit `1b2aed220452ab1942a5de519238b30abb5f6a7c`; після runtime regression current LocationTest rack був перероблений. |
| GAME-WEAPON-ALL-001 | LocationTest rack: 11 pickup classes біля possessed pawn | CODED_UNTESTED | `LocationTest=1` gating + 11 classes + anti-armor + world-pickup suppression. Потрібен runtime pickup test. |
| ASSET-LFS-PREFLIGHT-001 | Critical UE LFS assets hydrate/validate before test | CODED_UNTESTED | Launcher `git lfs pull` + minimum-size checks. Commit `54b8c2cd2a5c704e2692128ec539b29d04da28d2`. User run пройшов цей preflight. |
| VEHICLE-EXIT-RECOVERY-001 | Restore local input stack after exiting vehicle | CODED_UNTESTED | Commits `30cae92a...`, `7c4d9138...`. Runtime pending. |
| TACTICAL-MAP-SOURCE-001 | `M` tactical map; old DeployTrap moved to `V` | CODED_UNTESTED | Commits `a6d31480...`, `d9d36c1...`; compile blocker fixed `bb7d49b5...`. Runtime pending. |
| LANDMARK-EXCLUSION-001 | Museum/Silpo/Culture protected through startup + late actor spawn | CODED_UNTESTED | Commits `dc07e098...`, `c248578a...`: 40 passes / 8 sec + actor-spawn recheck + final validation. Runtime pending. |
| VERIFY-RUNTIME-RECOVERY-001 | Source-only guard for RT-01..RT-08 recovery contract | CODED_UNTESTED | `VERIFY_RUNTIME_RECOVERY_SOURCE.py`, commit `004b04ed...`; CI integration commit `6ad32abc...`. Source guard is not runtime proof. |
| LAUNCHER-UX-001 | Один user-facing launcher | CODED_UNTESTED | `START_HERE.cmd` спрощено до Test / Normal Game / Unreal Editor; commit `161591dc8f4a3e1953d224c762c8acc33b18970f`. Інші `RUN_*.cmd` вважати internal helpers, не створювати нові versioned user launchers. |
| LAUNCHER-VERIFY-001 | Version-neutral `START_HERE.cmd` не блокується старим R14.7 text check | CODED_UNTESTED | User run 2026-08-21 зупинився на двох false-positive `[FAIL]` до UE build. `VERIFY_R14_MAIN_LOCATION_OWNERSHIP.py` тепер перевіряє canonical launcher title + Test/Normal routes, а не `CURRENT MAIN/R14/R14.7` текст. Commit `bc106688ae1c7d764b86912c3bfccbd1799ddd6e`. Повторний локальний запуск pending. |
| TERRAIN-FAILSAFE-001 | Старий Ground вимикається тільки після успішного створення нового segmented terrain | CODED_UNTESTED | Commit `a2948d098304f79f39fad89dc6077ef8647f08f3`. |
| ASSET-STREETLIGHT-001 | Реальний `SM_StreetLight` використовується road infrastructure | CODED_UNTESTED | Commit `401971b0efec020d81d02b263f8c8e4097da28e9`. |
| LOC-SILPO-MAIN-001 | Photo-driven Silpo Oster R14.0–R14.3 forward-port у `main` | CODED_UNTESTED | Original integration commit `ad689dff859bc65332669788cb94f727623ce7ab`; restored after rollback by recovery commit `2db682b1acde8ac3b0ffb80abd5faedca87f35f0`. Runtime separation remains open under `LOC-SILPO-001`. |
| RECOVERY-R14-MAIN-001 | Повернуто R14 integration tree після помилкового R13 rollback | CODED_UNTESTED | Recovery commit `2db682b1acde8ac3b0ffb80abd5faedca87f35f0`; history не переписувалась. Відновлення source tree не є доказом коректного runtime. |

## 5. Відомі історичні проблеми процесу

### PROCESS-001 — «Названо готовим до playtest»
Проблема: кодова зміна інколи описувалась як фактично готова до того, як локальний UE runtime це підтвердив.

Рішення:
- використовувати `CODED_UNTESTED` для code-only змін;
- якщо runtime уже показує дефект — `IN_PROGRESS`;
- `VERIFIED` тільки після build/playtest;
- не переносити формулювання «виправлено» в ledger без тестового доказу.

### PROCESS-002 — Локації змішувались різними runtime subsystems
Проблема: Museum, Culture House, Silpo та інші site systems могли створювати/перебудовувати геометрію незалежними таймерами, що давало неправильне змішання та мерехтіння.

Рішення:
- один owner на landmark/site;
- інші subsystems можуть тільки додавати дозволений detail layer;
- у кожного landmark є exclusion zone від чужих placement systems;
- ніякого пізнього replacement після reveal;
- координати з низькою впевненістю позначати provisional.

### PROCESS-003 — Реальні assets існували, але blockout повертався поверх них
Проблема: real meshes могли бути підключені, а пізніший subsystem знову показував procedural/primitive architecture або vehicle/weapon proxies.

Рішення:
- перед написанням нового primitive blockout інвентаризувати `Content`;
- replacement logic не повинна затирати real-mesh layer;
- видимий proxy дозволений лише як явний diagnostic mode, не production gameplay;
- у ledger фіксувати asset path, який реально використовується.

### PROCESS-004 — Launcher/tree mismatch і launcher clutter
Проблема: stale launcher/branch label раніше трактувався як доказ стану всього source tree; накопичились окремі `RUN_R11/R14...` launchers, що плутає користувача. Другий 2026-08-21 запуск також показав, що source verifier був прив'язаний до текстового `R14.7` label у `START_HERE.cmd`, хоча user launcher має бути version-neutral.

Рішення:
- версію source визначати по HEAD/tree та integration commits;
- normal gameplay і Sandbox/LocationTest мають різні contracts, але для користувача вхід один — `START_HERE.cmd`;
- helper scripts можуть залишатись internal, але нові user-facing launcher-файли на кожну версію заборонені;
- source verifier перевіряє routes/contract, а не номер версії у user-facing launcher.

### PROCESS-005 — One-shot cleanup before late spawn
Проблема: одноразове очищення legacy geometry може відпрацювати раніше, ніж інший runtime subsystem пізніше створить цю geometry знову.

Рішення:
- cleanup не замінює ownership;
- заборонити не-owner subsystem створювати geometry у protected landmark zone;
- validation після завершення startup passes повинна падати/логувати конфлікт, а не мовчки залишати дубль.

## 6. Остання підтверджена робоча сесія

### 2026-08-21 — source recovery + два локальні launcher/build проходи після 14-screen audit

Authoritative runtime defects з 14 скрінів лишаються відкритими до нового gameplay run:
- primitive/proxy weapon presentation;
- BTR fallback/proxy box;
- character/skin runtime binding;
- weapon rack/spawn;
- Museum/Silpo/Culture overlap;
- late cleanup/respawn geometry;
- vehicle exit input/camera;
- tactical map contract.

Після цього source-side реалізовано recovery path для rack/assets/input/map/landmark guard/legacy audit.

Перший фактичний user launch 2026-08-21 підтвердив:
- Git LFS hydration дійшла до build;
- `R14 MAIN LOCATION OWNERSHIP: PASS`;
- UHT завершився;
- compile дійшов до нових recovery source files;
- build зупинився на конкретному `C4458` у tactical map, тому **гра ще не запускалась і runtime не перевірений**.

`C4458` source fix уже в `main` (`bb7d49b5...`), але наступний запуск після Pull був зупинений раніше самим ownership verifier через stale вимогу тексту `R14.7` у спрощеному `START_HERE.cmd`. Цей false-positive verifier fix уже в `main` (`bc106688...`). UE build після обох fixes ще не підтверджений.

## 7. Наступна черга робіт

1. На локальному ПК зробити `Fetch/Pull origin` current `main`.
2. Запускати **тільки `START_HERE.cmd` → `1. ТЕСТ ГРИ`**.
3. Спочатку ownership verifier має дати `PASS`, потім підтвердити `Result: Succeeded`. Якщо з'явиться новий compile blocker — виправляти його, не переходячи до декоративних задач.
4. Якщо build успішний, runtime перевірити: weapon rack/11 pickups, primitive weapon visuals, tactical map `M`, vehicle enter→exit input, Museum/Silpo/Culture після 8+ секунд, BTR/character assets.
5. Після diagnostic LocationTest окремо пройти `START_HERE.cmd` → `2. ЗВИЧАЙНА ГРА` і перевірити frontend → TEAM path/menu transition.
6. Лише після фактичного runtime proof підвищувати відповідні статуси з `IN_PROGRESS/CODED_UNTESTED`.

**Заборона поточного етапу:** не створювати нові декоративні R15/R16 layers, поки цей runtime backlog не закритий і не підтверджений UE playtest.