# OSTER CONFLICT — WORK LEDGER

> Постійний журнал фактичного стану `main`. Runtime-скрін/лог/playtest завжди має пріоритет над code-only твердженням.

## 1. Поточний контекст

- Repository: `valentronus95/OsterConflict`
- Active correction branch: `fix/museum-visible-spawn-weapon-palette-pass-37-20260823` → `main`
- UE target: 5.8.x Windows
- Project: `OsterConflict/OsterConflict.uproject`
- User-facing launcher: **тільки `START_HERE.cmd`**.
- `RUN_*.cmd` — внутрішні helper scripts. Не створювати новий user-facing launcher під кожну R-версію.
- Persistent evidence: `RUNTIME_AUDIT_2026-08-21.md`, `LEGACY_BLOCKOUT_AUDIT_2026-08-21.md`, `RUNTIME_PLAYTEST_AUDIT_2026-08-21_1744.md`, `RUNTIME_PLAYTEST_AUDIT_2026-08-22.md`, `RUNTIME_PLAYTEST_AUDIT_2026-08-23_PASS35.md`, `OsterConflict/Docs/WorkReports/RUNTIME_PLAYTEST_AUDIT_2026-08-23_PASS37.md`.
- Latest user playtest 2026-08-23 is authoritative over green Pass 35/36 CI: gameplay still opens beside the physical BASE rack in an empty/flat field, Museum is not visibly present in the initial view, most rack weapons are still white/grey while AK is textured, and the supplied screenshot shows `FPS 27` below the 30 FPS target.
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
| UI-MENU-001 | Головне меню стабільне | ≥7 | CODED_UNTESTED | Pass 29 static START route дозволив останнім playtest дійти до gameplay; старий Slate START crash у цьому run не повторився, але окремий повний frontend acceptance ще потрібний. |
| UI-TRAVEL-001 | Deployment START без freeze/layout jump, 0–100 loading → gameplay | ≥5 | CODED_UNTESTED | Останній run дійшов до gameplay. Blocking loading + batched startup збережені; Pass 36 прибрав progressive full-sector foliage population у normal LowCPU flow. |
| UI-CHAT-001 | Team chat `Y`, global chat `U`, панель прихована без вводу | 1 | CODED_UNTESTED | Runtime chat layer coded; acceptance pending. |
| GAME-SPAWN-001 | Фактичний spawn біля Museum, не порожнє поле | ≥7 | CODED_UNTESTED | Pass 36 runtime знову відхилений: BASE/rack візуально читається як поле далеко від Museum. Pass 37 переносить primary BASE з ≈41 m на ≈27.8 m front-side approach, вирівнює yaw прямо на Museum та guard приймає 20–45 m band. Runtime acceptance pending. |
| GAME-WEAPONS-001 | 11 pickup classes біля фактичного spawn | ≥7 | CODED_UNTESTED | 11-class rack фізично є, але Pass 36 runtime знову показує більшість real meshes білими/сірими. Pass 37 окремо палітрує known incomplete restored Stein payloads; AK authored appearance не чіпається. Exact missing texture payload не підміняти твердженням про texture restoration. |
| HUD-MINIMAP-001 | Постійна minimap на HUD + `M` full tactical map | 1 | CODED_UNTESTED | Доданий `OCMinimapSubsystem`; використовує той самий `OCTacticalMapSubsystem` render target/projection, player heading marker, приховується при blocking UI/full map. |
| UI-TACTICAL-MAP-001 | `M` tactical map без конфлікту та з видимим player marker | 2 | CODED_UNTESTED | Pass 35 піднімає існуючий player marker до Z=60/size26; новий runtime acceptance лишається обов’язковим. |
| GAME-VEHICLE-INPUT-001 | Після exit з авто повертаються WASD/sprint/mouse | 1 | IN_PROGRESS | Recovery coded; новий acceptance pending. |
| VEH-REVERSE-STEER-001 | Нормальний руль на малому ходу і заднім ходом | 1 | CODED_UNTESTED | 2026-08-22 runtime: reverse майже прямо. Root cause: steering authority → 0 при low speed. Доданий мінімальний steering authority, stronger reverse floor + reverse torque boost. |
| VEH-PICKUP-001 | Pickup/HMMWV має M2 Browning без proxy geometry | ≥4 | CODED_UNTESTED / ASSET CHECK | Old TurretBase/Barrel proxy hidden; normal launcher відновлює local HMMWV/M2 source, імпортує їх і робить fresh-process load verification. |
| VEH-PICKUP-GUNNER-001 | Зрозумілий rear-side gunner entry, solo gunner працює | 1 | CODED_UNTESTED | Rear/turret-side `E` вибирає gunner, front-side — driver; gunner operation не вимагає driver seat. |
| VEH-PICKUP-SPEED-001 | Pickup max speed 120 км/год | 1 | CODED_UNTESTED | Server/standalone speed contract coded; speed test pending. |
| ASSET-BTR-001 | BTR production model без proxy | ≥4 | CODED_UNTESTED / ASSET IMPORT CHECK | Normal launcher не допускає playtest, доки canonical BTR asset не відкрився у fresh UE process; runtime scale/material/ground-contact acceptance ще потрібний. |
| VEH-BTR-SPEED-001 | BTR max speed 90 км/год | 1 | CODED_UNTESTED | Runtime speed contract coded; speed test pending. |
| VIS-FP-001 | Production/real weapon visuals без primitive/white material presentation | ≥7 | CODED_UNTESTED / ASSET PREFLIGHT | Pass 36 припустив, що будь-який non-null/non-Default material valid; runtime це спростував. Restored Stein folders мають mesh/WPN payloads без standalone material/texture payload beside them. Pass 37 forces visible metal/wood/polymer palette only for those known incomplete restored models and preserves AK. |
| WEAPON-MUZZLE-001 | Visible shot FX стартує з дула | 2 | CODED_UNTESTED | Muzzle/socket rebase coded; runtime acceptance pending. |
| WEAPON-TRACER-001 | Немає жовтої круглої «кулі» | 1 | CODED_UNTESTED | Thin directional tracer coded; runtime acceptance pending. |
| ASSET-CHARACTER-001 | Production character/skins | ≥2 | IN_PROGRESS | Real character model є, final combat profile/skins pending. |
| DEBUG-FLIGHT-001 | Керований spectator/free-fly test mode | 1 | IN_PROGRESS | Not final. |
| LOC-MUSEUM-001 | Museum окремо від Silpo/Culture і реально присутній у runtime | ≥7 | IN_PROGRESS | Pass 35 owner-count recovery passed CI but user runtime again shows no visible Museum. Concrete false-positive: Pass 35 proves only R137/R138 actor counts. Pass 37 requires >=12 registered visible `MuseumStructural` components near `MuseumAnchor`, rebuilds stale/empty R138 and retires late duplicate R138 owners through >5.35 s startup window. Broader photo fidelity remains IN_PROGRESS. |
| LOC-SILPO-001 | Silpo лише на своїй реальній локації | ≥5 | IN_PROGRESS | 2026-08-22 still wrong/under-detailed. Canonical geo retained; detailed rebuild і runtime transform acceptance pending. |
| LOC-CULTURE-001 | Culture House лише на своїй реальній локації | ≥5 | IN_PROGRESS | 2026-08-22 still overlaps/wrong. Canonical geo retained; dedicated detail branch + runtime acceptance pending. |
| LOC-STADIUM-001 | Stadion Oster georeferenced, правильно орієнтований | ≥4 | IN_PROGRESS | Existing geo anchor remains authority; detailed stadium reconstruction needs dedicated branch and real-site acceptance. |
| LOC-TERRAIN-001 | Реальний relief, не плоска площина | ≥3 | IN_PROGRESS / DATA BLOCKED | Base still lacks verified terrain heightmap/Landscape elevation data. Не вигадувати relief формулою. |
| VIS-HOUSES-001 | Реальні Oster houses, не однакові huts | ≥4 | IN_PROGRESS | Requires broader real-house content pass and placement variation. |
| VIS-GRASS-001 | Натуральне покриття травою без progressive FPS collapse | 3 | CODED_UNTESTED | Pass 36 LowCPU scopes grass to ±75 m around Museum, 15 m grid, 8 cells/batch. Latest supplied Pass 36 screenshot shows FPS 27, so >=30 target is still not runtime-verified. |
| VIS-FLICKER-001 | Без distant flicker/z-fighting/late rebuild | ≥4 | IN_PROGRESS | Duplicate/late-owner cleanup remains suspect; Pass 37 now explicitly retires late duplicate R138 Museum architecture owners. |
| VIS-ROADS-001 | Roads/sidewalks не надмірно випуклі | 1 | IN_PROGRESS | Geometry pass pending. |
| VIS-LARGE-BUILDING-001 | Marked large building/stairs geometrically clean | 1 | IN_PROGRESS | Dedicated geometry/detail correction pending. |
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
| CRASH-FRONTEND-SLATE-20260823 | Frontend interaction → Slate/SlateCore array assertion | RUNTIME DID NOT RECUR IN LATEST GAMEPLAY RUN | Pass 29 static START route reached gameplay in latest run. Keep full acceptance gate. |
| VEHICLE-EXIT-RECOVERY-001 | Restore input stack after vehicle exit | CODED_UNTESTED | Existing source recovery. |
| TACTICAL-MAP-SOURCE-001 | `M` map / `V` trap canonical | CODED_UNTESTED | Pass 35 marker foreground fix; runtime acceptance pending. |
| LANDMARK-STARTUP-001 | Museum/Silpo/Culture без late startup rebuild | CODED_UNTESTED | Pass 37 adds actual visible-component proof + late duplicate R138 retirement for Museum only. |
| DEPLOY-LOADING-20260822 | Deployment START → blocking 0–100 overlay | CODED_UNTESTED | `OCDeploymentLoadingSubsystem`; routed from `UICommitDeployment()`. |
| START-FOLIAGE-BATCH-20260822 | Dense foliage не блокує один deployment frame | RUNTIME INSUFFICIENT | Superseded for LowCPU by bounded Pass 36 scope. |
| BASE-SPAWN-MUSEUM-20260822 | BASE spawn near canonical Museum | CODED_UNTESTED | Pass 37 primary ≈27.8 m, secondary ≈38.6 m, primary yaw faces Museum; 20–45 m guard band. |
| HUD-MINIMAP-20260822 | Runtime minimap | CODED_UNTESTED | `OCMinimapSubsystem` + Tactical Map render/projection getters. |
| VEH-REVERSE-20260822 | Low-speed/reverse steering authority | CODED_UNTESTED | `OCVehicleBase.cpp` steering floor/boost. |
| VEH-GUNNER-20260822 | Rear-side gunner entry + solo operation | CODED_UNTESTED | `OCArmedVehicleBase.cpp`. |
| MOUNTED-GUN-PROXY-20260822 | Primitive fake Browning hidden | CODED_UNTESTED | `OCPickupGunTruck.cpp`. |
| VEHICLE-ASSET-FRESHLOAD-20260822 | HMMWV/M2/BTR fresh-process gate | CODED_UNTESTED | Fresh UE open required before normal launch. |
| WEAPON-ASSET-GATE-20260822 | Required real weapon asset fresh UE gate | CODED_UNTESTED | Asset load is proven, authored material fidelity is not. |
| FOLIAGE-DENSITY-20260822 | Denser real grass HISM | SUPERSEDED FOR LOWCPU | Full-sector density replaced by bounded LowCPU recovery scope. |
| WEAPON-FX-20260822 | Thin tracer + directional muzzle presentation | CODED_UNTESTED | Existing source fix. |
| SOURCE-R10-FALSEPOSITIVE-20260822 | R10 verifier false positive removed | CODED_UNTESTED | Dedicated UI shadow check retained. |
| LAUNCHER-UX-001 | Один user launcher | VERIFIED ENTRY POINT | `START_HERE.cmd` remains only user-facing entry point. |
| MUSEUM-CORE-PRESENCE-20260823 | Empty field despite near-museum BASE | CODED_UNTESTED | Pass 35 actor-owner check is now known insufficient; Pass 37 proves visible structural components near anchor and rebuilds stale core. |
| TACTICAL-MAP-MARKER-20260823 | Player marker hidden under objective A | CODED_UNTESTED | Pass 35 raises canonical player marker to Z60/size26. |
| WEAPON-MATERIAL-20260823 | Real weapon silhouettes render white/grey | CODED_UNTESTED | Pass 36 default-only audit failed user runtime; Pass 37 explicitly palettes known incomplete restored Stein payloads while preserving AK. |
| PERF-PROGRESSIVE-FOLIAGE-20260823 | FPS ~32 → 8 → 7–4 during old ongoing population | CODED_UNTESTED | Pass 36 bounded LowCPU source fix exists; latest screenshot is 27 FPS and still below target. |
| MUSEUM-VISIBLE-CORE-PASS37-20260823 | Owner tags without visible Museum are no longer accepted | CODED_UNTESTED | New `OCMuseumVisibilityPass37Subsystem`: >=12 visible registered `MuseumStructural` components within 26 m of anchor, stale rebuild, duplicate retirement through delayed startup. |
| WEAPON-PALETTE-PASS37-20260823 | Non-null blank Stein materials no longer accepted as good presentation | CODED_UNTESTED | New `OCWeaponPalettePass37Subsystem`: weapon-specific metal/wood/polymer palette on known incomplete restored payloads; AK authored visual preserved. |
| BASE-VISIBLE-APPROACH-PASS37-20260823 | Spawn must visually read as “біля музею” | CODED_UNTESTED | Primary BASE moved to ≈27.8 m and faces Museum; deployment guard accepts 20–45 m exterior band. |

