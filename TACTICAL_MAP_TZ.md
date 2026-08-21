# ТЗ: Tactical Map 2.0 — Oster Conflict

**Ticket:** `UI-TACTICAL-MAP-001`  
**Branch:** `feat/tactical-map-v2`  
**Engine:** Unreal Engine 5.8  
**Status:** IMPLEMENTATION STARTED / RUNTIME NOT VERIFIED

## 1. Мета

Замінити стару примітивну/нестабільну карту, яка відкривалась клавішею `M`, на повноцінну тактичну карту Oster Conflict. Карта має відображати фактичну геометрію і розташування об'єктів ігрового світу, синхронізуватися з world-space координатами та бути придатною для подальшої роботи з гравцями, загоном, транспортом, цілями й маркерами.

Головний принцип: **джерело істини — фактичний level/world Oster Conflict.** Зовнішні карти, AI-концепти, Google/Apple/OSM та старі ручні координати не визначають положення об'єктів у Tactical Map.

## 2. Вихідні дані та обов'язкові виправлення

Візуальний концепт, погоджений у чаті, використовується тільки як **UI/style reference**:

- темний charcoal HUD;
- велика карта в центральній/правій частині;
- ліва панель легенди;
- координатна сітка;
- компас / North indicator;
- нижня смуга керування;
- зелені маркери гравця;
- сині/блакитні маркери загону та транспорту;
- помаранчеві маркери цілей;
- нейтральні POI;
- мінімум декоративного шуму.

AI-згенерована географія **не використовується**.

Відомі корекції, які мають бути враховані при перевірці world layout:

- музей знаходиться біля стадіону;
- `Сільпо` знаходиться в центральній частині біля водонапірної башти;
- парк у попередньому концепті був приблизно в правильній зоні;
- не додавати вигадану річку через місто;
- не додавати вигадані мости; реальні/ігрові мости знаходяться значно далі й мають братися тільки з level/world;
- будь-яка розбіжність між старою документацією та фактичним level вирішується на користь фактичного level.

## 3. Поточна проблема

1. Старий Tactical Map раніше існував і відкривався через `M`.
2. Після попередніх змін карта перестала стабільно працювати/зникла.
3. У ledger попередній source-fix позначений як `CODED_UNTESTED`, тому він не вважається підтвердженим runtime-виправленням.
4. Поточну реалізацію необхідно проаудитити до заміни, щоб не залишити паралельні toggle/input/widget шляхи.

## 4. Етап 1 — відновлення клавіші M

Необхідно знайти та перевірити повний ланцюжок відкриття карти:

`M / Enhanced Input -> PlayerController/HUD -> Tactical Map state -> Widget creation -> AddToViewport -> Input mode -> close/toggle`

Перевірити:

- Input Mapping Context;
- Input Action або прямий key binding;
- Enhanced Input registration;
- PlayerController / Character / HUD binding;
- наявність та валідність widget class;
- `CreateWidget`;
- `AddToViewport` / `RemoveFromParent`;
- повторне відкриття після закриття;
- `SetInputModeGameAndUI` / `SetInputModeUIOnly` / `SetInputModeGameOnly`;
- mouse cursor state;
- focus;
- pause state, якщо використовується;
- конфлікти з Escape/menu/chat/inventory;
- дубльовані старі toggle handlers;
- lifecycle після respawn / level travel / possession.

### Acceptance

- перше натискання `M` відкриває карту;
- друге натискання `M` закриває карту;
- цикл можна повторювати без зникнення input;
- після закриття повертається нормальне керування персонажем;
- карта не закривається сама через refresh/tick/state race;
- поведінка однакова після respawn/повторного possession.

## 5. Джерело зображення карти

### 5.1 Заборонено

- вручну малювати місто по пам'яті;
- використовувати AI-карту як географічну основу;
- вручну ставити POI в screen-space координатах;
- прив'язувати marker positions до пікселів конкретної PNG;
- вбудовувати Google/Apple screenshot як production texture.

