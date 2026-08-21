# OSTER CONFLICT — WORK LEDGER

> Постійний журнал того, **що просив користувач, що реально зроблено, що не дороблено, скільки разів вимога повторювалась, яким commit це змінювалось і чи підтверджено playtest**.

Оновлювати цей файл після кожного суттєвого робочого блоку.

## 1. Поточний контекст

- Repository: `valentronus95/OsterConflict`
- Active integration branch: `main`
- UE target: 5.8.x Windows
- Unreal project: `OsterConflict/OsterConflict.uproject`
- Current location ТЗ: `STADION_OSTER_TZ.md` + `OsterConflict/Docs/Locations/SILPO_OSTER_TZ.md`
- 2026-08-20 recovery: accidental R13 tree rollback on `main` was reversed without rewriting history; current source is again the R14 integration tree, with a current-main R14 launcher.
- Точний repeat count починаємо вести системно з цього ledger. Для старих вимог, які явно повторювалися до створення файлу, використовується `≥N`, а не вигадане точне число.
- **2026-08-21 runtime override:** останній пакет із 14 runtime-скрінів має пріоритет над старими optimistic/code-only записами. Поточний runtime не можна називати виправленим, verified або готовим до приймального тесту. Спочатку усуваються зафіксовані runtime-регресії, потім виконується UE 5.8 playtest current `main`.
- Окремий persistent evidence-файл для цього пакета: `RUNTIME_AUDIT_2026-08-21.md`.

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
| LOC-MUSEUM-001 | Музей має бути окремим правильним landmark, не змішаним з Будинком культури/Сільпо | ≥3 | IN_PROGRESS | У 14-screen runtime evidence знову присутній legacy/mixed blockout у зоні landmark. Current R14 museum source та separation guards існують, але їх runtime-ефект не доведений. Потрібні authoritative owner + exclusion zone і новий UE 5.8 `LocationTest=1` playtest current `main`. |
| LOC-STADIUM-001 | Стадіон Остер має бути окремим hard-georeferenced site, узгодженим із музеєм і реальними картами | ≥2 | CODED_UNTESTED | Реалізацію зі `stadion-oster` контрольовано перенесено в `main`: canonical WGS84 anchor `50.949360, 30.884660`, окремий authoritative owner, modern field/sport zones, вхідна стела, дерева, стежки, будинки й локальні паркани. Structural stadium/ownership verifiers проходять. Потрібні UE 5.8 build/playtest, перевірка Z/collision/flicker і візуальне підтвердження; збережений `.zip` photo-pack не проходить ZIP integrity test і має бути відновлений з 17 оригіналів. |
| LOC-TERRAIN-001 | За музеєм має бути спуск вниз, нижче хати | ≥2 | CODED_UNTESTED | Доданий окремий collision terrain + lower residential district; terrain swap зроблено fail-safe. Потрібен build/playtest і корекція форми/висоти за фото/місцевістю. |
| LOC-SILPO-001 | Сільпо має бути на своїй локації, окремо від музею | ≥3 | IN_PROGRESS | 14-screen runtime evidence показує, що legacy/blockout geometry ще може опинятися в зоні Silpo/Museum/Culture. Photo-driven Silpo R14.0–R14.3 і окремий owner є в source, але runtime це не підтвердив. Потрібні owner/exclusion-zone cleanup та UE 5.8 `LocationTest=1` retest. |
| LOC-WATERTOWER-001 | Водонапірна вежа біля Сільпо має бути окремим правильним landmark | 1 | TODO | Потрібно знайти/підтвердити фото та географічну прив'язку, перевірити наявні assets або створити/адаптувати модель. Не починати до закриття runtime-регресій поточного пріоритету. |
| LOC-CULTURE-001 | Будинок культури має стояти на своїй локації, не в музеї | ≥3 | IN_PROGRESS | Current source має окремі R14.6 Culture House / landmark-separation системи, але 14-screen runtime evidence не дозволяє вважати separation доведеним. Потрібен один placement-owner, exclusion zones від Museum/Silpo і current-main `LocationTest=1` playtest. |
| LOC-COLLEGE-001 | Коледж/технікум не повинен стояти у вигаданій точці | ≥2 | IN_PROGRESS | Неправильні College Facade/Access runtime layers раніше вимикались. Після основного runtime cleanup потрібно перевірити, чи інший legacy blockout не повертає їх або іншу geometry. |
| LOC-PARK-001 | Парк не повинен стояти у вигаданій точці | ≥2 | IN_PROGRESS | Неправильні park canopy/dressing/furniture layers раніше вимикались. Після основного runtime cleanup потрібен аудит усіх legacy placement passes. |
| VIS-FLICKER-001 | У грі не повинно все мерехтіти/перебудовуватись після входу | ≥2 | IN_PROGRESS | Частина late repair була вимкнена, але runtime evidence показує, що cleanup/placement порядок ще ненадійний: раннє одноразове очищення може виконатись до пізнього legacy spawn. Потрібна консолідація ownership/cleanup і перевірка після reveal. |
| UI-MENU-001 | Головне меню не повинно смикатись при вході/hover | ≥2 | CODED_UNTESTED | Прибрані повторні layout repair таймери 0.30/0.72; залишено один стартовий repair. Потрібен повторний playtest. |
| UI-TRAVEL-001 | Після START не має бути сірого старого меню на декілька секунд | ≥2 | CODED_UNTESTED | Loading/deployment presentation було перероблено на current `main`, але остаточного user runtime підтвердження немає. Потрібен повторний playtest через normal frontend → TEAM path. |
| GAME-WEAPONS-001 | Біля фактичного spawn має лежати вся реалізована зброя для тесту | ≥2 | IN_PROGRESS | Старий code-only запис про spawn-relative rack спростований 14-screen runtime evidence: видно legacy weapon spawn/rack біля старої world-зони (орієнтир близько `930000,-500000`) та `LocationTest=1` path не гарантує потрібний фактичний spawn. Потрібно прив'язати rack до реально possessed/deployed pawn і runtime-перевірити pickup усіх 11. |
| VIS-FP-001 | Прибрати кулі/прямокутники/debug geometry біля зброї | ≥2 | IN_PROGRESS | Runtime evidence показує primitive/proxy presentation щонайменше для M249, M1911 і MAC-10. Попередній code-only запис «fallback proxy приховано» не рахується підтвердженням. Потрібні production meshes або коректний невидимий fallback та playtest кожного класу. |
| ASSET-BTR-001 | BTR має використовувати production vehicle asset, без видимої fallback-коробки | 1 | IN_PROGRESS | У runtime evidence BTR представлений зеленою box/proxy geometry, тобто production asset не резолвиться або не застосовується. Перевірити asset path/runtime validation/fallback policy і лише після цього vehicle playtest. |
| ASSET-CHARACTER-001 | Гравець/боти мають використовувати production character/skin assets | 1 | IN_PROGRESS | Runtime evidence показує, що production character/skin pipeline не є стабільно підключеним; assets відсутні/розкидані або не резолвляться в runtime. Потрібна інвентаризація `Content`, canonical profiles і runtime binding без видимих primitive substitutes. |
| GAME-VEHICLE-INPUT-001 | Після виходу з авто повністю відновлюється керування персонажем | 1 | IN_PROGRESS | Runtime evidence: після vehicle exit не відновлюються гарантовано `GameOnly`, WASD, sprint, mouse-look/camera. Потрібна одна симетрична enter/exit input-state path і playtest. |
| UI-TACTICAL-MAP-001 | `M` відкриває tactical map і не конфліктує з іншою дією | 1 | IN_PROGRESS | Runtime evidence: `M` прив'язаний не до tactical map / tactical map не відкривається як очікується. Потрібно знайти active binding, прибрати конфлікт і перевірити open/close + input focus. |
| VIS-HOUSES-001 | Використовувати вже наявні реальні моделі хат замість кубів | ≥2 | CODED_UNTESTED | `SM_House_Var01/02` повернуті як real-mesh layer; нижня зона за музеєм теж використовує real houses. Потрібен playtest масштабу/grounding після legacy-blockout audit. |
| VIS-FENCES-001 | Використовувати реальні паркани замість видимих кубів | 1 | IN_PROGRESS | `SM_Fence_Var01..04` підключені, proxy collision збережена невидимою. Виявлено неправильний stretching до 12× — треба переробити на tiled sections після критичних runtime fixes. |
| VIS-STREETLIGHT-001 | Підключати готові environment assets, які вже лежать у Content | 1 | CODED_UNTESTED | `AdvancedVillagePack/SM_StreetLight` підключений до road network; виключена музейна valley zone. Commit `401971b0...`. Потребує runtime перевірки після ownership cleanup. |

