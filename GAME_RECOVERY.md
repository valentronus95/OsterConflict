# GAME RECOVERY

## Мета
Повернути OsterConflict у стан, де гра запускається без зависань, світ готовий до появи гравця, зброя/техніка/карта відображаються коректно, а меню та другорядні вікна мають єдиний завершений стиль.

## Робоча гілка
**Єдина робоча гілка:** `fix/pass45-runtime-rejection-material-closure-20260826`

Правила:
- не створювати нові робочі гілки для цього ТЗ;
- не робити reset/stash/discard локальних `Changes` користувача;
- PR #94 не merge до фактичного UE 5.8 runtime acceptance;
- не повертати старі proxy/заглушки як production-рішення;
- після кожного циклу: зроблено / у роботі / лишилось + % блоку і загальний % ТЗ.

---

## 1. CRITICAL — запуск світу без зависань
**Проблема:** після появи гравця гра зависає приблизно на 20 секунд, після чого світ і асети поступово домальовуються.

### Треба
- не випускати гравця у світ до готовності критичних runtime-асетів;
- важкі `LoadObject`, materialization, ISM/geometry build та інший великий startup не виконувати одним блокуючим кадром на game thread;
- критичні асети preload/async-load до spawn/possession;
- після появи гравця не повинно бути 10–20 секунд фріза та масового pop-in;
- Alt+Tab, minimize/maximize, меню та input повинні залишатися responsive під час підготовки світу;
- loading/deployment UI має показувати реальний стан готовності, а не приховувати завислий game thread.

### Acceptance
- spawn у готовий світ;
- немає фріза >1 секунди через startup materialization;
- немає масового домальовування карти після spawn.

---

## 2. CRITICAL — смерть, spectator, respawn
**Проблема:** після смерті можливий завислий spectator/сірий екран; відкривається Unreal Gameplay Debugger; відновлення займає неприйнятно довго.

### Треба
- respawn delay = **10 секунд**;
- після смерті керований spectator/death flow без втрати input/possession;
- через 10 секунд гарантований respawn або явна помилка в логах, без нескінченного зависання;
- Gameplay Debugger (`AI`, `BehaviorTree`, debug actor overlay) не повинен відкриватися звичайному гравцеві;
- сірий debug/spectator screen не повинен залишатися після respawn;
- death camera/spectator не повинні ламати HUD.

### Acceptance
- смерть -> 10 секунд -> стабільний respawn;
- жодного Gameplay Debugger у звичайній грі;
- HUD і керування відновлюються.

---

## 3. CRITICAL — гранати
**Проблема:** при кидку гранати можливий фріз 5–7 секунд; HUD не підключений; вибух занадто малий; після вибуху лишається дрібний повторний/живий VFX.

### Треба
- preload/prewarm гранатних mesh/material/VFX/audio до першого використання;
- кидок гранати не повинен викликати blocking asset load;
- підключити гранату до HUD: тип/кількість/активний слот;
- привести масштаб blast/VFX/audio/camera feedback до бойового гранатного вибуху;
- один explosion event на одну гранату;
- VFX/audio/decal/particle cleanup після завершення, без безкінечного міні-вибуху;
- перевірити damage radius і falloff окремо від візуального масштабу.

### Acceptance
- перший і наступні кидки без 5–7 с фріза;
- HUD показує гранату;
- один нормальний вибух, без залишкового циклу.

---

## 4. HIGH — зброя та respawn arsenal
**Проблема:** зброя лежить криво, частина відсутня, видно helper/proxy-компоненти, біля АК є сторонній синій диск, окремі моделі/частини розірвані або зависають.

### Треба
- повний required weapon roster на тестовому respawn/arsenal;
- не скидати showcase-зброю випадковою фізикою на землю;
- зробити стабільні rack/stand/socket transforms для кожної одиниці;
- прибрати видимі collision/helper/debug/proxy meshes;
- прибрати синій диск/технічний shape біля АК;
- перевірити M1911, launcher та іншу складену зброю на missing/detached components;
- одна production visual source-of-truth на weapon;
- pickup collider і interaction можуть бути невидимими, але не повинні ставати частиною видимої моделі.

