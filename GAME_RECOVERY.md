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

**Статус:** ТЗ у роботі. Загальний прогрес: **52%**. Залишилось приблизно **48%**.

### Зроблено / source-closed
- пункт 1/10: canonical Stadium + Museum + Silpo + Culture House готуються до spawn; historical delayed landmark timers прибрані з post-spawn шляху, loading UI чекає фактичний `WORLD_READY`;
- пункт 1/10: усі 4 знайдені park upgrade-власники (`ParkBenches`, ground, hardscape, memorial approach) переведені з post-spawn delay/`LoadObject()` на async/resident підготовку до spawn; Park Ground/Hardscape/Memorial/Semantic source gates зелені;
- пункт 1/10: stadium assets preload-яться async, а сама `ApplyStadiumSurface()` тепер має окремий `GAME_RECOVERY_STADIUM_MATERIALIZATION_TIMING` з warning budget 100 ms, щоб фактичний UE 5.8 показав, чи треба дробити геометрію по кадрах;
- пункт 2: server respawn зафіксований на **10 секунд**; confirmed death переходить у corpse-camera wait без spectator/debug pawn; input/menu stacks блокуються на час смерті та відновлюються після нового possession;
- пункт 2/9: додано production respawn HUD countdown `10 -> 0`; overlay прибирається при respawn та `EndPlay`, без `GameplayDebugger` або debug-text; `Game Recovery Respawn` source gate зелений на `c82e626bfc4e7f9f3db0e5808d269bff0786bf12`;
- пункт 3/9: grenade mesh/material/VFX/audio preload, HUD type/count, one-shot explosion і cleanup source-closed; відповідні grenade gates зелені на останніх перевірених exact heads;
- пункт 4: повний **23-позиційний weapon catalog** та primitive/proxy retirement source-side захищені; base weapon startup більше не будує декоративні Cube/Cylinder/material proxy; local imported bridge після production visual ховає старий source proxy, але не видаляє physics root;
- пункт 4/10: `OCLocalInboxWeaponOverrideSubsystem` більше не робить `LoadObject()` у normal gameplay: missing weapon asset вантажиться через `RequestAsyncLoad`, потім ставиться тільки через resident `ResolveObject()`; visual кріпиться до unscaled weapon root і отримує production tag, тому дві видимі моделі не повинні конкурувати;
- пункт 5/10: BTR-4, HMMWV/M2 та інші production vehicle source paths переведені на preload/resident assets, primitive/proxy fallback fail-closed;
- пункт 6/10: Stadium/Museum/Silpo/Culture House blocking landmark loads source-side прибрані; shared landmark preload утримує потрібні assets resident;
- пункт 8: Settings handlers для tabs/video/audio/FOV/HUD/accessibility/rebind та Apply/Save/Cancel/Defaults source-side присутні; `GAME_RECOVERY_SETTINGS_COMMIT` / `GAME_RECOVERY_SETTINGS_CANCEL` лишаються factual runtime markers;
- пункт 10: landmark stage timing, stadium materialization timing, foliage/regional-ground resident-only paths, local weapon async path та respawn lifecycle timing/log markers захищені source checks;
- exact source HEAD перед цим docs-комітом: `c82e626bfc4e7f9f3db0e5808d269bff0786bf12`;
- новий `Game Recovery Respawn` gate на `c82e...`: **SUCCESS**; ширший набір exact-head workflow після respawn HUD зміни ще виконується;
- PR #94: **OPEN / UNMERGED**, base `main` = `a1ad0e200611911102c48180956d82f73d0d8fc3`; merge не дозволений до factual UE 5.8 acceptance.

### У роботі
- пункт 1/10: у фактичному UE 5.8 зняти `GAME_RECOVERY_WORLD_PREP_STAGE_TIMING` і `GAME_RECOVERY_STADIUM_MATERIALIZATION_TIMING`; дробити лише реально over-budget stage/materialization, а не навмання;
- пункт 2/9: фактично пройти `death -> 10 s -> respawn`, побачити countdown і підтвердити HUD/input після нового pawn;
- пункт 4: перевірити rendered 23-позиційний каталог після local weapon async/production-owner cutover, без синіх дисків, proxy і подвійних моделей;
- пункт 6: фактично перевірити Stadium/Museum/Silpo/Culture House/park без post-spawn pop-in;
- пункт 8/9: фактично пройти Settings + HUD у UE 5.8;
- пункт 10: продовжити аудит лише реальних normal-game first-use paths; validation-only `LocalInboxWorldAssets` не вважається normal gameplay blocker.

### Ще не ACCEPTED
- фактичний UE 5.8 first spawn без >1 с freeze/pop-in та responsive Alt+Tab/minimize/maximize;
- `death -> 10 s -> respawn` із видимим countdown та стабільним HUD/input;
- перший і повторний grenade throw/explosion у rendered runtime;
- rendered повний weapon catalog без helper/proxy/detached/double visuals;
- production BTR-4/HMMWV/M2 у фактичній грі;
- Stadium/Museum/Silpo/Culture House/park/карта Остер у фактичній грі;
- Settings, HUD та весь пункт 11;
- пакетний runtime `START_HERE.cmd -> 2`.

### Наступний пункт
Пункт 1/4/10: продовжити normal-game first-use audit, а в UE 5.8 головні факти зараз: `GAME_RECOVERY_WORLD_PREP_STAGE_TIMING`, `GAME_RECOVERY_STADIUM_MATERIALIZATION_TIMING`, `GAME_RECOVERY_LOCAL_WEAPON_PRELOAD_*`, `GAME_RECOVERY_RESPAWN_HUD_READY`, `GAME_RECOVERY_RESPAWN_CLIENT_READY`, `GAME_RECOVERY_SETTINGS_COMMIT`/`CANCEL`, стабільний HUD та відсутність post-spawn proxy/pop-in.