## 4. Важливі вже виконані технічні фікси

| ID | Результат | Status | Commit / примітка |
|---|---|---|---|
| BUILD-UE58-001 | UE 5.8 compile blocker `UObject/CoreUObjectDelegates.h` виправлено на правильний include | VERIFIED BUILD | Build користувача: `Result: Succeeded`. Початковий fix commit `37a3a6d5...`. |
| CRASH-OBJECTNAME-001 | Crash `Cannot replace existing object of a different class` для `R13_BusStationConcrete` | VERIFIED RUNTIME FOR REPORTED CRASH | Конфлікт MaterialInstanceDynamic/ISM імен розділено; аналогічні конфлікти також перевірялись в інших photo-model systems. Після змін користувач зміг увійти в gameplay замість попереднього crash. |
| GAME-WEAPON-MAIN-001 | Forward-port spawn-relative weapon rack logic у `main` | MERGED TO MAIN, RUNTIME REGRESSED | `main` commit `1b2aed220452ab1942a5de519238b30abb5f6a7c`. Merge факт зберігається, але 14-screen runtime evidence показує, що фактичний spawn/rack усе ще неправильний; див. `GAME-WEAPONS-001`. |
| TERRAIN-FAILSAFE-001 | Старий Ground вимикається тільки після успішного створення нового segmented terrain | CODED_UNTESTED | Commit `a2948d098304f79f39fad89dc6077ef8647f08f3`. |
| GAME-WEAPON-ALL-001 | Rack розширено до 11 реалізованих pickup classes з anti-armor launcher | CODED_UNTESTED | Commit `588feaf59d3da346689f3164d953b02cf54987c7`. Це доводить наявність code path, але не правильний runtime placement/pickup. |
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