### Acceptance
- вся потрібна зброя присутня;
- лежить/стоїть акуратно;
- жодних синіх дисків, білих proxy, detached parts або debug shapes.

---

## 5. HIGH — БТР-4 «Буцефал» і бойова техніка
**Проблема:** у runtime досі видно proxy/спрощену модель замість production БТР-4.

### Треба
- підключити production model БТР-4 «Буцефал»;
- зберегти collision, movement, turret/weapons, seats та damage hooks;
- прибрати runtime fallback/proxy, якщо production asset доступний;
- fallback дозволений лише як fail-closed діагностика з явним логом, не як нормальна картинка гри;
- перевірити HMMWV/M2 та інші production vehicles за тим самим правилом.

### Acceptance
- у звичайній грі видно production vehicle model, не блок-аут.

---

## 6. HIGH — карта Остер
**Проблема:** карта виглядає пустою; зник/не з'явився стадіон; музей не відповідає потрібній якості; додане випадкове військове сміття/мішки не відповідає реальному місцю.

### Треба
- відновити/підключити стадіон у canonical runtime;
- перевірити, що існуючі city landmarks не перекриваються runtime fallback-композицією;
- музей довести до reference-backed вигляду за фото, без грубих cabin/blockout рішень як фіналу;
- мішки, барикади та інший military clutter залишати тільки там, де це свідомо задано дизайном сцени;
- заповнити порожні ділянки реальними елементами Остера: будинки, паркани, дерева, дороги, двори, дрібні міські об'єкти;
- не створювати випадкову «військову базу» замість міста Остер.

### Acceptance
- стадіон присутній;
- музей впізнаваний за reference;
- карта не виглядає порожньою або випадково засміченою.

---

## 7. HIGH — UI стиль
**Проблема:** `Створення сервера` та `Розгортання` мають технічний сірий вигляд, нерівні відступи, слабку ієрархію та не відповідають стилю головного меню.

### Треба
- один UI theme/style owner для primary і secondary screens;
- `Створення сервера`: вирівняти поля, labels, buttons, spacing, borders, hover/focus/disabled states;
- `Розгортання`: прибрати величезні мертві площі, зібрати команду/групу/роль/появу в чіткий послідовний flow;
- side cards (`Матч`, `Ваш вибір`, `Ваша група`) оформити в тому самому стилі;
- `Налаштування` оформити тим самим production theme, без суцільних світло-сірих debug controls;
- фон, panel opacity, typography, button treatment та accent повинні відповідати головному меню;
- input fields, combo boxes, sliders і checkboxes не повинні виглядати як стандартні Unreal/debug controls;
- підтримати 1280x720 і вище без кривого масштабування/перекриття.

### Acceptance
- secondary screens виглядають як частина однієї гри, а не developer UI.

---

## 8. CRITICAL — налаштування: функціональність і redesign
**Проблема:** екран `Налаштування` відкривається, але фактично не працює: вкладки/поля/слайдери/checkbox/combo/buttons не дають нормальної взаємодії. Візуально екран перевантажений великими сірими контролами і виглядає як developer/debug UI.

