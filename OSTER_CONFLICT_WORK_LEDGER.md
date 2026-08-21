# OSTER CONFLICT — WORK LEDGER

> Постійний журнал фактичного стану `main`. Runtime-скрін/лог/playtest завжди має пріоритет над code-only твердженням.

## 1. Поточний контекст

- Repository: `valentronus95/OsterConflict`
- Active branch: `main`
- UE target: 5.8.x Windows
- Project: `OsterConflict/OsterConflict.uproject`
- User-facing launcher: **тільки `START_HERE.cmd`**.
- `RUN_*.cmd` — внутрішні helper scripts. Не створювати новий user-facing launcher під кожну R-версію.
- 2026-08-21 runtime package з 14 скрінів лишається authoritative evidence для відкритих visual/gameplay regressions.
- Persistent evidence: `RUNTIME_AUDIT_2026-08-21.md`, `LEGACY_BLOCKOUT_AUDIT_2026-08-21.md`.
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
| LOC-MUSEUM-001 | Музей окремо від Silpo/Culture | ≥3 | IN_PROGRESS | Persistent exclusion guard є (`dc07e098...`, `c248578a...`), але legacy R13.7 museum pipeline має late cleanup ~4.95s і rebuild ~5.10s. Потрібен runtime після 8+ секунд. |
| LOC-STADIUM-001 | Окремий hard-georeferenced Stadion Oster | ≥2 | CODED_UNTESTED | Canonical anchor `50.949360, 30.884660`, окремий owner і stadium layers у `main`; потрібен runtime. |
| LOC-TERRAIN-001 | Спуск і нижчі хати за музеєм | ≥2 | CODED_UNTESTED | Segmented terrain + lower residential district coded; потрібен runtime. |
| LOC-SILPO-001 | Silpo лише на своїй локації | ≥3 | IN_PROGRESS | Photo-driven owner + exclusion guard coded; runtime separation pending. |
| LOC-CULTURE-001 | Будинок культури лише на своїй локації | ≥3 | IN_PROGRESS | Окремий owner + exclusion guard coded; runtime pending. |
| LOC-COLLEGE-001 | Коледж не у вигаданій точці | ≥2 | IN_PROGRESS | Legacy placement audit pending after critical runtime fixes. |
| LOC-PARK-001 | Парк не у вигаданій точці | ≥2 | IN_PROGRESS | Legacy placement audit pending. |
| LOC-WATERTOWER-001 | Водонапірна вежа біля Silpo | 1 | TODO | Не починати до закриття runtime backlog. |
| VIS-FLICKER-001 | Без late rebuild/flicker після входу | ≥2 | IN_PROGRESS | Guard працює source-side, але museum 4.95/5.10 late mutation лишається high-risk. |
| UI-MENU-001 | Головне меню не смикається і не підміняється gameplay | ≥3 | IN_PROGRESS | 2026-08-21 direct `LocationTest` показав deployment UI поверх живого світу, бо diagnostic route використовує `-NoFrontend`. Це не normal frontend. `START_HERE.cmd` змінено: пункт 1 тепер **ЗВИЧАЙНА ГРА** з головним меню, пункт 2 — явно **ТЕХНІЧНИЙ ТЕСТ БЕЗ ГОЛОВНОГО МЕНЮ** (`af2c24dd...`). Normal frontend треба повторно перевірити. |
| UI-TRAVEL-001 | Після START немає старого/сірого меню | ≥2 | CODED_UNTESTED | Normal frontend → TEAM runtime test pending. |
| GAME-WEAPONS-001 | 11 pickup classes біля фактичного spawn | ≥2 | IN_PROGRESS | `LocationTest=1` rack прив'язаний до possessed pawn; runtime був перерваний crash fallback subsystem до приймальної перевірки. |
| VIS-FP-001 | Без primitive/debug geometry біля зброї | ≥2 | IN_PROGRESS | Real R13 fallback coded. 2026-08-21 runtime crash стався всередині fallback path до візуальної перевірки; див. `CRASH-WEAPON-FALLBACK-001`. |
| ASSET-BTR-001 | BTR без green box/proxy | 1 | IN_PROGRESS | Exact `/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus` asset відсутній. |
| ASSET-CHARACTER-001 | Production character/skins | 1 | IN_PROGRESS | Quantum body/arms є через LFS; launcher hydration проходить; runtime binding pending. |
| GAME-VEHICLE-INPUT-001 | Після exit з авто повертаються WASD/sprint/mouse | 1 | IN_PROGRESS | Recovery coded (`30cae92a...`, `7c4d9138...`); runtime pending. |
| UI-TACTICAL-MAP-001 | `M` tactical map без конфлікту | 1 | IN_PROGRESS | `M` map, trap перенесено на `V`; compile blocker усунено, але runtime map ще не перевірено. |
| VIS-HOUSES-001 | Реальні house meshes замість кубів | ≥2 | CODED_UNTESTED | `SM_House_Var01/02` coded; runtime pending. |
| VIS-FENCES-001 | Реальні паркани без 12× stretching | 1 | IN_PROGRESS | Real assets є, tiling fix ще pending. |
| VIS-STREETLIGHT-001 | Використовувати imported streetlight | 1 | CODED_UNTESTED | `SM_StreetLight` coded; runtime pending. |
| LEGACY-BLOCKOUT-001 | Legacy blockout не перекриває current locations/assets | 1 | IN_PROGRESS | Audit `b137bee6...`; high-risk R13.7 museum late mutation + raw recovered building site `(-69000,64500,0)`. |

