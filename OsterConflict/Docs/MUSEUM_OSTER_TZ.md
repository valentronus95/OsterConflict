# OSTER CONFLICT — ТЗ: Музей Остра

## 0. Робоча гілка

Робота над музеєм ведеться **тільки** в окремій гілці:

`museum-oster`

Базовий стан гілки: актуальний `main` на момент створення, commit `fbe66f7502f0cf6ecc621bba575c1e1b35e7e76b`.

Не змішувати цю роботу з `stadion-oster`, іншими локаціями, зброєю, ботами, UI або загальними gameplay-змінами, якщо вони не потрібні безпосередньо для музею.

## 1. Мета

Створити максимально впізнавану, фото-орієнтовану 3D-модель музею в Острі та прилеглої території, інтегровану у фактичну географічну точку на карті Oster Conflict.

Музей має бути не декоративним фасадом, а повноцінним gameplay-об'єктом:

- до будівлі можна підійти з усіх доступних боків;
- у будівлю можна зайти;
- двері відкриваються/закриваються;
- вікна мають реальне скло;
- скло має розбиватися від пострілів;
- внутрішні приміщення мають collision і придатні для FPS-переміщення;
- модель одразу проєктується так, щоб пізніше підтримувати пошкодження і руйнування від RPG, гранат, вибухів та іншої важкої зброї.

## 2. Верифікована географічна прив'язка

Поточний код уже має `MuseumAnchor()` через `FOCGeoReference::Museum()`.

Верифікована точка:

- назва: `MuseumSolonyna`;
- координати: `50.948239, 30.883865`;
- адресний орієнтир: Татарівська 30;
- confidence у коді: `A`.

Нову модель не переносити в довільне місце. Вона повинна залишатися прив'язаною до цього anchor, а орієнтація, відступи, двір і рослинність уточнюються за фотографіями.

## 3. Що вже готово в `main`

### 3.1. Географія

Є `AOCWorldSectorOster::MuseumAnchor()` і геоприв'язка музею.

### 3.2. R13.7 photo-model

Є `UOCR137MuseumPhotoModelSubsystem`, який:

- запускається на `OsterConflict_Runtime`;
- прибирає попередню музейну заглушку;
- створює окремий actor з тегом `R137_MuseumPhotoModel`;
- відтворює основний силует будівлі;
- має цегляний корпус, цоколь, дах, дерев'яні секції, прибудову, димар, карнизи, декоративні елементи, вікна, решітки, двері, сходи, газову трубу та частину рослинності;
- використовує фото-пропорції, але не геодезичні заміри;
- поточні приблизні зовнішні габарити в коді: близько `17.6 м x 9.0 м`.

Поточна реалізація є доброю геометричною основою, але **не фінальною музейною моделлю**.

### 3.3. Cleanup старої геометрії

Є `UOCR137MuseumSiteReplacementSubsystem`, який перед побудовою нової моделі:

- ховає legacy-компоненти музею;
- прибирає старі landmark instances у зоні музею;
- прибирає старі паркани в зоні музею;
- прибирає primitive/decorator vegetation, яка конфліктує з новою моделлю.

Цю логіку зберегти, але радіуси cleanup перевірити після точного відтворення території.

### 3.4. Runtime validation

Є `UOCR137MuseumRuntimeValidationSubsystem`, який перевіряє:

- що фінальний музейний actor існує один;
- що в ньому є компоненти та instances;
- що є collision;
- що модель знаходиться біля museum anchor;
- що legacy/source residue не залишилось.

Validation потрібно розширити для нової інтерактивної версії музею.

### 3.5. Уже наявні reusable gameplay-системи

У проєкті вже є:

- `AOCInteractableDoor` — replicated двері з відкриванням/закриванням, collision та door audio;
- `AOCBreakableWindow` — replicated стан розбитого скла, collision до розбиття, локальні фізичні уламки та звук;
- `AOCDestructibleProp` — replicated durability/destroyed state та локальні уламки;
- `AOCEnterableHouse` — приклад будівлі з реальними дверними/віконними отворами, interior collision і spawning інтерактивних opening actors.

