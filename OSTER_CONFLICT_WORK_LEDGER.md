# OSTER CONFLICT — WORK LEDGER

> Постійний журнал фактичного стану `main`. Runtime-скрін/лог/playtest завжди має пріоритет над code-only твердженням.

## 1. Поточний контекст

- Repository: `valentronus95/OsterConflict`
- Active correction branch: `fix/frontend-single-owner-pass-28-20260823` → `main`
- UE target: 5.8.x Windows
- Project: `OsterConflict/OsterConflict.uproject`
- User-facing launcher: **тільки `START_HERE.cmd`**.
- `RUN_*.cmd` — внутрішні helper scripts. Не створювати новий user-facing launcher під кожну R-версію.
- Persistent evidence: `RUNTIME_AUDIT_2026-08-21.md`, `LEGACY_BLOCKOUT_AUDIT_2026-08-21.md`, `RUNTIME_PLAYTEST_AUDIT_2026-08-21_1744.md`, `RUNTIME_PLAYTEST_AUDIT_2026-08-22.md`.
- User playtest 2026-08-23 повторив той самий Slate/SlateCore assertion уже на `main` commit `f2d397f8b9a2348576dcf96b0c20522a8a8c8d8f` після Pass 26: `Array index out of bounds: -808103970 into an array of size 0`. Pass 26 lifecycle fence не усунув crash, тому ця runtime evidence має пріоритет над зеленим CI.
- Не створювати нові декоративні R15/R16 layers, доки поточний runtime backlog не закритий.

## 2. Статусні правила

- `IN_PROGRESS` — runtime уже показав проблему або fix ще не підтверджено/не завершено.
- `CODED_UNTESTED` — source fix є, але UE runtime його ще не підтвердив.
- `VERIFIED BUILD` — конкретний compile blocker підтверджено усуненим фактичним наступним build/run.
- `VERIFIED RUNTIME` — тільки після фактичного UE runtime/playtest.
- Один landmark/site = один authoritative placement owner; cleanup не замінює ownership.
- Generic fallback не може видаватися за exact production asset.
- Authored game-visual asset може закрити source-side visual gap лише як `CODED_UNTESTED`; runtime scale/pivot/material/placement acceptance все одно обов’язковий.

## 3. Активні вимоги

