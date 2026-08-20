# OSTER CONFLICT — ТЗ: СІЛЬПО, ОСТЕР

Status: active implementation
Branch: `silpo-oster`
Location: м. Остер, Чернігівська область, вул. Богдана Хмельницького, 54
Scope owner: Silpo location only

## 1. Мета

Створити окрему впізнавану, прохідну та географічно прив'язану 3D-реконструкцію остерського «Сільпо» з прилеглою ділянкою. Перший етап має дати правильну коробку будівлі, фасад, силует, вхід, колізію та базовий інтер'єр. Наповнення товаром, дрібний декор і брендова поліграфія виконуються окремим наступним проходом.

Модель не повинна перетворюватися на типовий сучасний магазин «за мотивами». Поточні фото конкретного остерського об'єкта є головним візуальним джерелом.

## 2. Ізоляція гілки

Уся робота цього ТЗ виконується тільки в `silpo-oster`.

Попередній набір фотографій музею не належить до цього ТЗ, не є джерелом для «Сільпо» і не повинен потрапляти до каталогу Silpo references.

Не змінювати в межах цієї гілки музей, стадіон, зброю, транспорт, ботів, меню або інші незалежні системи, якщо зміна не потрібна безпосередньо для роботи «Сільпо».

## 3. Ієрархія джерел

Порядок пріоритету:

1. Поточний користувацький фотонабір «Сільпо» з цього чату: зовнішні та внутрішні ракурси.
2. Офіційна адреса магазину «Сільпо».
3. Публічні картографічні дані для геоприв'язки, орієнтації ділянки та сусідніх об'єктів.
4. Публічні фото/панорами конкретно цього магазину.
5. Лише після цього — обережна інтерполяція невидимих частин.

Заборонено видавати приблизний розмір, реконструйований план або припущення за точне вимірювання.

## 4. Перевірена геолокація

Офіційний сайт «Сільпо» підтверджує адресу:

`м. Остер, вул. Хмельницького Богдана, 54`

Public reference:
`https://silpo.ua/stores/vul-khmiel-nits-kogho-boghdana-54`

Visicom address-center:

- latitude: `50.948833799986254`
- longitude: `30.87572244094098`

Public reference:
`https://maps.visicom.ua/i/ADR3KJXJOBMR8PFNND`

Visicom also returns an address bounding envelope:

- west/south: `30.875477200868417, 50.94875909434981`
- east/north: `30.875857188233603, 50.94896906215886`

This envelope is NOT treated as a cadastral building footprint. It is only a location confidence check.

Using the project WGS84 -> Unreal tangent-plane mapping with the museum origin (`50.948239, 30.883865`), the Silpo address center resolves approximately to:

- Unreal X: `-57107 cm` (west of the project origin)
- Unreal Y: `+6621 cm` (north of the project origin)
- Unreal Z: terrain level

The runtime implementation must call `FOCGeoReference::Silpo()` instead of duplicating raw coordinates in multiple systems.

## 5. Що вже було в проєкті до цієї гілки

Audit result:

- окремого `Silpo`, `Сільпо` або `Supermarket` actor/subsystem у `main` не знайдено;
- центральний Остер зараз формується `AOCWorldSectorOster` із source-only доріг, житлових блоків, рослинності та landmark proxy geometry;
- у районі адреси «Сільпо» немає окремої photo-driven production reconstruction;
- у проєкті вже є робочий replicated `AOCInteractableDoor`, тому для входу не створюється другий несумісний механізм дверей;
- музей уже має окремий photo-model/replacement pattern, але Silpo pass має бути незалежним і не видаляти музейні компоненти.

Тому Silpo implementation створюється як окремий world subsystem із вузьким cleanup radius навколо Silpo anchor.

## 6. Геометрія будівлі — Phase 1

Поточна runtime-модель використовує photo-proportioned перший блок-аут:

- приблизна довжина: `30.0 m`;
- приблизна глибина: `17.5 m`;
- висота стін: `4.3 m`;
- низький похилий дах;
- довгий одноповерховий торговий об'єм;
- центральний/майже центральний вхідний проріз;
- фасад із цегляно-бежевою/приглушеною старою кладкою;
- темний дах;
- прямокутні вікна;
- видимі решітки на бокових/задніх вікнах там, де це підтверджують фото;
- невеликий навіс над входом;
- бордово-марунова фасадна плашка `СІЛЬПО`;
- тверде покриття перед входом.

Ці 30.0 x 17.5 x 4.3 m є стартовими пропорціями, а не заявою про інвентаризаційні розміри. Вони мають коригуватися тільки після надійного footprint/планового джерела або фотограмметричної звірки.

## 7. Фасад

Обов'язкові ознаки першого візуального проходу:

- не робити скляний сучасний супермаркет;
- зберегти просту стару одноповерхову коробку;
- передати характер кладки, карнизної лінії, віконних прорізів і невеликого входу;
- фасадна вивіска повинна читатися з дороги;
- вікна мають бути частиною конкретного ритму фасаду, а не випадковими повтореннями;
- зовнішня геометрія не повинна блокувати фізичний вхід;
- масштаб дверей, вікон і висоти стелі перевіряється відносно капсули гравця.

## 8. Вхід і двері

«Сільпо» має бути реально прохідним.

Вхідний проріз у стіні створюється геометрично, а не текстурою.

Перший етап використовує дві окремі replicated `AOCInteractableDoor`:

- interaction prompt через існуючу систему `E`;
- серверна зміна стану;
- реплікація відкрито/закрито;
- фізична колізія дверного полотна;
- doorway не повинен перекриватися shell geometry;
- двері спавняться тільки authoritative world, щоб клієнт не створював дубль.

## 9. Інтер'єр — Phase 1

Мета зараз — не повністю наповнений магазин, а правильний прохідний торговий простір.

Обов'язково:

- проста підлога;
- невисока звичайна стеля;
- базове нейтральне магазинне світло;
- 6 простих двосторонніх gondola shelf lines;
- полиці пусті;
- 4 звичайні касові місця;
- прості wall-cooler/refrigeration shells уздовж задньої стіни;
- мінімальні back-of-house utility markers;
- нормальні проходи між рядами;
- жодних випадкових коробок/меблів, що перекривають навігацію.

Не робити на цьому етапі:

- повний асортимент товарів;
- дрібні цінники;
- брендовану рекламу кожної категорії;
- складський inventory simulation;
- касову економіку;
- NPC-покупців.

## 10. Освітлення

Перший pass:

- рівномірне нейтральне біле магазинне світло;
- без декоративної кольорової підсвітки;
- без темних провалів у проходах;
- світильники не повинні створювати надмірний shadow cost;
- фінальна форма LED/люмінесцентних світильників буде окремим art pass.

## 11. Прилегла територія

Моделювати тільки те, що стосується Silpo site і підтверджується фото/картою:

- передній paved forecourt / підхід до входу;
- край вулиці та реальний напрямок підходу;
- бокова і задня смуга ділянки;
- місця дерев, стовпів, дротів, огорож/сусідніх будівель — після звірки конкретних ракурсів;
- service/back-of-house зона без вигаданого великого loading dock, якщо його немає на фото.

Source cleanup не має видаляти дороги або великі міські системи. Дозволено прибирати лише generic building/fence/tree proxies, що фізично конфліктують із Silpo footprint.

## 12. Сусідні об'єкти та міський контекст

Публічні джерела підтверджують комерційний контекст тієї ж вулиці, зокрема аптеку біля №50Г та інші невеликі точки на Богдана Хмельницького. Вони є орієнтирами для вуличної щільності, але НЕ входять у Silpo branch як окремі detailed buildings без власного reference pass.

Головний принцип: не перетворювати ділянку на ізольовану коробку посеред порожньої карти, але й не вигадувати точний сусідній фасад без джерела.

## 13. План/схеми

Під час первинного web-audit знайдено надійні адресу і координатну прив'язку, але не знайдено індексованого офіційного поверхового плану саме будівлі «Сільпо» за адресою Остер, Богдана Хмельницького 54.

Тому:

- не вигадувати «офіційний план»;
- поточний interior layout є gameplay blockout, побудованим за фото;
- якщо буде знайдено публічний технічний/кадастровий/орендний план саме цієї будівлі, його геометрія має замінити приблизний blockout;
- будь-який новий plan source додається до reference manifest із URL, датою доступу та типом доказу.

## 14. Reference set

Поточний Silpo photopack: 20 зображень у поточному повідомленні користувача, зовнішні та внутрішні ракурси.

Музейні фото з попереднього повідомлення: `EXCLUDED`.

Планована структура репозиторію:

`OsterConflict/SourceReferences/Locations/Silpo_Oster/Photos/`

Naming convention:

- `SILPO_EXT_FRONT_01.*`
- `SILPO_EXT_FRONT_02.*`
- `SILPO_EXT_SIDE_LEFT_01.*`
- `SILPO_EXT_SIDE_RIGHT_01.*`
- `SILPO_EXT_REAR_01.*`
- `SILPO_EXT_APPROACH_01.*`
- `SILPO_INT_ENTRANCE_01.*`
- `SILPO_INT_CHECKOUTS_01.*`
- `SILPO_INT_AISLES_01.*`
- `SILPO_INT_AISLES_02.*`
- `SILPO_INT_COOLERS_01.*`
- інші кадри — за фактичним вмістом після binary ingest.

Reference image binaries мають зберігатися окремо від runtime `Content` і не імпортуватися автоматично як Unreal assets.

## 15. Реалізація в коді

### Geo contract

- `FOCGeoReference::Silpo()`
- single WGS84 source of truth

### Runtime model

- `UOCR140SilpoPhotoModelSubsystem`
- запускається тільки для `OsterConflict_Runtime` Game/PIE world;
- пропускає frontend-only session;
- локально прибирає conflicting source proxies;
- створює enterable Silpo shell;
- створює sparse interior;
- authoritative world створює replicated entrance doors.

### Tags

- `R140_SilpoPhotoModel`
- `SilpoOster_BohdanaKhmelnytskoho54`
- `R140_SilpoEntranceDoor`

## 16. Acceptance criteria

Географія:

- модель використовує `FOCGeoReference::Silpo()`;
- центр моделі не задається випадковим FVector;
- відстань runtime root від computed Silpo local anchor не перевищує 1 cm;
- жодна Silpo-specific зміна не переміщує музей/стадіон.

Exterior:

- будівля впізнається за силуетом і фасадом без HUD-підпису;
- пропорції не виглядають як generic cube;
- вхід і вивіска читаються з переднього підходу;
- дах, фасад і віконний ритм узгоджені з фото.

Interior:

- гравець проходить через двері всередину;
- немає invisible wall у doorway;
- полиці і каси мають collision;
- проходи залишаються прохідними;
- базове освітлення працює;
- немає товарного clutter у Phase 1.

Networking:

- двері створює сервер/standalone world;
- door state реплікується;
- клієнт не створює duplicate entrance doors.

Regression:

- frontend не запускає Silpo model pass;
- runtime map завантажується без class/name collision;
- музейні R13.7 компоненти не ховаються Silpo cleanup;
- source cleanup не торкається `Roads`/`Sidewalks`.

## 17. Наступні підетапи

1. Geo/site audit — DONE for address anchor; footprint bearing remains non-survey.
2. Branch isolation — DONE.
3. Enterable shell — IMPLEMENTED first pass.
4. Facade/photo proportions — IMPLEMENTED first pass, requires in-engine visual comparison.
5. Sparse shelves/checkouts/lights — IMPLEMENTED first pass.
6. Correct binary photo reference ingest — pending only because the current Silpo inline images are not exposed to the repository connector as downloadable file bytes in this session; do not substitute the mounted museum images.
7. Satellite/plan footprint refinement — pending reliable public footprint/plan source.
8. UE 5.8 compile + PIE visual validation — pending CI/local build run.
9. Detail pass: exact window spacing, signage, utility boxes, poles/wires, landscaping, rear service area.
10. Product/detail population — separate future pass.