Не створювати дублікати цих систем без необхідності. Для музею або повторно використати їх, або зробити музейні спеціалізації поверх спільних базових механік.

## 4. Головні проблеми поточної музейної реалізації

На поточному етапі:

- значна частина корпусу побудована з `/Engine/BasicShapes/Cube`;
- немає повноцінного walkable interior;
- поточні музейні двері є статичними ISM-елементами, а не `AOCInteractableDoor`;
- поточне музейне скло є статичним ISM-компонентом, а не `AOCBreakableWindow`;
- скло не має gameplay break state;
- немає системи структурних секцій будівлі;
- немає damage states для стін/даху/прибудов;
- немає логіки руйнування будівлі вибухом;
- приблизні пропорції треба повторно звірити з новим набором фотографій;
- територію навколо музею потрібно відтворити за фото, а не generic vegetation/layout.

## 5. Фото-орієнтоване моделювання

Після отримання нового набору фотографій:

1. Розкласти фото за сторонами будівлі.
2. Визначити головний фасад, тил, лівий/правий торці, прибудови.
3. Звірити:
   - кількість вікон;
   - розміри та пропорції прорізів;
   - тип і колір рам;
   - двері;
   - дах;
   - карнизи;
   - дерев'яні секції;
   - цегляні декоративні елементи;
   - димар;
   - газову трубу;
   - фундамент/цоколь;
   - сходи;
   - решітки;
   - водостоки;
   - дрібні фасадні деталі.
4. Встановити єдиний масштаб за кількома надійними людськими/архітектурними орієнтирами.
5. Не вигадувати декоративні деталі, яких не видно на фото.
6. Невідомі частини позначати в коді/документації як inferred, а не verified.

## 6. Архітектура фінальної 3D-моделі

Фінальний музей **не робити одним монолітним mesh**.

Будівлю розбити на логічні модулі:

- фундамент/цоколь;
- зовнішні стіни по секціях;
- кути;
- фронтони;
- дерев'яні верхні секції;
- основні roof sections;
- veranda/vestibule;
- rear annex;
- chimney;
- interior walls;
- floors;
- ceilings;
- door frames;
- window frames;
- glass panes;
- grilles;
- stairs/porch;
- utility details.

Кожна важлива structural section повинна мати стабільний ID/роль, щоб її можна було окремо пошкоджувати в майбутньому.

Використовувати реальні mesh/material assets там, де вони підходять. `/Engine/BasicShapes/*` допустимі лише як тимчасовий authoring/prototype fallback, але не як фінальна видима геометрія.

## 7. Інтер'єр і прохідність

Музей повинен бути enterable.

Обов'язково:

- реальні дверні отвори в shell geometry;
- реальні віконні отвори, а не скло поверх суцільної стіни;
- floor collision;
- wall collision;
- stair/porch collision;
- player capsule не повинен чіплятися за дрібний декор;
- дверні проходи мають бути достатні для FPS movement;
- не повинно бути invisible walls у місцях, де візуально є прохід;
- внутрішні двері, якщо вони підтверджені фото/планом, теж інтерактивні.

Якщо точного плану інтер'єру немає, створювати тільки консервативний gameplay-layout, який не суперечить зовнішній геометрії. Не видавати вигаданий інтер'єр за історично точний.

## 8. Двері

Зовнішні та потрібні внутрішні двері реалізувати через `AOCInteractableDoor` або музейний subclass на його основі.

Вимоги:

- server-authoritative interaction;
- replicated open/closed state;
- collision дверного полотна;
- плавне відкривання;
- правильна сторона петель;
- відкривання не повинно заштовхувати гравця в стіну;
- door open/close audio;
- reset підтримується для test mode;
- фінальна форма дверей має відповідати фото, а не prototype cube.

