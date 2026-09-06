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
- після кожного циклу коротко фіксувати: що зроблено, що лишилось, наступний пункт, % прогресу.

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
- фон, panel opacity, typography, button treatment та accent повинні відповідати головному меню;
- input fields не повинні виглядати як стандартні сірі debug controls;
- підтримати 1280x720 і вище без кривого масштабування/перекриття.

### Acceptance
- secondary screens виглядають як частина однієї гри, а не developer UI.

---

## 8. MEDIUM — HUD
### Треба
- перевірити weapon slot list, ammo, grenades, role/squad/team, interaction prompt;
- не показувати debug/fallback text як production HUD;
- HUD не губиться після death/respawn/weapon switch;
- grenade state синхронізований з inventory/server state.

---

## 9. PERFORMANCE — runtime spikes
### Треба
- знайти всі startup/first-use блокуючі `LoadObject`/sync loads у WorldSubsystem/weapon/grenade/vehicle paths;
- перенести допустиме на async preload;
- staged materialization для важких world builds;
- не виконувати великі цикли spawn/register components в один кадр, якщо це викликає hitch;
- додати компактні runtime timing logs для startup, grenade first-use, respawn та landmark build.

### Acceptance
- немає 5–20 секундних зависань у звичайному gameplay flow.

---

## 10. FINAL UE 5.8 ACCEPTANCE
Окремо перевірити на поточному exact HEAD:
1. запуск через `START_HERE.cmd` -> `1`;
2. створення сервера;
3. deployment/team/group/role/spawn;
4. перший spawn;
5. усі weapon pickups;
6. grenade throw/explosion;
7. death -> 10 s respawn;
8. БТР-4 production visual;
9. стадіон/музей/основна карта;
10. Alt+Tab/minimize/maximize/close;
11. HUD після respawn;
12. немає Gameplay Debugger/debug shapes;
13. після цього пакетний runtime test через `START_HERE.cmd` -> `2`.

## Definition of Done
ТЗ закрите тільки коли ці пункти підтверджені фактичним UE 5.8 runtime, а не лише компіляцією/CI/source inspection.