| ID | Вимога | Repeat | Status | Фактичний стан / що лишилось |
|---|---|---:|---|---|
| UI-BOOT-001 | Splash → main menu без чорної паузи | 1 | CODED_UNTESTED | MoviePlayer startup loading screen coded; потрібен UE 5.8 startup acceptance. |
| UI-MENU-001 | Головне меню стабільне | ≥5 | IN_PROGRESS | 2026-08-23 runtime повторив той самий Slate/SlateCore array assertion вже на Pass 26 (`f2d397f8...`). Новий concrete suspect: R13 overlay створював live UMG widgets через `NewObject` після побудови native `WidgetTree`, тоді як стабільний `UOCGameUIRootWidget` всюди використовує `WidgetTree->ConstructWidget`. Pass 27 переводить весь R13 frontend на WidgetTree ownership; runtime acceptance обов’язковий. |
| UI-TRAVEL-001 | Deployment START без freeze/layout jump, 0–100 loading → gameplay | ≥5 | CODED_UNTESTED | 2026-08-22 runtime повторно показав підвисання. `OCDeploymentLoadingSubsystem` дає blocking 0–100 overlay; dense foliage більше не робить десятки тисяч traces/HISM inserts одним синхронним кадром, а розноситься batch-ами. Новий UE runtime acceptance обов’язковий. |
| UI-CHAT-001 | Team chat `Y`, global chat `U`, панель прихована без вводу | 1 | CODED_UNTESTED | Runtime chat layer coded; acceptance pending. |
| GAME-SPAWN-001 | Фактичний spawn біля Museum, не порожнє поле | ≥5 | CODED_UNTESTED | User повторив дефект. Поточний source BASE прив’язаний до `MuseumAnchor()`; primary offset ≈17 m, ground snap, secondary також лишається біля Museum. Новий runtime має підтвердити, що саме цей transform використовується після deployment. |
| GAME-WEAPONS-001 | 11 pickup classes біля фактичного spawn | ≥5 | CODED_UNTESTED | 11-class rack збережений біля primary BASE. Нормальний launcher тепер окремо відкриває required real weapon assets у fresh UE process і блокує playtest, якщо хоча б один required visual не завантажується. |
| HUD-MINIMAP-001 | Постійна minimap на HUD + `M` full tactical map | 1 | CODED_UNTESTED | Доданий `OCMinimapSubsystem`; використовує той самий `OCTacticalMapSubsystem` render target/projection, player heading marker, приховується при blocking UI/full map. |
| UI-TACTICAL-MAP-001 | `M` tactical map без конфлікту | 1 | CODED_UNTESTED | Canonical Tactical Map key = `M`; runtime acceptance pending. |
| GAME-VEHICLE-INPUT-001 | Після exit з авто повертаються WASD/sprint/mouse | 1 | IN_PROGRESS | Recovery coded; новий acceptance pending. |
| VEH-REVERSE-STEER-001 | Нормальний руль на малому ходу і заднім ходом | 1 | CODED_UNTESTED | 2026-08-22 runtime: reverse майже прямо. Root cause: steering authority → 0 при low speed. Доданий мінімальний steering authority, stronger reverse floor + reverse torque boost. |
| VEH-PICKUP-001 | Pickup/HMMWV має M2 Browning без proxy geometry | ≥4 | CODED_UNTESTED / ASSET CHECK | User повторно показав/описав відсутність production models. Old TurretBase/Barrel proxy hidden; normal launcher відновлює local HMMWV/M2 source, імпортує їх і робить fresh-process load verification. Якщо HMMWV/M2 не відкриваються, normal playtest тепер не повинен стартувати. |
| VEH-PICKUP-GUNNER-001 | Зрозумілий rear-side gunner entry, solo gunner працює | 1 | CODED_UNTESTED | Rear/turret-side `E` тепер вибирає gunner, front-side — driver; gunner operation більше не вимагає driver seat. |
| VEH-PICKUP-SPEED-001 | Pickup max speed 120 км/год | 1 | CODED_UNTESTED | Server/standalone speed contract coded; speed test pending. |
| ASSET-BTR-001 | BTR production model без proxy | ≥4 | CODED_UNTESTED / ASSET IMPORT CHECK | User повторно показав proxy BTR. Normal launcher тепер не допускає звичайний playtest, доки local BTR source не імпортований і canonical `/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus` не відкрився у другому fresh UE process. Runtime scale/material/ground contact acceptance ще потрібний. |
| VEH-BTR-SPEED-001 | BTR max speed 90 км/год | 1 | CODED_UNTESTED | Runtime speed contract coded; speed test pending. |
| VIS-FP-001 | Production/real weapon visuals без primitive boxes | ≥5 | IN_PROGRESS / ASSET PREFLIGHT | User повторно показав M1911/weapon rack як primitive geometry. R13/AK LFS hydration збережена; pass 4 додає fresh UE preflight для AK, MP5, M1911, M700, M14, MAC-10, TEC-9, Lever, real M249/shotgun fallback та launcher. Якщо вони не loadable, normal playtest блокується. Exact M249/Remington production identity лишається окремим content gap. |
| WEAPON-MUZZLE-001 | Visible shot FX стартує з дула | 2 | CODED_UNTESTED | Знайдений конкретний дефект: tracer передавався в FX як target-side partial streak, тому старий camera-distance guard відмовлявся rebase-ити його на muzzle. Pass 4 приймає local aim-ray, пріоритетно бере Muzzle/Barrel socket або barrel-named visible component і перебазовує local streak на кінець ствола. Runtime acceptance pending. |
| WEAPON-TRACER-001 | Немає жовтої круглої «кулі» | 1 | CODED_UNTESTED | Muzzle sphere замінено directional cone/cylinder; tracer radius обмежений до thin streak. Після muzzle rebase streak також обмежений 9 m, щоб не стати суцільним laser beam. |
| ASSET-CHARACTER-001 | Production character/skins | ≥2 | IN_PROGRESS | Real character model є, final combat profile/skins pending. |
| DEBUG-FLIGHT-001 | Керований spectator/free-fly test mode | 1 | IN_PROGRESS | Not final. |
| LOC-MUSEUM-001 | Museum окремо від Silpo/Culture | ≥5 | IN_PROGRESS | 2026-08-22 user знову підтвердив overlap/placement defect. Canonical Museum anchor не змінювати для косметичного workaround; one-owner separation + runtime acceptance mandatory. |
| LOC-SILPO-001 | Silpo лише на своїй реальній локації | ≥5 | IN_PROGRESS | 2026-08-22 still wrong/under-detailed. Canonical geo retained; detailed rebuild і runtime transform acceptance pending. |
| LOC-CULTURE-001 | Culture House лише на своїй реальній локації | ≥5 | IN_PROGRESS | 2026-08-22 still overlaps/wrong. Canonical geo retained; dedicated detail branch + runtime acceptance pending. |
| LOC-STADIUM-001 | Stadion Oster georeferenced, правильно орієнтований | ≥4 | IN_PROGRESS | 2026-08-22 user says stadium still wrong. Existing geo anchor remains authority; detailed stadium reconstruction needs dedicated branch and real-site acceptance. |
| LOC-TERRAIN-001 | Реальний relief, не плоска площина | ≥3 | IN_PROGRESS / DATA BLOCKED | Base still lacks verified terrain heightmap/Landscape elevation data. Не вигадувати relief формулою. |
| VIS-HOUSES-001 | Реальні Oster houses, не однакові huts | ≥4 | IN_PROGRESS | 2026-08-22 distant/local houses still read as repeated generic huts. Requires broader real-house content pass and placement variation. |
| VIS-GRASS-001 | Натуральне покриття травою | 2 | CODED_UNTESTED | 2026-08-22 grass still visually absent/sparse. DenseGroundFoliage grid 13.5 m → 10 m, clumps 2–4 → 3–5, cull range increased; no real mesh now logs hard content error. Population is now batched to avoid a deployment-frame stall. |
| VIS-FLICKER-001 | Без distant flicker/z-fighting/late rebuild | ≥4 | IN_PROGRESS | 2026-08-22 marked horizon still flickers. Duplicate/late-owner cleanup remains suspect; new runtime check required after landmark/blockout pass. |
| VIS-ROADS-001 | Roads/sidewalks не надмірно випуклі | 1 | IN_PROGRESS | 2026-08-22 runtime confirms excessive raised/convex profile. Needs geometry pass. |
| VIS-LARGE-BUILDING-001 | Marked large building/stairs geometrically clean | 1 | IN_PROGRESS | 2026-08-22 marked building has crooked stairs + low detail. Needs dedicated geometry/detail correction. |
| LEGACY-BLOCKOUT-001 | Legacy blockout не перекриває current locations/assets | ≥3 | IN_PROGRESS | Runtime still shows generic/overlapping content. Cleanup source passes exist but are not accepted. |
| VIS-FENCES-001 | Реальні паркани без stretching | 1 | CODED_UNTESTED | Tiled real fence source exists; acceptance pending. |
| VIS-STREETLIGHT-001 | Imported streetlight | 1 | CODED_UNTESTED | Source coded; acceptance pending. |

