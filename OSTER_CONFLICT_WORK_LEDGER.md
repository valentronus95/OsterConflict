# OSTER CONFLICT — WORK LEDGER

> Постійний журнал фактичного стану `main`. Runtime-скрін/лог/playtest завжди має пріоритет над code-only твердженням.

## 1. Поточний контекст

- Repository: `valentronus95/OsterConflict`
- Active branch: `main`
- UE target: 5.8.x Windows
- Project: `OsterConflict/OsterConflict.uproject`
- User-facing launcher: **тільки `START_HERE.cmd`**.
- `RUN_*.cmd` — внутрішні helper scripts. Не створювати новий user-facing launcher під кожну R-версію.
- Persistent evidence: `RUNTIME_AUDIT_2026-08-21.md`, `LEGACY_BLOCKOUT_AUDIT_2026-08-21.md`, `RUNTIME_PLAYTEST_AUDIT_2026-08-21_1744.md`.
- Новий user playtest 2026-08-21 17:44 + пакет `Oster-photo-bag.docx` є authoritative evidence для поточного runtime.
- Не створювати нові декоративні R15/R16 layers, доки поточний runtime backlog не закритий.

## 2. Статусні правила

- `IN_PROGRESS` — runtime уже показав проблему або fix ще не підтверджено.
- `CODED_UNTESTED` — source fix є, але UE runtime його ще не підтвердив.
- `VERIFIED BUILD` — конкретний compile blocker підтверджено усуненим фактичним наступним build/run.
- `VERIFIED RUNTIME` — тільки після фактичного UE runtime/playtest.
- Один landmark/site = один authoritative placement owner; cleanup не замінює ownership.

## 3. Активні вимоги