### Треба
- відновити повну input-interaction для всього SettingsPanel після переходу з R13 frontend;
- перевірити, що панель не лишається `disabled` після приховування legacy UI;
- усі вкладки `ГРАФІКА / ЗВУК / КЕРУВАННЯ / ІНТЕРФЕЙС / ДОСТУПНІСТЬ` повинні реально перемикатися;
- `ЗАСТОСУВАТИ` застосовує зміни без закриття;
- `ЗБЕРЕГТИ Й НАЗАД` застосовує, зберігає і коректно повертає назад;
- `СКАСУВАТИ` відкидає незбережені зміни та повертає попередні значення;
- `СКИНУТИ НАЛАШТУВАННЯ` відновлює дефолтні значення з можливістю застосувати/скасувати;
- resolution/window mode/render scale/FPS limit/VSync/dynamic resolution та quality groups реально підключені до `UGameUserSettings`;
- audio sliders/checks реально підключені до audio user settings;
- controls/rebind, FOV, HUD scale, accessibility параметри реально зберігаються та застосовуються;
- після закриття/повторного відкриття показуються фактичні поточні значення;
- keyboard/mouse focus не губиться, Escape/назад працює передбачувано;
- зробити compact game-style layout: темні панелі, чіткі секції, нормальна типографіка, компактні combo/sliders/checks, помітний active tab, зрозумілі hover/focus/pressed states;
- зменшити візуальний шум і надмірну прозорість, щоб текст/контроли читались на фоні гри;
- зберегти адаптацію 1280x720 / 1600x900 / 1920x1080 і вище.

### Acceptance
- кожен control реагує на input;
- Apply/Save/Cancel/Defaults мають фактичну поведінку;
- значення зберігаються і відновлюються після повторного відкриття;
- settings виглядають у єдиному стилі з головним меню та іншими production screens;
- немає «сірої заблокованої форми», debug вигляду або мертвих кнопок.

---

## 9. MEDIUM — HUD
### Треба
- перевірити weapon slot list, ammo, grenades, role/squad/team, interaction prompt;
- не показувати debug/fallback text як production HUD;
- HUD не губиться після death/respawn/weapon switch;
- grenade state синхронізований з inventory/server state.

---

## 10. PERFORMANCE — runtime spikes
### Треба
- знайти всі startup/first-use блокуючі `LoadObject`/sync loads у WorldSubsystem/weapon/grenade/vehicle paths;
- перенести допустиме на async preload;
- staged materialization для важких world builds;
- не виконувати великі цикли spawn/register components в один кадр, якщо це викликає hitch;
- додати компактні runtime timing logs для startup, grenade first-use, respawn та landmark build.

### Acceptance
- немає 5–20 секундних зависань у звичайному gameplay flow.

---

## 11. FINAL UE 5.8 ACCEPTANCE
Окремо перевірити на поточному exact HEAD:
1. запуск через `START_HERE.cmd` -> `1`;
2. головне меню -> `Налаштування`;
3. усі settings tabs/controls + Apply/Save/Cancel/Defaults;
4. створення сервера;
5. deployment/team/group/role/spawn;
6. перший spawn;
7. усі weapon pickups;
8. grenade throw/explosion;
9. death -> 10 s respawn;
10. БТР-4 production visual;
11. стадіон/музей/основна карта;
12. Alt+Tab/minimize/maximize/close;
13. HUD після respawn;
14. немає Gameplay Debugger/debug shapes;
15. після цього пакетний runtime test через `START_HERE.cmd` -> `2`.

## Definition of Done
ТЗ закрите тільки коли ці пункти підтверджені фактичним UE 5.8 runtime, а не лише компіляцією/CI/source inspection.

---

## Поточний checkpoint — 2026-09-07

**Статус:** ТЗ у роботі. Загальний прогрес: **46%**. Залишилось приблизно **54%**.