### PROCESS-004 — Launcher/tree mismatch
Проблема: stale launcher/branch label раніше трактувався як доказ стану всього source tree.

Рішення:
- версію source визначати по HEAD/tree та integration commits;
- launcher ремонтувати окремо, не замінювати весь tree;
- перед recovery створювати backup branch;
- normal gameplay і Sandbox/LocationTest мають різні чіткі contracts.

### PROCESS-005 — One-shot cleanup before late spawn
Проблема: одноразове очищення legacy geometry може відпрацювати раніше, ніж інший runtime subsystem пізніше створить цю geometry знову.

Рішення:
- cleanup не замінює ownership;
- заборонити не-owner subsystem створювати geometry у protected landmark zone;
- validation після завершення startup passes повинна падати/логувати конфлікт, а не мовчки залишати дубль.

## 6. Остання підтверджена робоча сесія

### 2026-08-21 — runtime evidence correction after 14 screenshots

Зафіксовано як authoritative current evidence:
- primitive/proxy weapon presentation для M249, M1911, MAC-10;
- BTR fallback/proxy box замість production model;
- production character/skin binding не підтверджений runtime;
- weapon rack/spawn не доведений біля фактичного gameplay spawn; присутній legacy placement / `LocationTest=1` mismatch;
- legacy blockout/placement overlap у Museum/Silpo/Culture scope;
- one-shot cleanup не гарантує відсутність пізнього respawn geometry;
- vehicle exit input/camera restoration несправний;
- `M` не виконує очікуваний tactical-map contract.

Окремо branch audit уже зафіксував і source-level виправляв launcher-path, lighting baseline та AK first-person yaw. Ці source fixes не перекривають перелічені вище runtime defects і не дають права називати поточний gameplay verified.

## 7. Наступна черга робіт

Порядок зафіксований і не змінюється без нового runtime evidence:

1. `WORK_LEDGER` — привести статуси до фактичного runtime-стану. **Поточний крок виконано як ledger correction; це не runtime fix.**
2. Root persistent runtime audit по 14 скрінах: `RUNTIME_AUDIT_2026-08-21.md`.
3. `LocationTest=1` — перевірити/виправити current-main direct runtime-map contract без старого placement path.
4. Spawn/weapon rack — фактичний possessed/deployed pawn + усі 11 pickup classes.
5. Production assets — weapons → BTR/vehicles → character/skins; жодних видимих production proxies.
6. Vehicle input — симетричне enter/exit відновлення input mode, movement, sprint, mouse-look і camera.
7. Tactical map — `M` → open/close tactical map без binding/focus conflict.
8. Ownership/exclusion zones — Museum / Silpo / Culture: один landmark = один placement-owner; чужа geometry не може з'явитися в protected zone.
9. Аудит решти legacy blockout і late runtime placement systems.

**Заборона поточного етапу:** не створювати нові декоративні R15/R16 layers, поки цей runtime backlog не закритий і не підтверджений UE playtest.
