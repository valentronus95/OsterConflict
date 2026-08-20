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

## 3. Активні вимоги

| ID | Вимога | Repeat | Status | Фактичний стан / що залишилось |
|---|---|---:|---|---|
| LOC-MUSEUM-001 | Музей має бути окремим правильним landmark, не змішаним з Будинком культури/Сільпо | ≥3 | CODED_UNTESTED | Playtest на помилково відновленому R13 tree знову показав старий змішаний civic/Silpo вигляд. Цей тест не перевіряв актуальний R14 source. R14 integration tree відновлено на `main`; R14.5 museum source повернуто. Потрібен новий UE 5.8 build/playtest саме current `main`. |
| LOC-STADIUM-001 | Стадіон Остер має бути окремим hard-georeferenced site, узгодженим із музеєм і реальними картами | ≥2 | CODED_UNTESTED | Реалізацію зі `stadion-oster` контрольовано перенесено в `main`: canonical WGS84 anchor `50.949360, 30.884660`, окремий authoritative owner, modern field/sport zones, вхідна стела, дерева, стежки, будинки й локальні паркани. Legacy source visuals приховуються синхронно; delayed civic/landmark passes більше не додають окремі stadium-шари. Structural stadium/ownership verifiers проходять. Потрібні UE 5.8 build/playtest, перевірка Z/collision/flicker і візуальне підтвердження; збережений `.zip` photo-pack не проходить ZIP integrity test і має бути відновлений з 17 оригіналів. |
| LOC-TERRAIN-001 | За музеєм має бути спуск вниз, нижче хати | ≥2 | CODED_UNTESTED | Доданий окремий collision terrain + lower residential district; terrain swap зроблено fail-safe. Потрібен build/playtest і корекція форми/висоти за фото/місцевістю. |
| LOC-SILPO-001 | Сільпо має бути на своїй локації, окремо від музею | ≥3 | CODED_UNTESTED | R13 rollback-playtest не є валідним тестом R14 Silpo. На актуальному `main` відновлено photo-driven Silpo R14.0–R14.3 з canonical WGS84 `50.948833799986254, 30.87572244094098`, окремим R14 owner і без legacy `OCR13SilpoPhotoModelSubsystem.cpp`. Потрібен новий UE 5.8 build/runtime test current `main`, collision/door/navigation/flicker та візуальна звірка з фото. |
| LOC-WATERTOWER-001 | Водонапірна вежа біля Сільпо має бути окремим правильним landmark | 1 | TODO | Потрібно знайти/підтвердити фото та географічну прив'язку, перевірити наявні assets або створити/адаптувати модель. |
| LOC-CULTURE-001 | Будинок культури має стояти на своїй локації, не в музеї | ≥3 | IN_PROGRESS | Старий R13 Culture House photo-model повернувся лише через помилковий tree rollback; у відновленому R14 `main` legacy `OCR13CultureHousePhotoModelSubsystem.cpp` відсутній. Сам правильний landmark ще не повернуто без підтвердженої ділянки. Потрібен current-main playtest, щоб підтвердити відсутність старого composite. |
| LOC-COLLEGE-001 | Коледж/технікум не повинен стояти у вигаданій точці | ≥2 | IN_PROGRESS | Неправильні College Facade/Access runtime layers вимкнені. Потрібна правильна прив'язка перед поверненням. |
| LOC-PARK-001 | Парк не повинен стояти у вигаданій точці | ≥2 | IN_PROGRESS | Неправильні park canopy/dressing/furniture layers вимкнені. Потрібна правильна прив'язка перед поверненням. |
| VIS-FLICKER-001 | У грі не повинно все мерехтіти/перебудовуватись після входу | ≥2 | IN_PROGRESS | Вимкнений один whole-map late repair; loading overlay подовжений, Silpo cleanup пересунутий раніше. Потрібно прибрати/консолідувати інші late runtime passes та overlapping geometry. |
| UI-MENU-001 | Головне меню не повинно смикатись при вході/hover | ≥2 | CODED_UNTESTED | Прибрані повторні layout repair таймери 0.30/0.72; залишено один стартовий repair. Потрібен повторний playtest. |
| UI-TRAVEL-001 | Після START не має бути сірого старого меню на декілька секунд | ≥2 | CODED_UNTESTED | Loading overlay зроблено topmost/fullscreen і затримано до завершення startup passes. Потрібен повторний playtest. |
| GAME-WEAPONS-001 | Біля фактичного spawn має лежати вся реалізована зброя для тесту | ≥2 | CODED_UNTESTED | Rack прив'язаний до реального deployed pawn. Додано 11-й клас — anti-armor launcher. Потрібно build/playtest pickup усіх 11. |
| VIS-FP-001 | Прибрати кулі/прямокутники/debug geometry біля зброї | ≥2 | CODED_UNTESTED | First-person fallback proxy geometry приховано. Потрібен playtest різних weapon classes. |
| VIS-HOUSES-001 | Використовувати вже наявні реальні моделі хат замість кубів | ≥2 | CODED_UNTESTED | `SM_House_Var01/02` повернуті як real-mesh layer; нижня зона за музеєм теж використовує real houses. Потрібен playtest масштабу/grounding. |
| VIS-FENCES-001 | Використовувати реальні паркани замість видимих кубів | 1 | IN_PROGRESS | `SM_Fence_Var01..04` підключені, proxy collision збережена невидимою. Виявлено неправильний stretching до 12× — треба переробити на tiled sections. |
| VIS-STREETLIGHT-001 | Підключати готові environment assets, які вже лежать у Content | 1 | CODED_UNTESTED | `AdvancedVillagePack/SM_StreetLight` підключений до road network; виключена музейна valley zone. Commit `401971b0...`. |

