# OSTER CONFLICT — WORK LEDGER

> Постійний журнал фактичного стану `main`. Runtime-скрін/лог/playtest завжди має пріоритет над code-only твердженням.

## 1. Поточний контекст

- Repository: `valentronus95/OsterConflict`
- Active correction branch: `fix/runtime-playtest-2026-08-22` → `main`
- UE target: 5.8.x Windows
- Project: `OsterConflict/OsterConflict.uproject`
- User-facing launcher: **тільки `START_HERE.cmd`**.
- `RUN_*.cmd` — внутрішні helper scripts. Не створювати новий user-facing launcher під кожну R-версію.
- Persistent evidence: `RUNTIME_AUDIT_2026-08-21.md`, `LEGACY_BLOCKOUT_AUDIT_2026-08-21.md`, `RUNTIME_PLAYTEST_AUDIT_2026-08-21_1744.md`, `RUNTIME_PLAYTEST_AUDIT_2026-08-22.md`.
- User playtest 2026-08-22 + надані runtime screenshots є authoritative evidence для поточного runtime.
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
| UI-MENU-001 | Головне меню стабільне | ≥3 | IN_PROGRESS | 2026-08-22 main menu виглядає нормально; повторна перевірка після merge/build. |
| UI-TRAVEL-001 | Deployment START без freeze/layout jump, 0–100 loading → gameplay | ≥4 | CODED_UNTESTED | 2026-08-22 runtime показав підвисання і зміщення deployment panel. Доданий `OCDeploymentLoadingSubsystem`: blocking overlay, 0–100%, ready request після першого rendered interval, overlay прибирається після possession + закриття deployment UI. |
| UI-CHAT-001 | Team chat `Y`, global chat `U`, панель прихована без вводу | 1 | CODED_UNTESTED | Runtime chat layer coded; acceptance pending. |
| GAME-SPAWN-001 | Фактичний spawn біля Museum, не порожнє поле | ≥4 | CODED_UNTESTED | 2026-08-22 user знову spawn далеко. BASE тепер source-side прив’язаний до `MuseumAnchor()` з offsets ~22–33 m, ground snap, не всередині Museum. |
| GAME-WEAPONS-001 | 11 pickup classes біля фактичного spawn | ≥4 | CODED_UNTESTED | Existing 11-class rack збережений біля primary BASE, який тепер переноситься до Museum test hub. |
| HUD-MINIMAP-001 | Постійна minimap на HUD + `M` full tactical map | 1 | CODED_UNTESTED | Доданий `OCMinimapSubsystem`; використовує той самий `OCTacticalMapSubsystem` render target/projection, player heading marker, приховується при blocking UI/full map. |
| UI-TACTICAL-MAP-001 | `M` tactical map без конфлікту | 1 | CODED_UNTESTED | Canonical Tactical Map key = `M`; runtime acceptance pending. |
| GAME-VEHICLE-INPUT-001 | Після exit з авто повертаються WASD/sprint/mouse | 1 | IN_PROGRESS | Recovery coded; новий acceptance pending. |
| VEH-REVERSE-STEER-001 | Нормальний руль на малому ходу і заднім ходом | 1 | CODED_UNTESTED | 2026-08-22 runtime: reverse майже прямо. Root cause: steering authority → 0 при low speed. Доданий мінімальний steering authority, stronger reverse floor + reverse torque boost. |
| VEH-PICKUP-001 | Pickup/HMMWV має M2 Browning без proxy geometry | ≥3 | CODED_UNTESTED / ASSET CHECK | 2026-08-22 runtime знову показав primitive disc/bar. Тепер old TurretBase/Barrel proxy завжди hidden; canonical M2 first, real R13 MG fallback second; якщо обох нема — hard content error, без fake Browning. Exact asset acceptance pending. |
| VEH-PICKUP-GUNNER-001 | Зрозумілий rear-side gunner entry, solo gunner працює | 1 | CODED_UNTESTED | Rear/turret-side `E` тепер вибирає gunner, front-side — driver; gunner operation більше не вимагає driver seat. |
| VEH-PICKUP-SPEED-001 | Pickup max speed 120 км/год | 1 | CODED_UNTESTED | Server/standalone speed contract coded; speed test pending. |
| ASSET-BTR-001 | BTR production model без proxy | ≥3 | IN_PROGRESS / ASSET IMPORT CHECK | 2026-08-22 runtime BTR production model не підтверджений. Canonical import path існує source-side, але actual UE content/runtime acceptance обов’язковий. |
| VEH-BTR-SPEED-001 | BTR max speed 90 км/год | 1 | CODED_UNTESTED | Runtime speed contract coded; speed test pending. |
| VIS-FP-001 | Production/real weapon visuals без primitive boxes | ≥4 | IN_PROGRESS | AK працює; user повідомив, що pistol/model set досі неповний. Exact asset paths/import/runtime потрібно перевірити. |
| WEAPON-MUZZLE-001 | Visible shot FX стартує з дула | 1 | IN_PROGRESS | 2026-08-22 runtime: visual shot offset to side. Damage trace може лишатися aim/camera-driven; visible start треба rebased на actual weapon muzzle. Exact multicast rebase ще open. |
| WEAPON-TRACER-001 | Немає жовтої круглої «кулі» | 1 | CODED_UNTESTED | Muzzle sphere замінено directional cone/cylinder; tracer radius обмежений до thin streak. |
| ASSET-CHARACTER-001 | Production character/skins | ≥2 | IN_PROGRESS | Real character model є, final combat profile/skins pending. |
| DEBUG-FLIGHT-001 | Керований spectator/free-fly test mode | 1 | IN_PROGRESS | Not final. |
| LOC-MUSEUM-001 | Museum окремо від Silpo/Culture | ≥5 | IN_PROGRESS | 2026-08-22 user знову підтвердив overlap/placement defect. Canonical Museum anchor не змінювати для косметичного workaround; one-owner separation + runtime acceptance mandatory. |
| LOC-SILPO-001 | Silpo лише на своїй реальній локації | ≥5 | IN_PROGRESS | 2026-08-22 still wrong/under-detailed. Canonical geo retained; detailed rebuild і runtime transform acceptance pending. |
| LOC-CULTURE-001 | Culture House лише на своїй реальній локації | ≥5 | IN_PROGRESS | 2026-08-22 still overlaps/wrong. Canonical geo retained; dedicated detail branch + runtime acceptance pending. |
| LOC-STADIUM-001 | Stadion Oster georeferenced, правильно орієнтований | ≥4 | IN_PROGRESS | 2026-08-22 user says stadium still wrong. Existing geo anchor remains authority; detailed stadium reconstruction needs dedicated branch and real-site acceptance. |
| LOC-TERRAIN-001 | Реальний relief, не плоска площина | ≥3 | IN_PROGRESS / DATA BLOCKED | Base still lacks verified terrain heightmap/Landscape elevation data. Не вигадувати relief формулою. |
| VIS-HOUSES-001 | Реальні Oster houses, не однакові huts | ≥4 | IN_PROGRESS | 2026-08-22 distant/local houses still read as repeated generic huts. Requires broader real-house content pass and placement variation. |
| VIS-GRASS-001 | Натуральне покриття травою | 2 | CODED_UNTESTED | 2026-08-22 grass still visually absent/sparse. DenseGroundFoliage grid 13.5 m → 10 m, clumps 2–4 → 3–5, cull range increased; no real mesh now logs hard content error. |
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
| VEHICLE-EXIT-RECOVERY-001 | Restore input stack after vehicle exit | CODED_UNTESTED | Existing source recovery. |
| TACTICAL-MAP-SOURCE-001 | `M` map / `V` trap canonical | CODED_UNTESTED | Existing source/input CI passed previously; actual UE runtime still pending. |
| LANDMARK-STARTUP-001 | Museum/Silpo/Culture без late startup rebuild | CODED_UNTESTED | Coordinator/source cleanup exists; 2026-08-22 runtime still rejects result. |
| DEPLOY-LOADING-20260822 | Deployment START → blocking 0–100 overlay | CODED_UNTESTED | New `OCDeploymentLoadingSubsystem`; routed from `UICommitDeployment()`. |
| BASE-SPAWN-MUSEUM-20260822 | BASE spawn near canonical Museum | CODED_UNTESTED | `OCTeamSpawnPoint.cpp` now uses Museum anchor offsets + ground snap; weapon rack preserved. |
| HUD-MINIMAP-20260822 | Runtime minimap | CODED_UNTESTED | New `OCMinimapSubsystem` + Tactical Map render/projection getters. |
| VEH-REVERSE-20260822 | Low-speed/reverse steering authority | CODED_UNTESTED | `OCVehicleBase.cpp` steering floor/boost. |
| VEH-GUNNER-20260822 | Rear-side gunner entry + solo operation | CODED_UNTESTED | `OCArmedVehicleBase.cpp`. |
| MOUNTED-GUN-PROXY-20260822 | Primitive fake Browning hidden | CODED_UNTESTED | `OCPickupGunTruck.cpp`. |
| FOLIAGE-DENSITY-20260822 | Denser real grass HISM | CODED_UNTESTED | 10 m grid, 3–5 clumps, expanded cull. |
| WEAPON-FX-20260822 | Thin tracer + directional muzzle presentation | CODED_UNTESTED | `OCTransientVisualFX.cpp`; exact weapon-muzzle start remains separate open item. |
| LAUNCHER-UX-001 | Один user launcher | VERIFIED ENTRY POINT | `START_HERE.cmd` remains only user-facing entry point. |

