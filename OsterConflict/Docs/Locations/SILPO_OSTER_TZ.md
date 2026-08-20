# OSTER CONFLICT — ТЗ: СІЛЬПО, ОСТЕР

Status: `CODED_UNTESTED`
Source branch: `silpo-oster`
Integration branch: `main`
Main forward-port: `ad689dff859bc65332669788cb94f727623ce7ab`
Location: м. Остер, Чернігівська область, вул. Богдана Хмельницького, 54
Scope owner: Silpo location only

## 1. Мета

Створити впізнавану, прохідну та географічно прив'язану 3D-реконструкцію остерського «Сільпо» з безпосередньою прилеглою територією. Модель має відповідати конкретному магазину з референсів, а не бути типовим супермаркетом.

Поточний реалізований scope:

- правильний силует і фасад;
- реальний вхід у будівлю;
- робочі інтерактивні двері;
- базовий інтер'єр без товарного наповнення;
- пусті полиці, каси, холодильне обладнання, острів овочів/фруктів;
- низька підвісна стеля та магазинне освітлення;
- візуальні деталі підлоги, стелажів, холодильників і кас;
- впізнавана фасадна вивіска та характерний ступінчастий парапет;
- найближчий вуличний контекст;
- 20 фото-референсів збережені в репозиторії разом із цим ТЗ.

## 2. Стан інтеграції

Початкова ізольована реалізація виконувалась у `silpo-oster`.

2026-08-20 користувач прямо схвалив перенесення всього Silpo pass у `main`. Інтеграцію виконано контрольованим forward-port поверх актуального `main`, а не force-переміщенням гілки. Так збережені новіші зміни `main`, зокрема окремі геореференси інших локацій.

Це не означає runtime verification. До UE 5.8 build/PIE і користувацького playtest статус залишається `CODED_UNTESTED`.

У межах Silpo pass не змінювати незалежні локації, зброю, транспорт, ботів, меню/UI та глобальні gameplay-системи, якщо зміна не потрібна безпосередньо для роботи «Сільпо».

## 3. Джерела та рівні впевненості

Пріоритет доказів:

1. 20 користувацьких фото конкретного остерського «Сільпо».
2. Офіційна сторінка магазину «Сільпо».
3. Публічна картографічна адресна прив'язка.
4. Публічні містобудівні матеріали для контексту району.
5. Обережна інтерполяція лише тих частин, які не видно на фото.

Заборонено видавати приблизні розміри, реконструйований план або орієнтацію фасаду за точне кадастрове/інвентаризаційне вимірювання.

## 4. Перевірена геолокація

Офіційна адреса:

`м. Остер, вул. Хмельницького Богдана, 54`

Public reference:
`https://silpo.ua/stores/vul-khmiel-nits-kogho-boghdana-54`

Visicom address-center:

- latitude: `50.948833799986254`
- longitude: `30.87572244094098`

Public reference:
`https://maps.visicom.ua/i/ADR3KJXJOBMR8PFNND`

Visicom address envelope:

- west/south: `30.875477200868417, 50.94875909434981`
- east/north: `30.875857188233603, 50.94896906215886`

Цей envelope використовується тільки як адресний контроль і не вважається кадастровим footprint будівлі.

Project WGS84 -> Unreal mapping дає приблизно:

- Unreal X: `-57107 cm`;
- Unreal Y: `+6621 cm`;
- Unreal Z: terrain level.

Єдина runtime-точка істини: `FOCGeoReference::Silpo()`.

## 5. Візуальні висновки з фото

Фото підтверджують такі ключові ознаки:

- довга одноповерхова світло-персикова/бежева будівля;
- фасад із симетричним ступінчастим парапетом і темними металевими cap-лініями;
- велика помаранчева овальна/фігурна вивіска з темно-синім контуром та біло-синім написом у верхній центральній частині;
- громадський вхід розташований біля лівого краю довгого фасаду, а не по центру;
- невеликий виступаючий вхідний вузол/навіс і піднятий поріг;
- більша частина довгого фасаду глуха та зайнята рекламними панелями;
- перед фасадом звичайний асфальтований майданчик з припаркованими автомобілями;
- виразної постійної розмітки паркомісць на референсах не видно;
- перед фасадом видно окремий синій знак `P` на стійці;
- біля правого краю будівлі є стовп/повітряні комунікації;
- поруч із боковою частиною видно дрібний ринковий/вуличний торговий контекст;
- всередині низька підвісна плиткова стеля;
- світлі квадратні підлогові плитки;
- довгі ряди звичайних магазинних стелажів;
- дерев'яний/деревоподібний низький острів овочів/фруктів;
- довгі холодильні прилавки/вітрини;
- компактна багатокасова зона;
- над касами присутні невеликі нумеровані маркери;
- освітлення звичайне лінійне магазинне, без декоративної кольорової сцени.

## 6. Геометрія R14.0 — базовий photo model

Runtime owner:

- `UOCR140SilpoPhotoModelSubsystem`
- actor tag: `R140_SilpoPhotoModel`
- identity tag: `SilpoOster_BohdanaKhmelnytskoho54`

Поточний blockout:

- приблизна довжина: `30.0 m`;
- приблизна глибина: `17.5 m`;
- висота основної стіни: `3.9 m`;
- low/flat roof за парапетом;
- ступінчастий фасадний парапет;
- громадський вхід: `EntranceCenterX = -1315 cm`;
- ширина вхідного прорізу: `140 cm`;
- висота вхідного прорізу: `245 cm`.

Ці розміри є gameplay/photo blockout і можуть коригуватися після надійнішого footprint або вимірювального джерела.

## 7. Фасад R14.0

Реалізовано:

- світлий штукатурний фасад;
- темний нижній цоколь;
- ступінчастий парапет;
- центральну sign-zone;
- базову `СІЛЬПО` sign placeholder geometry;
- рекламні панелі вздовж фасаду;
- невеликий навіс і поріг біля входу;
- бокові/задні прорізи тільки там, де вони не суперечать основним фото;
- локальний cleanup generic building/fence/tree proxy geometry навколо site anchor;
- Roads і Sidewalks не є ціллю cleanup.

## 8. Вхід і двері

Публічний вхід повинен бути реально прохідним.

Поточний контракт:

- один головний вхід, як на фото;
- реальний геометричний проріз у front wall;
- один replicated `AOCInteractableDoor`;
- actor tag: `R140_SilpoEntranceDoor`;
- identity tag: `SilpoEntranceMain`;
- двері спавняться лише при `World.GetNetMode() != NM_Client`;
- interaction використовує існуючу систему `E`;
- стан open/closed реплікується існуючим door actor;
- дверне полотно має фізичну колізію.

## 9. Інтер'єр R14.0

Поточний gameplay-ready interior pass:

- базова підлога;
- 6 довгих порожніх shelf runs;
- полиці пусті;
- 4 касові місця;
- порожній produce island;
- rear wall cooler/refrigeration shells;
- мінімальні service/back-of-house markers;
- широкі проходи без випадкових блокуючих об'єктів;
- 15 runtime point lights;
- геометричні лінійні ceiling fixtures.

Товарне наповнення, цінники, дрібна брендова реклама та NPC не входять у поточний pass.

## 10. R14.1 — exterior/site photo detail pass

Runtime owner:

- `UOCR141SilpoDetailSubsystem`
- actor tag: `R141_SilpoPhotoDetails`

R14.1 додає поверх базової моделі без руйнування R14.0:

- низьку suspended ceiling plane;
- grid підвісної стелі;
- уточнення trim біля входу;
- тонкі cap-елементи ступінчастого парапету;
- кронштейни зовнішніх фасадних світильників;
- просту checkout queue/cart-bay geometry;
- 4 overhead checkout number signs;
- один utility pole біля правого краю фасаду;
- низькодетальний immediate side-market edge;
- asphalt correction поверх ранньої умовної парковочної розмітки, оскільки на фото майданчик читається як звичайний зношений асфальт.