| ID | Вимога | Repeat | Status | Фактичний стан / що лишилось |
|---|---|---:|---|---|
| UI-BOOT-001 | Splash → main menu без чорної паузи | 1 | IN_PROGRESS | Normal launch показує splash Oster Conflict, потім проміжковий чорний екран, потім main menu. Треба зробити безперервний loading/frontend presentation. |
| UI-MENU-001 | Головне меню стабільне | ≥3 | IN_PROGRESS | Main menu знову є і візуально виглядає нормально, але START дає сірий transition/фон і flow сприймається як повторний START. |
| UI-TRAVEL-001 | START має один зрозумілий перехід у deployment/game | ≥3 | IN_PROGRESS | Після першого START весь екран/фон сіріє, потім deployment step 4 теж має кнопку `СТАРТ`, через що виглядає як подвійний запуск. Потрібно розділити loading і deploy action, перейменувати final deploy CTA. |
| UI-CHAT-001 | Team chat `Y`, global chat `U`, панель прихована без вводу | 1 | IN_PROGRESS | Runtime: великий `КАНАЛ: КОМАНДА` постійно висить зліва. Треба hidden-by-default; `Y` відкриває Team, `U` Global. |
| GAME-SPAWN-001 | Нормальний фактичний spawn, не порожнє поле | ≥3 | IN_PROGRESS | Normal gameplay знову спавнить гравця в плоскому полі. Попередня вимога: тестові weapons мають бути біля фактичного/current spawn; конкретний landmark історично не був зафіксований. Потрібно перенести BASE spawn у реальну придатну зону карти та прив'язати diagnostics до нього. |
| GAME-WEAPONS-001 | 11 pickup classes біля фактичного spawn | ≥3 | IN_PROGRESS | `LocationTest=1` rack source-side прив'язаний до possessed pawn, але normal gameplay його не показує. Новий runtime показує окремі floating/proxy weapons у старому blockout. Потрібен один test contract і pickup test усіх 11. |
| VIS-FP-001 | Production/real weapon visuals без primitive boxes | ≥3 | IN_PROGRESS | AK first-person виглядає як real asset; M1911 та частина world weapons усе ще мають грубі primitive/proxy елементи. Reload і muzzle smoke вже видно, але production binding неповний. |
| UI-TACTICAL-MAP-001 | `M` tactical map без конфлікту | 1 | IN_PROGRESS | Source: `M` map, trap `V`; новий playtest не дав runtime acceptance evidence для карти. |
| GAME-VEHICLE-INPUT-001 | Після exit з авто повертаються WASD/sprint/mouse | 1 | IN_PROGRESS | Recovery coded; новий run показує driving/vehicles, але повторний enter→exit acceptance не зафіксовано. |
| VEH-PICKUP-001 | Pickup/HMMWV має M2 Browning без proxy geometry | ≥2 | IN_PROGRESS | Runtime: пікап має реальну базову машину, але зверху великі чорні/сині primitive елементи; нормального Browning не видно. Потрібно знайти imported M2 asset і зробити його authoritative gun visual. |
| VEH-PICKUP-SPEED-001 | Pickup max speed 120 км/год | 1 | IN_PROGRESS | User runtime повідомляє ~30 км/год. Потрібно перевірити vehicle tuning/units і поставити 120 км/год cap. |
| ASSET-BTR-001 | BTR production model без green box/proxy | ≥2 | IN_PROGRESS | Новий runtime знову показує зелений blocky BTR. Треба повторно інвентаризувати Content і підключити фактичний imported BTR asset, якщо він є, замість старого очікуваного `/Game/Production/...` path. |
| VEH-BTR-SPEED-001 | BTR max speed 90 км/год | 1 | IN_PROGRESS | User requirement: 90 км/год. Поточний tuning треба перевірити та виправити. |
| ASSET-CHARACTER-001 | Production character/skins | ≥2 | IN_PROGRESS | Новий runtime уже показує повну real character model (джинси/кепка/бронежилет), тобто asset path працює хоча б для одного profile. Але це ще не прийнятий бойовий skin/profile і не всі персонажі перевірені. |
| DEBUG-FLIGHT-001 | Керований spectator/free-fly test mode | 1 | IN_PROGRESS | User випадково відкрив Unreal gameplay debugger/spectator-like view через клавішу й побачив персонажа збоку. Не вважати готовим flight mode. Потрібен окремий зрозумілий dev free-fly contract без debugger overlay. |
| LOC-MUSEUM-001 | Museum окремо від Silpo/Culture | ≥4 | IN_PROGRESS | Новий runtime підтвердив: landmark geometry досі змішується/накладається. Старий R13.7 late cleanup ~4.95s + rebuild ~5.10s більше не можна вважати лише ризиком, його треба прибрати з current runtime ownership. |
| LOC-SILPO-001 | Silpo лише на своїй реальній локації | ≥4 | IN_PROGRESS | Interior уже існує, але exterior знову сирий/regressed: parking/signage/banners/фасад втрачено або перекрито. Runtime separation від Museum/Culture не підтверджена. |
| LOC-CULTURE-001 | Culture House лише на своїй реальній локації | ≥4 | IN_PROGRESS | Runtime усе ще дає враження `будинок у будинку`; потрібен один owner, правильна geo position + orientation. |
| LOC-STADIUM-001 | Stadion Oster georeferenced, правильно орієнтований | ≥3 | IN_PROGRESS | Runtime показує box/blockout і криве розміщення. Сам anchor недостатній: потрібні footprint, door/front orientation, road/forest relation та relief по фото. |
| LOC-TERRAIN-001 | Реальний relief, не плоска площина | ≥3 | IN_PROGRESS | Новий runtime показує майже суцільну плоску зелену площину. Segmented terrain source не дає потрібного результату. |
| VIS-HOUSES-001 | Реальні canonical houses замість коробок | ≥3 | IN_PROGRESS | Новий overview показує десятки білих/сірих box houses. Потрібно прибрати visible legacy blockout і використовувати imported/photographic house assets. |
| VIS-GRASS-001 | Натуральне покриття травою | 1 | IN_PROGRESS | Runtime: трава лише окремими рідкими `клубочками`; потрібен landscape/foliage coverage, не випадкові clumps. |
| VIS-FLICKER-001 | Без distant flicker/z-fighting/late rebuild | ≥3 | IN_PROGRESS | User прямо підтвердив мерехтіння здалеку. Пріоритет: duplicate/overlap ownership і z-fighting, потім LOD. |
| LEGACY-BLOCKOUT-001 | Legacy blockout не перекриває current locations/assets | ≥2 | IN_PROGRESS | Новий playtest довів, що blockout реально домінує по карті. High-confidence legacy owners треба вимикати, а не лише cleanup-ити після spawn. |
| VIS-FENCES-001 | Реальні паркани без stretching | 1 | IN_PROGRESS | Real assets є, tiling/placement pending. |
| VIS-STREETLIGHT-001 | Imported streetlight | 1 | CODED_UNTESTED | Source coded; current playtest не був acceptance check. |