## 4. Технічні фікси / build evidence

| ID | Результат | Status | Commit / доказ |
|---|---|---|---|
| BUILD-UE58-001 | Старий UE 5.8 include blocker | VERIFIED BUILD | `37a3a6d5...`. |
| BUILD-TACTICAL-MAP-001 | `C4458 Slot hides class member` | VERIFIED BUILD | `bb7d49b5...`; попередній playtest дійшов до gameplay. |
| CRASH-OBJECTNAME-001 | Bus-station object-name crash | VERIFIED RUNTIME FOR REPORTED CRASH | Старий name collision не повторився. |
| ASSET-LFS-PREFLIGHT-001 | Git LFS hydration | VERIFIED FOR PREFLIGHT | `54b8c2cd...`. |
| CRASH-WEAPON-FALLBACK-001 | `Pure virtual` у `ApplyRealFallback()` | RUNTIME DID NOT RECUR IN LATEST RUN | GC-safe refs + guards. |
| CRASH-FRONTEND-SLATE-20260823 | Frontend interaction → Slate/SlateCore array assertion | CODED_UNTESTED | Pass 26 runtime FAILED: exact assertion repeated on `f2d397f8...`. Pass 27 aligns the R13 overlay with the native root lifecycle by constructing every UMG widget through `Root->WidgetTree->ConstructWidget`; direct `NewObject<UWidget>` construction is forbidden in this frontend. Launcher also auto-prints PASS markers + last 180 gameplay-log lines on non-zero exit. Runtime acceptance pending. |
| VEHICLE-EXIT-RECOVERY-001 | Restore input stack after vehicle exit | CODED_UNTESTED | Existing source recovery. |
| TACTICAL-MAP-SOURCE-001 | `M` map / `V` trap canonical | CODED_UNTESTED | Existing source/input CI passed previously; actual UE runtime still pending. |
| LANDMARK-STARTUP-001 | Museum/Silpo/Culture без late startup rebuild | CODED_UNTESTED | Coordinator/source cleanup exists; 2026-08-22 runtime still rejects result. |
| DEPLOY-LOADING-20260822 | Deployment START → blocking 0–100 overlay | CODED_UNTESTED | `OCDeploymentLoadingSubsystem`; routed from `UICommitDeployment()`. |
| START-FOLIAGE-BATCH-20260822 | Dense foliage не блокує один deployment frame | CODED_UNTESTED | Full-grid population розбито на 96-cell batches через timer; попередній single-frame workload прибраний. |
| BASE-SPAWN-MUSEUM-20260822 | BASE spawn near canonical Museum | CODED_UNTESTED | `OCTeamSpawnPoint.cpp` primary ≈17 m від Museum anchor + ground snap; 11-weapon rack preserved. |
| HUD-MINIMAP-20260822 | Runtime minimap | CODED_UNTESTED | New `OCMinimapSubsystem` + Tactical Map render/projection getters. |
| VEH-REVERSE-20260822 | Low-speed/reverse steering authority | CODED_UNTESTED | `OCVehicleBase.cpp` steering floor/boost. |
| VEH-GUNNER-20260822 | Rear-side gunner entry + solo operation | CODED_UNTESTED | `OCArmedVehicleBase.cpp`. |
| MOUNTED-GUN-PROXY-20260822 | Primitive fake Browning hidden | CODED_UNTESTED | `OCPickupGunTruck.cpp`. |
| VEHICLE-ASSET-FRESHLOAD-20260822 | HMMWV/M2/BTR fresh-process gate | CODED_UNTESTED | Production importer must import all 3 canonical assets, then second Unreal process must reopen them before normal game launch. |
| WEAPON-ASSET-GATE-20260822 | Required real weapon asset fresh UE gate | CODED_UNTESTED | Pass 4 `verify_required_weapon_assets.py`; normal launcher stops before gameplay if required AK/R13 visual asset cannot load. |
| FOLIAGE-DENSITY-20260822 | Denser real grass HISM | CODED_UNTESTED | 10 m grid, 3–5 clumps, expanded cull. |
| WEAPON-FX-20260822 | Thin tracer + directional muzzle presentation | CODED_UNTESTED | `OCTransientVisualFX.cpp`; pass 4 fixes target-side tracer rebase, local aim-ray ownership check and barrel/socket preference. |
| SOURCE-R10-FALSEPOSITIVE-20260822 | R10 verifier no longer treats unrelated namespace `Slot` locals as old UI shadow blocker | CODED_UNTESTED | Dedicated `OCGameUIRootWidget.cpp` shadow check retained; project-wide false-positive token removed. |
| LAUNCHER-UX-001 | Один user launcher | VERIFIED ENTRY POINT | `START_HERE.cmd` remains only user-facing entry point. |

