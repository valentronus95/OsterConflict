# OSTER CONFLICT — STADION OSTER IMPLEMENTATION STATUS

Статус: `CODED_UNTESTED`
Інтеграційна гілка: `main`
Джерело forward-port: `stadion-oster`, Draft PR `#14`
Дата: 2026-08-20

## Уже реалізовано

- canonical WGS84 anchor стадіону: `50.949360, 30.884660`;
- один authoritative stadium site owner: `UOCR13StadiumSurfaceSubsystem`;
- runtime site root: `R13_StadionOsterSiteRoot`;
- retired старий delayed museum/stadium presentation pass;
- legacy `StadiumGeometry` / `StadiumDetails` приховуються синхронно ще під час source-world build, а не залежать від порядку BeginPlay;
- старі delayed `CivicLandscaping` і `LandmarkSiteDressing` більше не додають на стадіон окремі дерева, смітники або utility props через 2.05–2.35 с;
- приховування legacy `StadiumGeometry` / `StadiumDetails` зі збереженням collision/backstop;
- локальне видалення legacy fence instances у межах stadium site без вимкнення глобального `Fences` component;
- основне поле 105 × 68 м зі штучним покриттям;
- базова футбольна розмітка та повнорозмірні ворота 7.32 × 2.44 м;
- restrained artificial running surface навколо поля;
- додаткові тренувальні ворота;
- баскетбольні стійки/щити;
- група вуличних перекладин;
- photo-derived синьо-жовта вхідна стела;
- окремий `TextRenderComponent` із написом `СТАДІОН ОСТЕР`, прив'язаний до того самого authoritative site root;
- actor-level collision увімкнено; visual-only grass/turf/track/path залишені `NoCollision`;
- collision увімкнено для спортивних металевих елементів, несучої частини входу та replacement fence meshes;
- природні сегментовані ґрунтові стежки замість прямих road-like смуг;
- нерівномірний tree belt із наявних imported meshes;
- прилеглі будинки та короткі секції парканів із наявних imported meshes;
- `INDEX.md` для 17 референсів і збережений payload photo pack; під час інтеграції виявлено, що payload не проходить ZIP integrity test, тому потрібне відновлення з оригіналів;
- окремий structural verifier `VERIFY_R13_STADION_OSTER.py`;
- verifier підключений до `.github/workflows/source-verify.yml`.

## CI

- окремий `VERIFY_R13_STADION_OSTER.py` проходить локально та допускає повне видалення застарілих civic/landmark subsystems у `main`;
- локально виправлено два stale regression-маркери: S16A тепер перевіряє актуальний canonical college anchor, а Silpo verifier перевіряє фактичну геометрію замість видалених коментарів;
- legacy stadium verifiers переведено на новий exclusive-owner contract; окремо проходять stadium, landmark-dressing і retired R13.6 photo-fidelity checks;
- exclusive-owner runtime handoff: commit `e836625`;
- source-verification exit-code hardening та stale marker repairs: commit `292a114`;
- виявлено false-green у `Source verification`: PowerShell продовжував виконання після ненульового exit code попереднього Python verifier і повертав успіх останнього stadium verifier;
- workflow посилено явною перевіркою `$LASTEXITCODE` після кожного Python-прогону;
- окремий `S01 location-first verification` має baseline-падіння в `VERIFY_R13_LOCATION_FIRST_S01_ROAD_TOPOLOGY.py` для `S01_KR_SPINE_SOUTH_SHARED`;
- stadium branch не змінює `OCLocationSectorS01RoadData.cpp` або цей topology verifier, а серед geo anchors, що впливають на stadium diff, змінений тільки `Stadium()`. Тому S01 road geometry не виправляється всередині stadium task.
- на вихідній стадіонній гілці після виправлення stale regression-маркерів повний локальний `RUN_ALL_VERIFY.py` доходив до відомого S01 baseline-падіння;
- на інтегрованій базі `main` повний `RUN_ALL_VERIFY.py` зупиняється раніше на наявному R11 baseline-маркері `SetFogDensity(0.0085f)`: у поточному `main` і до, і після forward-port фактичне значення `0.0060f`, а `OCVisualEnvironment.cpp` цією інтеграцією не змінювався;
- `stadion_oster_reference_pack_2026-08-20.zip` не проходить `unzip -t`; payload збережено без підміни, але canonical pack треба повторно зібрати з 17 оригіналів і перевірити.

## Ще не VERIFIED

До статусу `VERIFIED` заборонено переходити без:

1. успішного UE 5.8 compile/build;
2. локального gameplay test;
3. перевірки фактичного Z/terrain контакту на stadium anchor;
4. runtime-перевірки collision воріт, огорож і entrance structure;
5. перевірки, що немає z-fighting / duplicate surfaces / delayed flicker;
6. візуального зіставлення входу, дерев, стежок, житлової межі та спортзон із canonical photo pack;
7. візуальної перевірки напису `СТАДІОН ОСТЕР`: правильний бік, кириличні glyphs, відсутність mirrored rendering;
8. уточнення трибун, роздягалень і санвузлів лише за підтвердженим візуальним матеріалом, без вигаданих великих споруд.

## Стан інтеграції

За прямою вказівкою користувача реалізацію контрольовано перенесено в `main` без blind merge застарілої бази. Статус залишається `CODED_UNTESTED`: інтеграція в `main` не замінює UE 5.8 build і локальний gameplay test.