## 5. Останній фактичний user run — 2026-08-23

Підтверджено runtime після merge Pass 36 (`ea81b01d...`):
- gameplay відкривається і фізичний 11-weapon BASE rack присутній;
- початковий кадр усе ще показує плоске/порожнє поле, Museum не видно, тому користувач повторно відхилив spawn як «досі дуже далеко»;
- більшість rack weapon meshes знову білі/сірі; AK-47 має нормальну authored/textured presentation;
- supplied screenshot shows `FPS 27`, тобто поточний >=30 FPS acceptance не виконаний;
- user explicitly reports visual presentation regression («зіпсувалась графіка»).

Source diagnosis after this run:
- Pass 35 `PASS35_MUSEUM_CORE_READY` could be a false positive because it counted only R137/R138 owners and did not inspect actual visible `MuseumStructural` components;
- R13.8 still has a normal delayed 5.35 s startup, so early recovered architecture can later gain a duplicate owner unless ownership is stabilized through that window;
- Pass 36 `IsMissingOrDefaultMaterial()` preserved any non-null/non-Default material, but runtime proves some restored Stein assignments are still visually blank; repository folders show mesh/WPN payloads without standalone material/texture assets beside these restored meshes;
- 41 m was technically near by code, but the user has now rejected it twice as visually too far. Runtime requirement therefore supersedes the old 30–60 m acceptance band.