## 4. Важливі вже виконані технічні фікси

| ID | Результат | Status | Commit / примітка |
|---|---|---|---|
| BUILD-UE58-001 | UE 5.8 compile blocker `UObject/CoreUObjectDelegates.h` виправлено на правильний include | VERIFIED BUILD | Build користувача: `Result: Succeeded`. Початковий fix commit `37a3a6d5...`. |
| CRASH-OBJECTNAME-001 | Crash `Cannot replace existing object of a different class` для `R13_BusStationConcrete` | VERIFIED RUNTIME FOR REPORTED CRASH | Конфлікт MaterialInstanceDynamic/ISM імен розділено; аналогічні конфлікти також перевірялись в інших photo-model systems. Після змін користувач зміг увійти в gameplay замість попереднього crash. |
| GAME-WEAPON-MAIN-001 | Forward-port spawn-relative weapon rack logic у `main` | MERGED TO MAIN | `main` commit `1b2aed220452ab1942a5de519238b30abb5f6a7c`. Це не означає, що весь R13 merged. |
| TERRAIN-FAILSAFE-001 | Старий Ground вимикається тільки після успішного створення нового segmented terrain | CODED_UNTESTED | Commit `a2948d098304f79f39fad89dc6077ef8647f08f3`. |
| GAME-WEAPON-ALL-001 | Rack розширено до 11 реалізованих pickup classes з anti-armor launcher | CODED_UNTESTED | Commit `588feaf59d3da346689f3164d953b02cf54987c7`. |
| ASSET-STREETLIGHT-001 | Реальний `SM_StreetLight` використовується road infrastructure | CODED_UNTESTED | Commit `401971b0efec020d81d02b263f8c8e4097da28e9`. |
| LOC-SILPO-MAIN-001 | Photo-driven Silpo Oster R14.0–R14.3 forward-port у `main` | CODED_UNTESTED | Original integration commit `ad689dff859bc65332669788cb94f727623ce7ab`; restored after rollback by recovery commit `2db682b1acde8ac3b0ffb80abd5faedca87f35f0`. |
| RECOVERY-R14-MAIN-001 | Повернуто сьогоднішнє R14 integration tree після помилкового R13 rollback | CODED_UNTESTED | Recovery commit `2db682b1acde8ac3b0ffb80abd5faedca87f35f0`; history не переписувалась. `START_HERE.cmd` переведено на current R14 (`a4184ff3...`), додано прямий R14 Sandbox launcher (`ba20e959...`). |