## 5. Останній фактичний user run — 2026-08-22

Підтверджено runtime:
- launch і main menu працюють; main menu візуально прийнятне;
- deployment `СТАРТ` підвисає/зміщує panel перед входом у гру;
- spawn досі далеко від Museum;
- game enters with AK visual, but pistol/BTR/asset coverage incomplete;
- pickup has invalid turret/proxy presentation and gunner interaction unclear;
- reverse steering weak;
- grass absent/sparse;
- Museum/Silpo/Culture separation not achieved;
- Stadium location/fidelity rejected;
- Silpo and several buildings still blockout-like;
- repeated generic houses remain;
- distant objects flicker;
- roads/sidewalks too convex;
- marked building stairs/detail rejected;
- HUD minimap absent;
- shot/tracer visual offset from muzzle and reads as round yellow element.

Пакет доказів: user runtime screenshots цього turn + `RUNTIME_PLAYTEST_AUDIT_2026-08-22.md`.

Усі source fixes після цього run залишаються `CODED_UNTESTED`/`IN_PROGRESS`, доки новий UE 5.8 build/playtest їх не підтвердить.

## 6. Наступна черга

1. Build/CI correction branch; compile blockers have priority over content polish.
2. Runtime acceptance: deployment 0–100, Museum spawn/rack, minimap, rear gunner, reverse steering, grass, tracer.
3. Exact M2/BTR/pistol asset import/path acceptance. Missing assets are blockers, not permission to show primitive fake models.
4. Global landmark separation acceptance at canonical Museum/Silpo/Culture anchors.
5. Separate detail branches: Museum, Stadium, Silpo, Culture House; merge without moving canonical geo anchors.
6. Distant flicker/duplicate geometry and roads/sidewalk geometry pass.
7. Real Oster house variation and large-building/stairs detail pass.
8. Exact visible shot origin rebase to weapon muzzle.

**Заборона:** ніяких нових декоративних R15/R16 layers до закриття цього backlog.