## 11. R14.2 — interior photo detail pass

Runtime owner:

- `UOCR142SilpoInteriorDetailSubsystem`
- actor tag: `R142_SilpoInteriorDetails`

R14.2 є суто візуальним, non-destructive та non-colliding шаром поверх gameplay geometry. Він додає:

- тонку сітку швів квадратної підлогової плитки з кроком приблизно `62 cm`;
- торцеві панелі та нижні kick-rails на шести порожніх shelf runs;
- темні front glass-площини холодильників;
- вертикальні та верхні/нижні рамки холодильних дверей;
- темні conveyor-belt поверхні на чотирьох касах;
- вузькі теплі accent trims касових столів;
- shallow dividers на порожньому produce island, щоб читалась структура лотків;
- темний entrance mat одразу всередині громадського входу.

Усі R14.2 visual components мають `NoCollision`, тому цей pass не повинен змінювати маршрути гравця або створювати нові невидимі блокери.

## 12. R14.3 — facade identity pass

Runtime owner:

- `UOCR143SilpoFacadeIdentitySubsystem`
- actor tag: `R143_SilpoFacadeIdentity`

R14.3 виправляє найбільш впізнавану частину будівлі, не торкаючись collision shell:

- перекриває ранню прямокутну placeholder-плашку кольором фасаду;
- використовує source-only `/Engine/BasicShapes/Cylinder.Cylinder` як сплющені еліпси;
- формує більший темно-синій outline і меншу помаранчеву face-площину;
- додає два TextRender layers `Сільпо`: синій shadow та світлу face-площину;
- додає темні тонкі top rails по п'яти рівнях ступінчастого парапету;
- додає окремий синій знак `P` на стійці перед фасадом та нейтральну нижню табличку;
- усі R14.3 components мають `NoCollision`.

Це source-only наближення характерної форми вивіски, а не імпорт фотографії як текстури. Фінальний logo mesh/material можна замінити пізніше без зміни положення будівлі.

## 13. Освітлення

Вимоги:

- нейтральне біле магазинне світло;
- без декоративної кольорової підсвітки;
- не залишати темні провали між рядами;
- runtime-created point lights мають бути `Movable`;
- shadow cost для внутрішніх базових lights мінімізується;
- геометрія світильників повинна читатися як довгі лінійні fixtures під підвісною стелею.

## 14. Безпосередня територія

Моделюється тільки photo-supported immediate context:

- підхід до входу;
- асфальтований майданчик;
- вузька смуга рослинності біля фасаду;
- parking sign перед фасадом;
- utility pole біля правої сторони;
- найпростіший side-market edge як контекст масштабу;
- зв'язок із наявною дорожньою мережею карти без її видалення або перебудови.

Детальні сусідні будівлі не входять у цей Silpo pass без власного reference pass.

## 15. Публічні плани/схеми

Під час web-audit підтверджено існування містобудівного детального плану території в Острі, обмеженої вулицями Б. Хмельницького, Будівельників та ринковим містечком, для будівництва торговельного центру.

Це корисний контекст для району, але на поточному етапі не доведено, що його графічна частина є точним поверховим або технічним планом існуючої будівлі магазину за №54.

Тому:

- не називати реконструйований interior layout «офіційним планом»;
- не копіювати приблизні межі як кадастровий footprint;
- якщо буде знайдено публічний plan/technical drawing саме цієї будівлі, він має отримати окремий запис у reference manifest;
- після такого джерела допускається корекція `SilpoYawDegrees`, footprint і внутрішніх пропорцій.

## 16. Photo reference pack

У `main` збережено 20 окремих review copies:

1. `Photos/01_exterior_facade_front.jpg`
2. `Photos/02_interior_entry_vertical.jpg`
3. `Photos/03_interior_produce_aisle.jpg`
4. `Photos/04_exterior_facade_sign_close.jpg`
5. `Photos/05_interior_beverage_shelf.jpg`
6. `Photos/06_interior_refrigerated_counter.jpg`
7. `Photos/07_interior_checkout_zone.jpg`
8. `Photos/08_exterior_entrance_sidewalk_oblique.jpg`
9. `Photos/09_exterior_entrance_corner.jpg`
10. `Photos/10_exterior_side_wall_posters.jpg`
11. `Photos/11_interior_main_aisles.jpg`
12. `Photos/12_exterior_entrance_close.jpg`
13. `Photos/13_exterior_facade_across_road.jpg`
14. `Photos/14_exterior_facade_market_side.jpg`
15. `Photos/15_exterior_side_market_activity.jpg`
16. `Photos/16_exterior_entrance_porch_close.jpg`
17. `Photos/17_context_opposite_building_annotation.jpg`
18. `Photos/18_exterior_side_facade_warm_light.jpg`
19. `Photos/19_context_facade_street_wide.jpg`
20. `Photos/20_context_bohdana_khmelnytskoho_street.jpg`

Review/reference path:

`OsterConflict/SourceReferences/Locations/Silpo_Oster/Photos/`

Canonical persistent location pack under the repository-wide retention convention:

`REFERENCE_PHOTOS/silpo_oster/`

Ці файли є reference/review assets і не імпортуються в Unreal `Content` як gameplay textures.

## 17. Static validation

Repository validator:

`python OsterConflict/Tools/validate_silpo_location.py`

Перевіряє:

- WGS84 geo contract;
- expected local coordinate conversion;
- R14.0/R14.1/R14.2/R14.3 runtime-map і frontend guards;
- відсутність target-логіки для Roads/Sidewalks;
- один головний `AOCInteractableDoor`;
- photo-derived far-left entrance constants;
- базові компоненти фасаду й інтер'єру;
- R14.1 detail contract;
- R14.2 non-colliding interior detail contract;
- R14.3 layered facade-logo/parapet/parking-sign contract;
- наявність усіх 20 photo files;
- узгодженість ТЗ, scope і reference README.

## 18. Acceptance criteria перед статусом VERIFIED

Обов'язково перевірити в UE 5.8:

- проєкт компілюється без C++ errors;
- `OsterConflict_Runtime` запускається без crash;
- R14.0 actor з'являється біля `FOCGeoReference::Silpo()`;
- R14.1 detail actor з'являється після базової моделі;
- R14.2 interior-detail actor з'являється без collision side effects;
- R14.3 facade-identity actor з'являється поверх placeholder sign без z-fighting;
- фасад читається з дороги як конкретний магазин із фото;
- layered orange/blue logo не провалюється в стіну й не висить перед фасадом надто далеко;
- вхід знаходиться біля лівого краю фасаду;
- doorway фізично не перекритий wall/trim geometry;
- одна public entrance door відкривається і закривається;
- клієнт не створює duplicate door;
- всередину можна зайти FPS-персонажем;
- між shelf runs можна пройти;
- касова зона не блокує головний маршрут;
- suspended ceiling не перетинає player camera;
- tile-grout, shelf trim, cooler frames і R14.3 facade identity не мають collision;
- basic lights працюють у runtime;
- дорога біля site не зникає після cleanup;
- generic proxy building не стирчить крізь Silpo shell;
- 20 photo reference files присутні в `main`;
- `python OsterConflict/Tools/validate_silpo_location.py` повертає PASS.

## 19. Наступні art passes

Після compile/PIE validation:

1. точніше звірити yaw і footprint по надійному top-down source;
2. уточнити пропорції вхідного вузла;
3. перевірити масштаб/нахил layered logo в реальному camera view та відкоригувати форму;
4. зробити реалістичніші дверні/віконні рами;
5. деталізувати касові столи, cart bay і холодильники;
6. додати реалістичні empty shelf meshes замість чистих box primitives;
7. додати дрібний exterior wear, цоколь, бордюри, кабелі та фасадні світильники;
8. лише після цього переходити до товарного наповнення.
