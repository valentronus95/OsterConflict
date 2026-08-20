# OSTER CONFLICT — Музей Остра: implementation status

Оновлено: 2026-08-20

Робоча гілка: `museum-oster`
Draft PR: `#16`

Цей файл фіксує фактичний стан реалізації музею. Він не замінює `MUSEUM_OSTER_TZ.md` і фото-індекс `MUSEUM_OSTER_PHOTO_REFERENCES_2026-08-20.md`.

## Поточний стек музейних шарів

### R13.7 — базова фото-модель
`UOCR137MuseumPhotoModelSubsystem`

Зберігаються як основа:
- головний дах;
- металеві roof sections;
- цоколь;
- частина карнизів;
- chimney;
- rear annex mass;
- yellow gas pipe;
- concrete approach/steps.

Суцільний brick body, prototype glass, prototype doors, старі ґрати, частина trim і старий симетричний tree pass замінюються новішими шарами.

### R13.8 — segmented architecture + enterable shell
`UOCR138MuseumInteractiveArchitectureSubsystem`

Реалізовано:
- зовнішні стіни секціями замість одного solid cube;
- реальні отвори під двері/вікна;
- walkable ground floor;
- консервативна interior partition схема;
- hollow blue-grey upper room;
- glazed vestibule shell;
- side veranda geometry;
- structural component tags `MuseumStructural` + `Section:*` для подальшого damage/Chaos pass;
- server-spawned interactive openings.

### R13.9 — головні двостулкові двері
`AOCMuseumDoubleDoor`
`UOCR139MuseumMainDoorReplacementSubsystem`

Реалізовано:
- окремі ліва/права стулки;
- collision;
- server-authoritative interaction;
- replicated open/closed state;
- плавне відкривання двох стулок;
- door audio;
- три яруси relief panels;
- додаткові raised perimeter rails навколо кожного ярусу REF-06;
- зовнішні/внутрішні вертикальні стійки на обох стулках;
- центральний raised spine з окремими relief nodes;
- окремі ручки;
- весь декор рухається разом із відповідною стулкою;
- автоматична заміна двох generic R13.8 leaves.

### R14.0 — corrected service gable + facade details
`UOCR140MuseumFacadeDetailSubsystem`
`AOCMuseumServiceDoubleDoor`

Реалізовано за REF-02/08/11/12/15/20:
- старий R13.7 red timber gable на неправильному торці приховується;
- red/brown timber gable перебудовується на торці зі службовими дверима;
- у gable є реальний отвір під верхнє вікно;
- додається upper gable breakable window;
- старий зміщений service canopy instance видаляється локально, не разом з усім дахом;
- створюється новий canopy у правильній зоні;
- carved horizontal gable band;
- dense brick dentil rhythm;
- corner pilaster relief;
- upper window surround + three-panel visual separation;
- fan-style upper grille;
- memorial plaque proxy;
- electrical boxes/cable detail;
- entrance-side window sill projections;
- address `30` як окремий world text detail;
- generic service door замінюється на tall replicated two-leaf `AOCMuseumServiceDoubleDoor`.

### R14.1 — музейні breakable windows
`AOCMuseumBreakableWindow`
`UOCR141MuseumWindowReplacementSubsystem`

Реалізовано:
- shared breakable gameplay state не переписується;
- музейні вікна отримують реальний `Glass_Window` material;
- pale/white frame material;
- center mullion;
- upper transom;
- glass shards успадковують glass material;
- лише музейні R13.8/R14.0 windows автоматично замінюються;
- інші будинки гри не змінюються;
- transform/scale/tags/owner prototype-вікна зберігаються;
- already-broken prototype window навмисно не «лікується» replacement pass-ом.

### R14.2 — entrance/vestibule close-range details
`UOCR142MuseumEntranceDetailSubsystem`

Реалізовано за REF-06/15/17/18:
- grey lower cladding під entrance side-lights;
- exposed brick base на боках vestibule;
- dark curtains behind breakable glass;
- information-board sheet behind right entrance glazing;
- layered pale entrance canopy fascia;
- small repeated carved rhythm under canopy;
- additional dormer carved trim;
- ornamental porch side railings;
- radial/sunburst railing motif;
- small dark base/access hatch under vestibule side.

### R14.3 — територія і низька рослинність
`UOCR143MuseumSiteVegetationSubsystem`

