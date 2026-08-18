# Oster Conflict — R13.4/R13.5 великий візуальний пакет

## Статус

Локальний UE 5.8 compile/runtime test свідомо **відкладено до завершення великого пакета**. Мета — не змушувати тестувальника робити повний pull/build після кожного дерева, трав'яного шару або фасадної правки.

Поточний пакет збирається в `r13-content-gameplay-pass`, після чого має бути один консолідований локальний прохід.

## Карта відповідальності runtime-art систем

### `OCR13WholeOsterArtSubsystem`
Власник базового структурного шару:
- Roadside Construction roads/sidewalks;
- основні готові `SM_House_Var01/02` замість житлових cube-proxy;
- основні листяні AdvancedVillage дерева;
- реальні `SM_Pine_Tree_01/03/05` для authored pine topology;
- приховування старих structural residential proxy та rejected fantasy R12 fence/light сімейств.

Не малює окремий другий килим трави.

### `OCR13EnvironmentDressingSubsystem`
Єдиний власник базового густого ground dressing:
- PN grass;
- mown / rough / wetland density;
- ground plants;
- house companion `Extra` modules;
- базові дворові props;
- companion trees/stumps;
- grass exclusions біля доріг, будинків та civic geometry.

### `OCR13FoliageDiversitySubsystem`
Власник додаткової видової різноманітності:
- shrubs;
- bushes;
- reeds/cattails;
- companion pines;
- периферійна рослинність.

Тимчасовий дубль `OCR13FoliageDetailSubsystem` видалено.

### `OCR13ResidentialInfillSubsystem`
Заповнює порожні житлові краї після compact crop:
- максимум 18 будинків;
- тільки від authored довгих road transforms;
- exclusion навколо музею, стадіону, парку, технікуму та обох коридорів Крушельницької;
- clearance від source/infill будинків;
- collision-backed `SM_House_Var01/02`;
- canonical `R13_House01/02`, щоб `EnvironmentDressing` автоматично давав ті самі extras/yard dressing;
- hidden source footprint metadata резервують grass/yard space, але не рендеряться і не мають collision.

### `OCR13ResidentialYardSubsystem`
Тільки унікальні props, яких немає в `EnvironmentDressing`:
- side shed;
- outhouse;
- wheelbarrow;
- pallet;
- tire;
- utility box.

Не дублює logs/barrels/crates.

### `OCR13EnterableHouseArtSubsystem`
Покращує функціональний enterable-house без закриття прорізів:
- painted rural wood surface;
- `Wood_Old` fences;
- normal yard material;
- real pitched `Roof_Both_Ends_4m`;
- `Metal_Roof`;
- debug label hidden.

Авторські door/window openings, rooms, interior і gameplay collision залишаються незмінними.

## Музей

Музей не замінюється generic house і залишається географічним landmark anchor.

### `OCR13MuseumReferenceSubsystem`
Фото-референсний фасадний шар:
- темний цоколь;
- теплий red/orange brick band/cornice;
- сіро-блакитний верхній дерев'яний об'єм;
- реальний `Wood_Planks_Painted_Blue` з контрольованим fallback;
- світлий трикутний gable outline;
- сірі подвійні двері;
- локальні сходи/landing;
- боковий glazed porch з `Glass_Window`;
- mature `SM_Pine_Tree_01/03/05`.

### `OCR13LandmarkSiteDressingSubsystem`
Єдиний власник довгої museum approach path через `SM_Forest_Path`.

### `OCR13CivicLandscapingSubsystem`
Дає ширший deciduous/shrub frame навколо museum garden, не перекриваючи entrance axis.

## Центральний парк

### `OCR13CentralParkDressingSubsystem`
- shrubs;
- flowers;
- long grass;
- clear memorial/core paths/skate area.

### `OCR13CentralParkCanopySubsystem`
- 20 deterministic mature canopy trees;
- AdvancedVillage deciduous + actual pine meshes;
- memorial plaza, primary alleys і skate pad залишаються відкритими;
- visual-only, без collision/navigation influence.

### `OCR13ParkFurnitureSubsystem`
Зберігається як власник 14 bench replacements через real `Old_Planks`.

## Технікум

### `OCR13CollegeFacadeSubsystem`
Не перебудовує authored 4-storey footprint:
- dark plinth;
- thin facade/floor bands;
- real `Glass_Window` entrance glazing;
- dark entrance canopy;
- source 9x4 windows, broad stairs і footprint збережені.

### `OCR13CivicLandscapingSubsystem`
Озеленює side/back campus edges, тримаючи головний entrance clear.

## Стадіон

### `OCR13StadiumSurfaceSubsystem`
Поверх source collision geometry:
- 105×68 m turf;
- track/apron;
- touch/goal/center lines;
- restrained penalty markings;
- visual goal-frame accents;
- small real-plank spectator seating (`Old_Planks`).

### `OCR13CivicLandscapingSubsystem`
Тільки perimeter greenery поза pitch/track/goal mouths.

## Крушельницька

### `OCKrushelnytskaVisualSliceSubsystem`
- PN grass family, спільна з whole-city dressing;
- real pine meshes;
- denser residential verges;
- legacy fantasy streetlights залишаються suppressed.

### `OCR13KrushelnytskaInfrastructureSubsystem`
Окремий owner, тому що generic roadside pass свідомо пропускає dedicated slice:
- `Power_Pole_1`;
- `Power_Pole_Addons`;
- sparse `Power_Pole_Light`;
- no collision/navigation influence.

## Загальна дорожня інфраструктура

### `OCR13RoadsideInfrastructureSubsystem`
Єдиний generic owner:
- real utility poles;
- addons;
- sparse lights;
- deterministic spacing;
- long authored road segments;
- dedicated Krushelnytska slice виключений.

Тимчасовий дубль `OCR13UtilityPoleSubsystem` видалено.

## Ground

### `OCR13GroundSurfaceSubsystem`
- `Diorama_Ground` material замість однотонного зеленого debug tint;
- **material only**;
- не змінює position, scale, visibility або collision source `Ground`;
- spawn-safety і vehicle-grounding продовжують використовувати той самий authoritative floor.

## Frontend rule

Нові dressing-системи не повинні створювати art actors у frontend-only session. Для них використовується `GameMode->IsFrontendOnlySession()` guard.

## Верифікація

Нові source gates:
- `VERIFY_R13_4_VISUAL_BATCH_CONSOLIDATION.py`
- `VERIFY_R13_4_RESIDENTIAL_INFILL.py`
- `VERIFY_R13_5_CENTRAL_PARK_CANOPY.py`
- `VERIFY_R13_5_COLLEGE_STADIUM_VISUALS.py`

Вони перевіряють ownership, дублювання, frontend guards, deterministic placement, hard caps, navigation/collision neutrality декоративних шарів і збереження функціональної геометрії.

## Коли робити локальний тест

Не після окремого фасаду/дерева/grass-pass. Локальний pull + UE 5.8 compile + runtime test проводиться один раз після завершення поточного environment/content batch.

Очікуваний консолідований runtime порядок перевірки:
1. main menu flicker;
2. deployment flow;
3. valid grounded spawn;
4. runtime map integrity;
5. museum reference look;
6. residential density / houses / yards;
7. grass / trees / park / college / stadium;
8. map scale feel;
9. bots;
10. red civilian car cockpit;
11. BoxTruck exit view recovery.

Аудіо до цього visual/content pass не входить.