## 4. Технічні фікси / build evidence

| ID | Результат | Status | Commit / доказ |
|---|---|---|---|
| BUILD-UE58-001 | Старий UE 5.8 include blocker | VERIFIED BUILD | `37a3a6d5...`; раніше user build `Result: Succeeded`. |
| BUILD-TACTICAL-MAP-001 | `C4458 Slot hides class member` | VERIFIED BUILD | Fix `bb7d49b5...`. Наступний user launch 2026-08-21 уже дійшов до runtime/deployment screen і crash у іншому subsystem, отже цей compile blocker пройдено. |
| CRASH-OBJECTNAME-001 | `Cannot replace existing object of a different class` | VERIFIED RUNTIME FOR REPORTED CRASH | Старий bus-station name collision усунено. |
| ASSET-LFS-PREFLIGHT-001 | `git lfs pull` + critical asset size checks | VERIFIED FOR PREFLIGHT | `54b8c2cd...`; user launches проходять hydration stage. |
| GAME-WEAPON-ALL-001 | 11-class LocationTest rack | CODED_UNTESTED | Possessed-pawn placement + anti-armor + legacy pickup suppression. |
| VEHICLE-EXIT-RECOVERY-001 | Restore input stack after vehicle exit | CODED_UNTESTED | `30cae92a...`, `7c4d9138...`. |
| TACTICAL-MAP-SOURCE-001 | `M` map, `V` trap | CODED_UNTESTED | `a6d31480...`, `d9d36c1...`, compile fix `bb7d49b5...`. |
| LANDMARK-EXCLUSION-001 | Museum/Silpo/Culture startup + late-spawn guard | CODED_UNTESTED | `dc07e098...`, `c248578a...`. |
| VERIFY-RUNTIME-RECOVERY-001 | Source structural recovery guard | CODED_UNTESTED | `004b04ed...`; CI integration `6ad32abc...`. Source guard ≠ runtime proof. |
| LAUNCHER-VERIFY-001 | Version-neutral START_HERE no longer blocked by stale R14.7 text check | CODED_UNTESTED | `bc106688...`; later launch passed far enough to enter runtime, so old false-positive did not recur. |
| LAUNCHER-UX-001 | Один зрозумілий user entry point | CODED_UNTESTED | `START_HERE.cmd`: option 1 = normal frontend; option 2 = direct technical LocationTest without main menu. Commit `af2c24dd774a6c1281544e13d15e004b6ffa6598`. |
| CRASH-WEAPON-FALLBACK-001 | `Pure virtual function being called` у `ApplyRealFallback()` | CODED_UNTESTED | User runtime stack: `OCRealWeaponFallbackSubsystem.cpp:111`, тобто `Mesh->GetBounds()`. Loaded fallback meshes були timer-held `TObjectPtr` без reflected `UPROPERTY` lifetime. Header тепер тримає 4 meshes як `UPROPERTY(Transient)` (`0213b53b90a9c7ea9ac5f1328257394abb480b3f`); cpp додатково використовує `IsValid`, skips destroying weapons/components і `.Get()` (`6c13f70f457a0ee9a09119b4ea53dcc4355c4804`). Повторний runtime pending. |

## 5. Останній фактичний user run — 2026-08-21

Підтверджено:
- LFS hydration пройшла;
- ownership verifier більше не зупинив запуск;
- UE build пройшов попередній tactical-map `C4458` blocker, бо runtime відкрився;
- direct LocationTest показав deployment UI поверх уже завантаженого gameplay world. Це diagnostic `-NoFrontend` route, а не normal frontend;
- runtime впав з `Pure virtual function being called` у `UOCRealWeaponFallbackSubsystem::ApplyRealFallback()` на `Mesh->GetBounds()`.

Після цього source-side:
- fallback mesh lifetime виправлено GC-safe references + validity guards (`0213b53b...`, `6c13f70f...`);
- `START_HERE.cmd` перестав підсовувати direct LocationTest як пункт 1. Normal main-menu launch тепер пункт 1 (`af2c24dd...`).

Жоден з runtime-дефектів weapons/BTR/character/vehicle/map/landmarks не переводити в `VERIFIED`, доки не буде нового playtest.

## 6. Наступна черга

1. User: GitHub Desktop → Fetch/Pull current `main`.
2. `START_HERE.cmd` → **1. ЗВИЧАЙНА ГРА**. Перевірити, що спочатку видно попереднє головне меню, а gameplay не працює за ним.
3. Через головне меню запустити match/TEAM і перевірити, що `Pure virtual` crash більше не повторюється.
4. Після normal path: `START_HERE.cmd` → **2. ТЕХНІЧНИЙ ТЕСТ ГРИ** для LocationTest diagnostics.
5. У LocationTest перевірити 11 weapons, primitive visuals, `M` map, vehicle exit input, Museum/Silpo/Culture після 8+ секунд, BTR/character assets.
6. Після runtime evidence оновити statuses; до цього не робити R15/R16 decoration.