### Зроблено / source-closed
- пункт 1/10: world startup не випускає гравця до готовності canonical stadium + landmark chain; historical delayed timers для Museum/Silpo/Culture скасовуються, всі 13 landmark stages виконуються до `GAME_RECOVERY_WORLD_READY`;
- пункт 1: deployment/loading UI починає фактичний прогрес з **0%** і відпускає гравця тільки після `bWorldReady`;
- пункт 2: player respawn delay зафіксований у `AOCGameMode` як незмінне правило **10.0 секунд**, а не Blueprint/default параметр, який можна тихо перевизначити;
- пункт 2/10: додано validation-only `UOCRespawnRecoveryValidationSubsystem`: після зміни лічильника смертей він очікує живого нового `AOCCharacter` біля 10-секундної межі та пише `GAME_RECOVERY_RESPAWN_READY` або явний `GAME_RECOVERY_RESPAWN_FAIL`; сам subsystem не spawn/possess і не створює другу respawn-логіку;
- пункт 2: input recovery source-side скидає накопичені `SetIgnoreMoveInput/SetIgnoreLookInput` стани перед gameplay; у `DefaultInput.ini` немає окремої прив’язки Gameplay Debugger;
- пункт 3: grenade mesh/material/VFX/audio preload виконується async до deployment release; first-use blocking loads прибрані, smoke/frag presentation та cleanup source-closed;
- пункт 4: sandbox weapon arsenal стабілізований, physics dropping/helper/basic-shape visuals retired source-side;
- пункт 4: один current runtime owner перевіряє повний **23-позиційний weapon catalog**; launcher, Pass8 і strict runtime evidence вимагають `PASS45_COMPLETE_WEAPON_CATALOG_VISUAL_READY` та відхиляють `PASS45_COMPLETE_WEAPON_CATALOG_VISUAL_GAP`;
- пункт 4: Remington 870 production wiring verifier актуалізований під current UE 5.8 `BY_SKELETON` import contract і поточний batch runtime route; вимоги до single skeletal weapon, pump animation та fresh-load не послаблені;
- пункт 4: manual-action audio provenance verifier актуалізований під compact canonical TZ; pinned donor/provenance/LFS/fail-closed audio вимоги збережені;
- пункт 5/10: production BTR-4, HMMWV/M2/gun-truck presentation переведені на preload/`ResolveObject()` без runtime `LoadObject()`; старі primitive/proxy visuals fail-closed;
- пункт 5/10: `OCProductionVehicleRuntimeValidationSubsystem` більше не робить чотири відкладені `LoadObject()` через 6.25 с після старту; валідатор читає тільки resident assets через `FSoftObjectPath::ResolveObject()` і при preload-gap пише явний `GAME_RECOVERY_VEHICLE_VALIDATION_PRELOAD_GAP` замість прихованого disk-load hitch;
- пункт 6/10: canonical stadium async-preload і readiness gate source-closed;
- пункт 6/10: Museum R13.8/R14.0/R14.2/R14.3/R14.4/R14.5 переведені на resident assets через `ResolveObject()` без blocking package load;
- пункт 6/10: Silpo R14.0/R14.1/R14.2/R14.3 переведені на resident assets через `ResolveObject()`; Culture House R14.6 також більше не використовує `LoadObject`;
- shared pre-spawn landmark preload розширено до **29 exact assets** і зберігається resident до `Deinitialize`;
- `VERIFY_GAME_RECOVERY_STADIUM_PRELOAD.py` захищає Stadium + Museum + Silpo + Culture House від повернення blocking `LoadObject`;
- пункт 10: Block0 foliage тепер після async preload використовує тільки resident meshes через `FSoftObjectPath::ResolveObject()`; старий post-preload `LoadObject()` прибраний, preload-gap пишеться явно, а source contract забороняє повернення sync package load;
- пункт 10: `OCRegionalGroundDetailSubsystem` тепер async-preload-ить `SM_DeadLeaves`, зберігає asset resident, використовує `ResolveObject()` замість post-spawn `LoadObject()` і робить один bounded deferred population pass замість повторного polling timer; exact source verifier на `790fdf39...` зелений;
- пункт 10: додані точні runtime timing logs для кожного landmark stage, загального часу підготовки та найповільнішого stage; stage понад 100 ms окремо позначається warning;
- центральні Pass7 / main launcher / strict harness source-перевірки синхронізовані з поточною loading/world-ready логікою та повним 23-позиційним weapon catalog, без повернення старої 11-class rack логіки;
- Pass7 захищає фіксовані 10 секунд respawn, factual respawn `READY/FAIL` probe та заборону delayed vehicle `LoadObject`;
- застарілі Pass4/Pass22/Pass3 verifier-и оновлені під current source-recovery wording, поточний batch runtime wrapper chain та актуальний async/resident foliage flow замість вимог до вже видалених старих функцій/назв;
- пункт 8: SettingsPanel source-side enabled/visible; вкладки, video/audio/FOV/HUD/accessibility/rebind та Apply/Save/Cancel/Defaults мають фактичні backend handlers; production styling присутній;
- пункт 8: `ЗАСТОСУВАТИ` тепер застосовує video/audio/player settings без permanent save і лишає екран відкритим; `ЗБЕРЕГТИ Й НАЗАД` застосовує, зберігає і закриває; `СКАСУВАТИ` перечитує останні збережені значення та повторно застосовує їх у runtime; `СКИНУТИ НАЛАШТУВАННЯ` лишається staged-only до Apply/Save;
- пункт 8: додані runtime markers `GAME_RECOVERY_SETTINGS_COMMIT` і `GAME_RECOVERY_SETTINGS_CANCEL` для фактичної UE 5.8 перевірки семантики кнопок;
- одноразовий workflow, який використано лише для безпечного точкового патчу великого UI-файла, після застосування видалений з гілки;
- source checkpoint HEAD перед цим docs-комітом: `790fdf39d4263c5294b5c9d37c56bb8fb6ad48f3`;
- PR #94 залишається **OPEN / UNMERGED**, `main` не чіпається.