## 9. Вікна і скло

Кожна gameplay-доступна шибка має бути окремим breakable gameplay element.

База: `AOCBreakableWindow`.

Вимоги:

- intact glass visible;
- glass collision до руйнування;
- попадання кулі/уламка/вибуху може розбити скло;
- replicated `broken` state;
- після руйнування collision скла вимикається;
- клієнтські уламки скла є косметичними і не реплікуються як сотні physics actors;
- уламки мають зникати через контрольований час;
- frame/grille залишаються окремими від скла;
- розбиття однієї шибки не повинно автоматично прибирати всі вікна будівлі.

Поточний `R137Museum_WindowGlass` ISM потрібно замінити на реальні breakable window actors/sections.

## 10. Підготовка до руйнування будівлі

Це обов'язкова архітектурна вимога вже на етапі моделювання.

Мета: у подальшому RPG, гранати, вибухові заряди або інша важка зброя повинні пошкоджувати і частково/суттєво руйнувати музей.

### 10.1. Не реплікувати Chaos-сміття як gameplay state

Для multiplayer authoritative state повинен бути компактним.

Рекомендована модель:

- сервер зберігає стан структурних секцій;
- кожна секція має durability/health;
- radial damage від вибуху розподіляється по секціях;
- стан секції: `Intact -> Damaged -> Destroyed`;
- мережею передається structural state;
- великі collision-зміни синхронізуються;
- дрібні уламки, пил, крихти, скло і більшість Chaos debris — локальні cosmetic FX.

### 10.2. Structural sections

Мінімально розділити:

- зовнішні wall bays між прорізами;
- кути будівлі;
- timber upper section;
- vestibule/veranda;
- rear annex;
- chimney;
- roof sections;
- ключові interior partitions.

Не робити одну health-bar на всю будівлю.

### 10.3. Вибухи

Майбутній RPG/explosive hit повинен:

- створювати radial structural damage;
- сильніше пошкоджувати найближчі секції;
- розбивати nearby glass;
- пошкоджувати/знищувати слабші props;
- викликати masonry/wood/glass FX залежно від surface;
- не руйнувати всю будівлю від одного слабкого попадання, якщо damage недостатній.

### 10.4. Обвал

Для першої production-версії не потрібна повна інженерна симуляція споруди.

Потрібна керована логіка:

- якщо опорна wall section знищена, залежна roof section може перейти в damaged/collapsed state;
- при критичному сумарному пошкодженні окремої частини будівлі дозволяється локальний collapse;
- collision після collapse має відповідати видимій геометрії;
- сервер не повинен симулювати сотні уламків як replicated physics actors.

## 11. Прилегла територія

Відтворити за фотографіями:

- підхід до музею;
- доріжки/плити/ґрунт;
- сходи;
- паркани/огорожі;
- дерева;
- кущі;
- трава;
- хвоя/шишки/листя, якщо це видно на референсах;
- utility elements;
- дрібні об'єкти, що реально є на території;
- взаємне положення музею і сусідніх об'єктів.

Не використовувати випадкове озеленення лише для заповнення порожнього простору.

Територія музею може географічно межувати/перетинатися із зоною іншої окремо розроблюваної локації, але музейна гілка не повинна переробляти чужу production-модель. Спільні межі узгоджуються через anchor/terrain/path contracts.

## 12. Матеріали

Потрібні окремі surface families:

- brick/masonry;
- painted wood;
- bare/dark wood;
- metal roof;
- glass;
- painted metal;
- concrete;
- plaster/trim;
- ground/vegetation.

Матеріали повинні підтримувати правильні hit FX/audio surface mappings для майбутньої системи пошкоджень.

## 13. Collision / Navigation / FPS

Перевірити:

- підхід до всіх входів;
- doorway clearance;
- window openings після розбиття;
- interior movement;
- сходи;
- кути;
- collision даху/стін після damage state switch;
- bot navigation, якщо navmesh охоплює музей;
- відсутність stuck points;
- відсутність player clipping через shell.