## 5. Останній фактичний user run — 2026-08-23

Підтверджено runtime:
- `main` commit `6d1ff2605573c4a1cdcf51e132ac56f986db216a` підтягнутий і використаний у запуску;
- UE 5.8 editor build успішний, тобто це не compile blocker;
- required weapon fresh-process preflight проходить до запуску normal frontend;
- normal frontend запускається через D3D11/SM5 safe renderer;
- frontend interaction після Pass 25 завершується assertion crash: `(Index >= 0) & (Index < ArrayNum)`, negative index `-808103970`, array size `0`;
- call stack концентрується у `CoreUObject` + великій кількості `SlateCore`/`Slate` frames;
- наданий launcher output не містить самого `R14_CURRENT_GAMEPLAY.log`, тому конкретний останній PASS24/PASS25 marker перед crash у цьому evidence не визначений.

Попередні gameplay/location дефекти з run 2026-08-22 цим коротким frontend run не перевірені повторно і залишаються в попередніх статусах.

Pass 26 після цього crash є лише `CODED_UNTESTED`, доки новий UE 5.8 build/playtest не підтвердить frontend.

## 6. Наступна черга

1. Pass 26 source/CI + локальний UE 5.8 build; потім перевірити `СТАРТ / МЕРЕЖЕВА ГРА / НАЛАШТУВАННЯ / ВИЙТИ З ГРИ` без Slate crash.
2. `СТАРТ` має відкрити `СТВОРЕННЯ СЕРВЕРА`; після цього `СТВОРИТИ СЕРВЕР` має пройти у Deployment без повторного frontend crash.
3. Runtime acceptance: deployment 0–100 без freeze, Museum spawn/rack, minimap, rear gunner, reverse steering, grass, tracer саме з дула.
4. Під час `START_HERE.cmd → 1` normal launcher має зупинятись ДО гри, якщо required weapon visuals або HMMWV/M2/BTR не відкриваються у fresh UE process.
5. Exact M249/Remington production identity лишається content gap; real R13 fallback допускається лише як позначений fallback, не як exact production asset.
6. Global landmark separation acceptance at canonical Museum/Silpo/Culture anchors.
7. Separate detail branches: Museum, Stadium, Silpo, Culture House; merge without moving canonical geo anchors.
8. Distant flicker/duplicate geometry and roads/sidewalk geometry pass.
9. Real Oster house variation and large-building/stairs detail pass.

**Заборона:** ніяких нових декоративних R15/R16 layers до закриття цього backlog.