### У роботі
- exact-head CI після regional-ground resident-only closure ще частково queued/in-progress; зелений статус наперед не заявляється;
- пункт 1/10: за фактичними UE 5.8 timing logs визначити, чи є stage, який сам займає >100 ms або тим більше >1 s, і такий stage розкласти на менші частини по кадрах;
- пункт 2: у фактичному UE 5.8 отримати `GAME_RECOVERY_RESPAWN_READY` після смерті та підтвердити HUD/input; будь-який `GAME_RECOVERY_RESPAWN_FAIL` є блокером;
- пункт 4: дочистити лише актуальні CI/runtime gaps по повному 23-позиційному каталогу, Remington/manual-action audio без повернення старої 11-class rack логіки;
- пункт 6: після source closure потрібна фактична перевірка в UE 5.8, що стадіон, музей, Сільпо і Будинок культури реально видимі й не з'являються після spawn;
- пункт 8: у фактичному UE 5.8 перевірити всі settings controls і підтвердити markers для Apply/Save/Cancel/Defaults; source-side семантика Apply проти Save вже розведена.

### Ще не ACCEPTED
- фактичний UE 5.8 first spawn без >1 с freeze/pop-in та responsive Alt+Tab/minimize/maximize;
- `death -> 10 s -> respawn` із HUD/input і фактичним `GAME_RECOVERY_RESPAWN_READY`;
- перший і повторний grenade throw/explosion у rendered runtime;
- rendered повний 23-позиційний weapon catalog без helper/proxy/detached parts;
- production BTR-4/HMMWV/M2 у фактичній грі;
- stadium/museum/Silpo/Culture House/карта Остер у фактичній грі;
- UI/settings interaction, HUD та весь пункт 11;
- пакетний runtime `START_HERE.cmd -> 2`.

### Наступний пункт
Пункт 1/8/10: перевірити exact-head CI як source regression signal, далі продовжити аудит актуальних first-use/runtime paths і Settings interaction без повернення старих verifier assumptions. У локальному UE 5.8 ключові факти: `GAME_RECOVERY_WORLD_PREP_STAGE_TIMING`, `GAME_RECOVERY_RESPAWN_READY`, `GAME_RECOVERY_SETTINGS_COMMIT`, `GAME_RECOVERY_SETTINGS_CANCEL`, відсутність `GAME_RECOVERY_VEHICLE_VALIDATION_PRELOAD_GAP`/`GAME_RECOVERY_FOLIAGE_PRELOAD_GAP`/`GAME_RECOVERY_REGIONAL_GROUND_PRELOAD_FAIL`, стабільний HUD та робочі Settings.