## 14. Multiplayer

Будь-який gameplay state музею повинен бути authoritative на сервері:

- doors;
- broken windows;
- structural damage;
- destroyed structural sections.

Не реплікувати cosmetic debris по мережі.

Late-joining client повинен отримати актуальний стан дверей, вікон і structural damage.

## 15. Performance

Цільовий ПК не розрахований на безконтрольний AAA Chaos scene.

Тому:

- LOD/Nanite використовувати там, де доречно;
- не створювати тисячі окремих replicated actors;
- static decorative repeats можуть бути ISM/HISM;
- gameplay openings/structural sections мають бути окремими тільки там, де це потрібно;
- debris cleanup обов'язковий;
- не тримати постійно активну physics simulation для цілого музею.

## 16. Validation

Поточний `UOCR137MuseumRuntimeValidationSubsystem` розширити.

Мінімальні перевірки:

- рівно один active museum root actor;
- модель стоїть біля `MuseumAnchor`;
- legacy museum geometry відсутня;
- source residue відсутня;
- є collidable structural shell;
- є walkable floor;
- є мінімум один interactable external door;
- усі передбачені gameplay windows мають breakable actors;
- немає static fake glass поверх gameplay windows;
- немає суцільної стіни в дверному прорізі;
- після відкривання дверей прохід реально доступний;
- після розбиття скла його collision реально зникає;
- structural section IDs унікальні;
- damage state коректно реплікується в multiplayer test.

## 17. Етапи робіт

### Етап M1 — Photo audit

- прийняти новий набір фото;
- розкласти ракурси;
- створити список verified/inferred деталей;
- уточнити footprint, yaw і пропорції.

### Етап M2 — Exterior production shell

- замінити prototype cube-driven фасад на production modular geometry;
- відтворити всі видимі фасади;
- зробити реальні openings.

### Етап M3 — Territory

- відтворити прилеглу територію за фото;
- прибрати generic leftovers;
- звірити межі з іншими сусідніми локаціями.

### Етап M4 — Enterable interior

- floor/walls/ceilings;
- доступні приміщення;
- collision;
- navigation.

### Етап M5 — Interactive doors

- інтеграція `AOCInteractableDoor`;
- правильні hinges/meshes/audio/replication.

### Етап M6 — Breakable glass

- інтеграція `AOCBreakableWindow`;
- окремі шибки;
- shards/FX/audio;
- network state.

### Етап M7 — Structural damage foundation

- stable structural sections;
- durability/state model;
- radial damage contract;
- collision state changes;
- damage/collapse placeholders без важкої постійної Chaos simulation.

### Етап M8 — Runtime validation

- compile;
- automation;
- PIE/listen-server;
- late join state;
- visual screenshots з усіх фото-ракурсів;
- regression check gameplay/network baseline.

## 18. Definition of Done

Музей не вважати готовим лише тому, що код компілюється.

Статуси відрізняти:

1. `source ready`;
2. `compile verified`;
3. `runtime verified`;
4. `network verified`;
5. `visual verified against photos`;
6. `interaction verified`;
7. `damage architecture verified`.

Фінальний статус для merge у `main` можливий лише після visual/runtime/network перевірки.

## 19. Заборонено в цій гілці

- робити зміни у `stadion-oster`;
- випадково переносити роботу зі стадіону в музей;
- змінювати глобальний UI без прямої потреби музею;
- переписувати gameplay/network baseline;
- називати prototype cube model фінальною 3D-моделлю;
- merge у `main` до окремої перевірки.

## 20. Наступний вхідний пакет

Наступний великий крок починається після отримання нового набору фотографій музею.

Фотографії використовуються для повторного точного photo audit існуючої R13.7 моделі та перебудови exterior/territory/interior contracts без втрати вже готової географічної прив'язки і cleanup/validation логіки.