## 4. Технічні фікси / build evidence

| ID | Результат | Status | Commit / доказ |
|---|---|---|---|
| BUILD-UE58-001 | Старий UE 5.8 include blocker | VERIFIED BUILD | `37a3a6d5...`. |
| BUILD-TACTICAL-MAP-001 | `C4458 Slot hides class member` | VERIFIED BUILD | Fix `bb7d49b5...`; новий playtest дійшов до повного gameplay. |
| CRASH-OBJECTNAME-001 | Bus-station object-name crash | VERIFIED RUNTIME FOR REPORTED CRASH | Старий name collision не повторився. |
| ASSET-LFS-PREFLIGHT-001 | Git LFS hydration | VERIFIED FOR PREFLIGHT | `54b8c2cd...`; launches проходять hydration. |
| CRASH-WEAPON-FALLBACK-001 | `Pure virtual` у `ApplyRealFallback()` | RUNTIME DID NOT RECUR IN LATEST RUN | GC-safe refs `0213b53b...` + validity guards `6c13f70f...`. Останній довгий gameplay run не впав цим crash; visual fallback issue лишається відкритим. |
| VEHICLE-EXIT-RECOVERY-001 | Restore input stack after vehicle exit | CODED_UNTESTED | `30cae92a...`, `7c4d9138...`. |
| TACTICAL-MAP-SOURCE-001 | `M` map, `V` trap | CODED_UNTESTED | `a6d31480...`, `d9d36c1...`, build fix `bb7d49b5...`. |
| LANDMARK-EXCLUSION-001 | Museum/Silpo/Culture cleanup guard | RUNTIME INSUFFICIENT | `dc07e098...`, `c248578a...`; latest playtest still shows mixed/box geometry, so guard is not a final ownership solution. |
| LAUNCHER-UX-001 | Один user launcher | VERIFIED ENTRY POINT | `START_HERE.cmd` працює як єдиний вхід; normal main menu реально відкрився. Інші RUN scripts лишаються internal. |

## 5. Останній фактичний user run — 2026-08-21 17:44

Підтверджено runtime:
- splash Oster Conflict показується;
- є проміжковий чорний екран перед main menu;
- main menu повернулося і виглядає нормально;
- START має незрозумілий gray/double-START flow;
- gameplay запускається без попереднього `Pure virtual` crash;
- AK real first-person visual працює; reload animation + shot smoke є;
- player character real model уже видно;
- Silpo interior існує;
- pickup можна бачити/використовувати;
- BTR, pickup gun, weapon/world visuals, terrain, houses і landmark layout залишаються неприйнятними;
- великий team chat panel постійно видимий;
- distant flicker є;
- map largely flat, grass sparse;
- Museum/Silpo/Culture separation runtime не досягнута;
- stadium/photo-geography fidelity не досягнута.

Пакет доказів: user `Oster-photo-bag.docx`, 13 screenshots цього run.

## 6. Наступна черга

1. Persist current playtest audit у root.
2. UI: chat hidden-by-default; `Y` Team, `U` Global; fix stale HUD key hints.
3. UI boot/travel: прибрати black gap і double-START confusion.
4. Vehicle tuning: pickup 120 км/год, BTR 90 км/год.
5. Inventory actual Content paths для BTR, M2 Browning і exact weapon meshes; підключити real assets, прибрати visible proxies.
6. Spawn/test contract: нормальний BASE spawn + weapons біля фактичного test spawn.
7. Disable confirmed legacy late museum/blockout owners; Museum/Silpo/Culture = 3 owners, 3 real sites.
8. Silpo exterior regression, stadium orientation/terrain, houses/grass/relief.
9. Distant flicker after duplicate geometry removal.
10. Новий UE 5.8 playtest. Статуси підвищувати лише після runtime evidence.

**Заборона:** ніяких нових декоративних R15/R16 layers до закриття цього backlog.