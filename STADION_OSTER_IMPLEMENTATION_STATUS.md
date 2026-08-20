# OSTER CONFLICT — STADION OSTER IMPLEMENTATION STATUS

Статус: `CODED_UNTESTED`
Гілка: `stadion-oster`
Draft PR: `#14`
Базова гілка: `r13-content-gameplay-pass`
Дата: 2026-08-20

## Уже реалізовано

- canonical WGS84 anchor стадіону: `50.949360, 30.884660`;
- один authoritative stadium site owner: `UOCR13StadiumSurfaceSubsystem`;
- runtime site root: `R13_StadionOsterSiteRoot`;
- retired старий delayed museum/stadium presentation pass;
- приховування legacy `StadiumGeometry` / `StadiumDetails` зі збереженням collision/backstop;
- локальне видалення legacy fence instances у межах stadium site без вимкнення глобального `Fences` component;
- основне поле 105 × 68 м зі штучним покриттям;
- базова футбольна розмітка та повнорозмірні ворота 7.32 × 2.44 м;
- restrained artificial running surface навколо поля;
- додаткові тренувальні ворота;
- баскетбольні стійки/щити;
- група вуличних перекладин;
- photo-derived вхідна синьо-жовта стела як окремий stadium prop blockout;
- природні сегментовані ґрунтові стежки замість прямих road-like смуг;
- нерівномірний tree belt із наявних imported meshes;
- прилеглі будинки та короткі секції парканів із наявних imported meshes;
- canonical photo pack та `INDEX.md` для 17 референсів;
- окремий structural verifier `VERIFY_R13_STADION_OSTER.py`;
- verifier підключений до `.github/workflows/source-verify.yml`.

## Ще не VERIFIED

До статусу `VERIFIED` заборонено переходити без:

1. успішного UE 5.8 compile/build;
2. локального gameplay test;
3. перевірки фактичного Z/terrain контакту на stadium anchor;
4. перевірки collision воріт, огорож і service geometry;
5. перевірки, що немає z-fighting / duplicate surfaces / delayed flicker;
6. візуального зіставлення входу, дерев, стежок, житлової межі та спортзон із canonical photo pack;
7. остаточного пропа напису `Стадіон Остер`, який не може бути mirrored/placeholder;
8. уточнення трибун, роздягалень і санвузлів лише за підтвердженим візуальним матеріалом, без вигаданих великих споруд.

## Правило merge

PR #14 залишається Draft. Не зливати в базову гілку до проходження source CI та UE 5.8 локальної перевірки.