Pass 37 is **CODED_UNTESTED** until a new UE 5.8 run confirms the visible Museum, ~27.8 m BASE, non-grey restored weapon presentation, and >=30 FPS.

## 6. Наступна черга

1. Pass 37 source/CI: visible Museum structural guard, closer canonical BASE and Stein palette recovery. Merge only after all relevant checks are green.
2. `START_HERE.cmd → 2. ПОВНИЙ RUNTIME-ТЕСТ`: Museum must be physically visible in front of spawn; BASE deployment must emit `PASS37_BASE_DEPLOYMENT_VISIBLE_MUSEUM_APPROACH`.
3. Keep gameplay >=20 s so the guard runs past the historical R13.8 5.35 s delayed startup and can detect/retire duplicate architecture.
4. Rack must emit `PASS37_WEAPON_VISIBLE_PALETTE_READY`; visually inspect that restored Stein weapons are no longer white/blank while AK stays unchanged.
5. FPS acceptance stays >=30. Do not lower the threshold to make a bad run green.
6. Exact authored textures for incomplete restored payloads remain a separate content gap; runtime palette is not exact texture restoration.
7. Global landmark separation acceptance at canonical Museum/Silpo/Culture anchors.
8. Separate detail branches: Museum, Stadium, Silpo, Culture House; merge without moving canonical geo anchors.
9. Distant flicker/duplicate geometry, roads/sidewalk geometry, real Oster house variation and large-building/stairs detail remain after the current regression is closed.