### 5.2 Правильний pipeline

Статичний фон Tactical Map має бути отриманий з **актуального ігрового level**:

1. визначити bounds playable area;
2. визначити орієнтацію world axes;
3. створити top-down orthographic representation актуального level;
4. зафіксувати `WorldMin`, `WorldMax`, rotation/orientation та texture aspect ratio;
5. використовувати ці самі bounds для world-to-map transform;
6. при зміні геометрії міста фон можна регенерувати без переписування marker logic.

Перевага надається editor/offline capture, а не постійному дорогому runtime SceneCapture, якщо runtime capture не потрібний функціонально.

## 6. Coordinate Mapping

Усі динамічні об'єкти мають використовувати world-space координати.

Базова нормалізація:

```text
U = (WorldX - MinX) / (MaxX - MinX)
V = (WorldY - MinY) / (MaxY - MinY)
```

Залежно від фактичної орієнтації texture/world допускаються:

- інверсія `V`;
- swap X/Y;
- rotation offset;
- configurable yaw correction.

Після нормалізації:

```text
MapX = U * MapWidth
MapY = V * MapHeight
```

Трансформація повинна бути централізована в одному map subsystem/manager/helper, а не дублюватися у кожному widget marker.

## 7. Архітектура UI

Цільова логічна структура:

```text
TacticalMapRoot
 ├─ MapViewport
 │   ├─ MapBackground
 │   ├─ MapGrid
 │   ├─ POILayer
 │   ├─ ObjectiveLayer
 │   ├─ VehicleMarkerLayer
 │   ├─ SquadMarkerLayer
 │   ├─ PingLayer
 │   └─ PlayerMarkerLayer
 ├─ MapLegend
 ├─ MapHeader
 ├─ Compass
 ├─ ScaleBar
 └─ ControlsHint
```

Назви конкретних C++/Blueprint classes повинні узгоджуватися з уже наявною архітектурою проєкту. Не створювати дубль існуючого manager/subsystem, якщо в проєкті вже є відповідний owner.

## 8. Map viewport

Потрібно реалізувати:

- map clipping у межах viewport;
- zoom in/out;
- pan drag;
- clamp, щоб карту не можна було повністю вивести за viewport;
- marker scaling policy;
- коректне позиціонування marker при zoom/pan;
- опціональне центрування на player;
- reset/focus on player;
- стабільну роботу при різних aspect ratio.

### Керування

Мінімальний набір:

- `M` — відкрити/закрити;
- wheel — zoom;
- LMB drag — pan;
- RMB — поставити tactical ping/marker;
- окрема кнопка/дія — focus player, якщо це не перевантажує UI.

## 9. Player marker

Маркер гравця:

- зелений;
- завжди отримує позицію з фактичного pawn/character;
- має direction/yaw indicator;
- не використовує статичний screen coordinate;
- оновлюється плавно;
- коректно працює після зміни pawn/respawn.

## 10. Squad markers

Маркер союзника:

- синій/блакитний;
- позиція з authoritative gameplay/network state;
- підтримка кількох членів загону;
- відсутній/прихований для невалідного або нерелевантного actor;
- не створювати widget заново кожен tick без потреби;
- marker pool/cache бажаний при великій кількості actors.

## 11. Vehicle markers

Транспорт:

- синій/блакитний icon;
- world-space позиція;
- тип icon може залежати від vehicle class;
- підтримати HMMWV/BTR та майбутні vehicle classes без hardcoded screen positions.

## 12. POI

POI повинні визначатися world-space anchor-ами або даними, що напряму відповідають level actors.

Перший набір:

- Музей;
- Стадіон;
- Парк;
- Сільпо;
- Центр;
- Автостанція;
- Водонапірна башта як орієнтир/POI, якщо вона вже присутня в level і потрібна gameplay/UI.

POI data мінімально містять:

```text
Id
DisplayName
WorldLocation
Category
Icon
Visibility
OptionalGameplayTag
```