Реалізовано за REF-04/05/07/10/13/14:
- використовуються наявні `PN_FoliageCollection` grass/ground-plant meshes;
- центральна бетонна алея лишається чистою;
- зона входу та безпосередній периметр стін не заростають;
- передній газон отримує низьку, навмисно нещільну траву;
- бічна/задня зона має нерівномірні ground plants;
- біля rear annex додана невелика асиметрична зелена група за REF-13;
- placement deterministic, без випадкових змін між запусками.

### R14.4 — довгі фасади, водовідведення і rear annex
`UOCR144MuseumRearExteriorDetailSubsystem`

Реалізовано за REF-03/09/11/13/19/20:
- циліндричні жолоби вздовж довгих карнизів;
- вертикальні downspouts;
- короткі нижні відводи;
- службові сходи на торці зі service double door;
- темний base band задньої прибудови;
- fascia/edge detail задньої прибудови;
- невідомі door/window openings задньої прибудови навмисно не вигадуються.

Під час статичного аудиту виправлена орієнтація gutter cylinders: горизонтальні труби використовують `Pitch=90`, а не yaw-only rotation.

### R14.5 — photo-oriented mature tree layout
`UOCR145MuseumTreeLayoutSubsystem`

Реалізовано за REF-01/03/04/05/07/09/10/13/14:
- старі симетричні `R137Museum_Pine01/Pine03/Deciduous01` приховуються і втрачають collision;
- створюється окремий R14.5 tree actor;
- ялини вздовж центрального підходу розставлені нерівномірно, без дзеркальних пар;
- центральна алея лишається вільною;
- біля будівлі додані mature deciduous trees, які видно на фасадних/бокових фото;
- side conifers не перекривають службовий вхід;
- використовуються наявні project foliage meshes, без нових generic tree assets.

## Source-contract CI

Додано:
- `OsterConflict/Scripts/validate_museum_source_contracts.py`;
- `.github/workflows/museum-source-contracts.yml`.

CI перевіряє:
- наявність усіх музейних runtime-класів;
- structural tags/Section IDs;
- final door/window replacement contracts;
- `MakeUniqueObjectName` у runtime-created object/material paths;
- правильний порядок R13.8 → R14.5 delay layers;
- validator запускається після останнього шару;
- правильну орієнтацію gutter cylinders;
- suppression старого симетричного tree pass;
- заборону вигадувати нерозбірливий historical inscription;
- збереження підтвердженого номера будинку `30`.

Перший source-contract run після додавання workflow пройшов `success`. Після R14.5 workflow має повторно перевірити оновлений контракт.

## Runtime validation

`UOCR138MuseumRuntimeValidationSubsystem` після R14.5 перевіряє:
- рівно один segmented architecture actor;
- рівно один R14.0 facade detail actor;
- рівно один R14.2 entrance detail actor;
- рівно один R14.3 site vegetation actor;
- рівно один R14.4 rear exterior actor;
- рівно один R14.5 photo tree layout actor;
- не менше 30 structural sections;
- один final main double-door actor;
- один final service double-door actor;
- відсутність старої prototype service door;
- не менше 21 museum breakable windows;
- усі активні музейні breakable windows мають бути `AOCMuseumBreakableWindow`;
- рівно одне upper gable window;
- на старті жодне музейне вікно не повинно бути broken.

## Що ще НЕ вважається завершеним

### Обов'язково перед merge
- UE 5.8 Windows compile;
- UHT/UBT validation усіх нових UCLASS;
- runtime launch `OsterConflict_Runtime`;
- перевірка відсутності `Cannot replace existing object of a different class`;
- visual walkaround з 4 сторін;
- collision test біля кожного фасаду;
- main/service door interaction test;
- bullet test для кожного типу музейного вікна;
- multiplayer replication test дверей і broken state;
- FPS movement через main entrance та interior passage;
- перевірка foliage/old geometry overlap.

### High-fidelity exterior, наступні проходи
- texture/decal pass старої цегли, плям, тріщин та вицвілого історичного напису тільки після наявності/верифікації придатних decal assets;
- заміна remaining BasicShapes visual authoring parts на production/static assets там, де є відповідні mesh;
- фінальне звірення scale/placement після першого runtime walkaround.

### Interior
Точних фото/плану інтер'єру в поточному reference pack немає.
Тому interior зараз лише conservative gameplay shell. Не видавати його за історично точний.

### Destruction
Structural IDs уже закладені, але RPG/grenade building destruction ще не увімкнено.
Наступний damage pass має працювати по structural section ID, не руйнуючи будівлю одним глобальним health value.

## Merge policy

PR #16 лишається Draft.
Не зливати `museum-oster` у `main`, доки Windows compile/runtime validation не пройдені і музей не переглянуто в грі з близької дистанції.