**Заборона:** ніяких нових декоративних R15/R16 layers до закриття цього backlog.

## 2026-08-23 — Pass 29 frontend crash localization

- Pass 28 runtime again reproduced the exact Slate/SlateCore `Array index out of bounds: -808103970 into an array of size 0` immediately after pressing main-menu START.
- Pass 29 removed runtime page transitions from the startup shell; latest gameplay runs reached gameplay through this route.
- Keep status conservative because full frontend regression acceptance is broader than one successful START.

## 2026-08-23 — Pass 30 museum spawn / overlap correction

- Pass 30 moved primary BASE to ~41 m exterior positions, removed distorted window-frame geometry and speculative interior slabs, and widened old shell cleanup.
- Subsequent runtime rejected 41 m as visually too far and still showed an empty museum site; Pass 30 is not runtime-verified as a complete solution.

## 2026-08-23 — Pass 35 museum presence / tactical-map marker

- Pass 35 recovered a missing R13.7 carrier and invoked R13.8 when no architecture owner existed; Tactical Map marker was raised above objective labels.
- PR #69 merged as `ce39a101735a1b75c87c1f2f2ba4bb665b972299` with green source CI.
- Latest runtime proves owner-count-only Museum acceptance was insufficient. Status remains CODED_UNTESTED.