Положення POI на Tactical Map генерується через world-to-map transform.

## 13. Objectives

Objective markers:

- помаранчеві/amber;
- підтримка IDs (`A`, `B`, `C`, ...);
- world-space location;
- state: neutral/active/completed/contested за потреби gameplay;
- UI не повинен залежати від конкретної кількості objectives.

## 14. Tactical Ping

RMB ping:

1. конвертувати local map coordinate у normalized UV;
2. конвертувати UV назад у world-space X/Y;
3. визначити допустиму world position;
4. створити ping data/event;
5. відобразити ping через `PingLayer`;
6. у multiplayer пізніше реплікувати тільки необхідні data, а не widget.

У першій ітерації допускається локальний ping до завершення multiplayer integration, але API не має блокувати майбутню replication.

## 15. Візуальний стиль

### Загальний вигляд

- full-screen tactical overlay;
- темний напівпрозорий HUD frame;
- карта займає більшу частину екрана;
- sidebar не перекриває ключову карту;
- professional military UI без копіювання конкретної комерційної гри.

### Header

```text
TACTICAL MAP
OSTER CONFLICT
```

Допускається локалізація заголовка пізніше.

### Legend

```text
ЛЕГЕНДА
ГРАВЕЦЬ
ЧЛЕНИ ЗАГОНУ
ТРАНСПОРТ
ЦІЛЬ
ТОЧКА ІНТЕРЕСУ
```

### Grid

- літери по X (`A-J` або динамічно за aspect/scale);
- цифри по Y (`1-10` або динамічно);
- grid має бути overlay і не змінювати фактичну географію;
- grid coordinate може надалі використовуватися для squad callouts.

### Bottom hints

```text
M — ЗАКРИТИ МАПУ
КОЛЕСО МИШІ — ЗМІНИТИ МАСШТАБ
ЛКМ + ПЕРЕТЯГНУТИ — ПЕРЕМІСТИТИ
ПКМ — ПОСТАВИТИ МАРКЕР
```

## 16. North / orientation

North indicator не можна ставити декоративно.

Потрібно:

- визначити, де фактична північ у level/world;
- зафіксувати orientation metadata;
- map texture і world-to-map transform мають використовувати одну орієнтацію;
- player yaw marker повинен бути узгоджений з цією орієнтацією.

## 17. Scale bar

Scale bar має розраховуватись із world bounds, а не бути намальованим фіксованим числом.

Якщо `1 Unreal Unit = 1 cm`, переводити world distance у метри стандартним способом і показувати scale відповідно до поточного zoom.

## 18. Multiplayer readiness

UI не реплікується.

Реплікуються/отримуються gameplay data:

- player/squad actor state;
- vehicle state;
- objective state;
- tactical ping data.

Tactical Map лише візуалізує ці дані локально.

Не створювати networking dependency для статичних POI.

## 19. Performance

- не робити дорогий full-map SceneCapture кожен frame без обґрунтованої потреби;
- static background texture бажана для першої production ітерації;
- marker updates можуть бути throttled, якщо не погіршується сприйняття;
- не алокувати widgets/arrays кожен tick;
- map closed = мінімальна/нульова UI update cost;
- підтримати 1080p/1440p/4K без прив'язки до конкретної pixel resolution.

## 20. State ownership

Потрібен один authoritative UI state owner для `IsTacticalMapOpen`.

Не допускаються паралельні незалежні bool у Character + HUD + Widget, які можуть розсинхронізуватись.

Open/close повинен бути idempotent:

- `OpenMap()` при already-open не створює другий widget;
- `CloseMap()` при already-closed не ламає input;
- `ToggleMap()` викликає один із двох шляхів.

## 21. Legacy cleanup

Після визначення нового owner:

- знайти старі map widget classes;
- знайти старі bindings `M`;
- знайти hardcoded POI positions;
- знайти obsolete textures/config;
- прибрати або ізолювати legacy path;
- не залишати два активні способи відкриття Tactical Map.