## 5. Відомі історичні проблеми процесу

### PROCESS-001 — «Названо готовим до playtest»
Проблема: кодова зміна інколи описувалась як фактично готова до того, як локальний UE runtime це підтвердив.

Рішення:
- використовувати `CODED_UNTESTED`;
- `VERIFIED` тільки після build/playtest;
- не переносити формулювання «виправлено» в ledger без тестового доказу.

### PROCESS-002 — Локації змішувались різними runtime subsystems
Проблема: Museum, Culture House, Silpo та інші site systems могли створювати/перебудовувати геометрію незалежними таймерами, що давало неправильне змішання та мерехтіння.

Рішення:
- один owner на landmark/site;
- інші subsystems можуть тільки додавати дозволений detail layer;
- ніякого пізнього replacement після reveal;
- координати з низькою впевненістю позначати provisional.

### PROCESS-003 — Реальні assets існували, але blockout повертався поверх них
Проблема: наприклад, real house meshes могли бути підключені, а пізніший subsystem знову показував procedural architecture.

Рішення:
- перед написанням нового primitive blockout інвентаризувати `Content`;
- replacement logic не повинна затирати real-mesh layer;
- у ledger фіксувати asset path, який реально використовується.

### PROCESS-004 — Launcher/tree mismatch
Проблема: stale R11 `START_HERE.cmd` на актуальному R14 `main` був помилково трактований як доказ, що весь `main` старий. Через це tree `r13-content-gameplay-pass` був перенесений поверх актуального R14 source, і playtest закономірно повернув старі R13 location models.

Рішення:
- версію source визначати по HEAD/tree та наявних integration commits, не по одному launcher-файлу;
- stale launcher ремонтувати окремо, не замінювати весь tree;
- перед recovery створювати backup branch;
- current-main launcher прямо маркує R14 і запускає current source.

## 6. Остання підтверджена робоча сесія

### 2026-08-20 — R14 main recovery + location isolation retest preparation

Зроблено:
- відновлено R14 integration tree, який існував у `main` до помилкового R13 rollback;
- повернуто museum R14.5, Silpo R14.0–R14.3, stadium hard-georeferenced pass та інші сьогоднішні main integrations;
- підтверджено, що в актуальному R14 tree немає legacy `OCR13SilpoPhotoModelSubsystem.cpp` і `OCR13CultureHousePhotoModelSubsystem.cpp`;
- `START_HERE.cmd` більше не запускає R11/R13 і маркує `R14 CURRENT MAIN`;
- додано `RUN_R14_MAIN_SANDBOX_TEST.cmd`, який спочатку компілює current main, потім напряму відкриває `OsterConflict_Runtime` у Sandbox;
- museum R14.5 launcher дозволено запускати з `main`;
- попередній помилковий стан збережено в backup branch `backup/accidental-r13-main-20260820-2340`.

Незакрито:
1. локально підтягнути current `main`, UE 5.8 build і новий R14 Sandbox playtest;
2. візуально підтвердити, що старий mixed civic/Culture House/Silpo blockout більше не повертається;
3. остаточно підтвердити Museum/Silpo placement, collision та flicker;
4. знайти підтверджену точку правильного Будинку культури перед його поверненням;
5. водонапірна вежа біля Сільпо;
6. topology/fence/flicker refinements.

## 7. Наступна черга робіт

1. UE 5.8 build → `START_HERE.cmd` option 5 → current-main R14 location playtest.
2. За результатом playtest оновити `LOC-MUSEUM-001`, `LOC-SILPO-001`, `LOC-CULTURE-001` без передчасного `VERIFIED`.
3. `LOC-WATERTOWER-001`: photo/geography verification та окремий landmark.
4. `LOC-CULTURE-001`: знайти верифіковану ділянку і повернути Будинок культури окремо.
5. `LOC-TERRAIN-001`: topology refinement за реальними орієнтирами.
6. `VIS-FENCES-001`: tiled fence sections.
7. `VIS-FLICKER-001`: аудит усіх timer-based late replacements.