## 2026-08-23 — Pass 36 weapon materials / progressive FPS recovery

- Pass 36 added default-material audit and bounded LowCPU foliage; PR #70 merged as `ea81b01d28bd0a49a0333afa5ebe753ff0e73c10` with green source CI.
- Latest runtime still shows grey/white rack weapons and 27 FPS. Green source CI did not constitute visual runtime acceptance.
- Status remains CODED_UNTESTED.

## 2026-08-23 — Pass 37 visible Museum / closer BASE / weapon presentation

- User repeated the same three visual regressions after Pass 36: no visible Museum in the spawn view, BASE reads as too far, and most rack weapons remain grey/white.
- Primary canonical BASE moves to approximately 27.8 m from `MuseumAnchor`; camera/base yaw faces the Museum. Guard band becomes 20–45 m.
- New Museum guard validates actual registered visible `MuseumStructural` components near the anchor. It can rebuild an empty/stale R13.8 owner and retires duplicate late architecture owners through the historical 5.35 s delayed startup.
- New weapon presentation pass forces a deterministic visible palette only for known incomplete restored Stein payloads and preserves the correctly textured AK.
- Full runtime acceptance now requires Pass 37 visible-core/base/palette markers and keeps the existing >=30 FPS floor.
- Status: CODED_UNTESTED until UE 5.8 runtime.