Binary assets видаляти/замінювати лише після підтвердження references.

## 22. Implementation plan

### Phase A — Audit & restore

1. знайти current input binding;
2. знайти current Tactical Map widget/owner;
3. відновити стабільний `M` toggle;
4. додати/оновити source tests;
5. runtime verification в UE 5.8.

### Phase B — Mapping foundation

1. map bounds data;
2. world-to-map / map-to-world transform;
3. orientation;
4. player marker;
5. automated tests math/edge cases.

### Phase C — Actual level background

1. визначити playable bounds;
2. створити top-down representation з актуального level;
3. синхронізувати texture bounds;
4. перевірити музей/стадіон/Сільпо/парк/інші POI по level.

### Phase D — Tactical layers

1. POI;
2. squad;
3. vehicles;
4. objectives;
5. pings.

### Phase E — UX

1. zoom;
2. pan;
3. focus player;
4. grid;
5. scale bar;
6. legend;
7. controls hints;
8. responsive layout.

### Phase F — Multiplayer/runtime validation

1. listen server;
2. dedicated server + clients;
3. respawn;
4. possession;
5. level travel;
6. repeated M toggle;
7. marker synchronization;
8. performance.

## 23. Automated tests

Мінімум для source layer:

- world min -> `(0, 0)`;
- world max -> `(1, 1)` з урахуванням orientation;
- center -> `(0.5, 0.5)`;
- map-to-world round trip;
- bounds clamp;
- yaw conversion;
- zoom transform;
- invalid/zero-size bounds fail safely;
- open/close state idempotence, якщо architecture дозволяє unit/automation test.

## 24. Runtime acceptance criteria

Статус `VERIFIED_RUNTIME` можна ставити тільки після фактичної перевірки в UE 5.8.

Обов'язково перевірити:

1. запуск game/PIE без crash;
2. `M` відкриває карту;
3. `M` закриває карту;
4. 20+ циклів open/close без втрати input;
5. player marker стоїть у правильній точці level;
6. player marker рухається в правильному напрямку;
7. marker yaw відповідає напрямку персонажа;
8. музей відображається біля стадіону відповідно до фактичного level;
9. `Сільпо` відображається в центрі біля водонапірної башти відповідно до level;
10. парк відповідає фактичній позиції;
11. відсутня вигадана річка/вигадані мости;
12. zoom/pan не ламають marker alignment;
13. 16:9 та інші підтримувані aspect ratio не зміщують markers;
14. після respawn карта працює;
15. multiplayer markers не створюють дублікати.

## 25. Git strategy

Уся робота ведеться в:

```text
feat/tactical-map-v2
```

Порядок:

1. ТЗ у корені;
2. audit/restore commit;
3. mapping foundation commit;
4. UI/layout commit;
5. POI/marker integration commit;
6. tests/fixes;
7. підтягнути актуальний `main`;
8. regression/runtime validation;
9. merge у `main` тільки після перевірки.

## 26. Definition of Done

Tactical Map 2.0 вважається завершеною, коли:

- `M` стабільно працює;
- географія відповідає actual Oster Conflict level;
- карта не містить вручну вигаданих water/road/bridge/POI placements;
- player/world markers використовують єдину world-to-map transform;
- POI синхронізовані з level;
- zoom/pan/grid/legend/scale працюють;
- multiplayer data path не залежить від widget replication;
- automated source tests проходять;
- UE 5.8 runtime validation пройдена;
- ledger оновлений;
- гілка готова до merge без втрати актуальних змін `main`.

## 27. Out of scope першої ітерації

До окремого етапу можна відкласти:

- fog of war;
- heatmaps;
- commander mode;
- live UAV feed;
- складні drawing tools;
- persistence user markers між сесіями;
- зовнішні online map APIs;
- автоматичний routing/navigation.

Ці функції не повинні блокувати базову стабільну Tactical Map 2.0.