# OSTER CONFLICT — WORK LEDGER

> Постійний журнал фактичного стану `main`. Runtime-скрін/лог/playtest завжди має пріоритет над code-only твердженням.

## 1. Поточний контекст

- Repository: `valentronus95/OsterConflict`
- Active correction branch: `fix/slate-render-target-startup-pass-43-20260824` → `main`
- UE target: 5.8.x Windows
- Project: `OsterConflict/OsterConflict.uproject`
- User-facing launcher: **тільки `START_HERE.cmd`**.
- `RUN_*.cmd` — внутрішні helper scripts. Не створювати новий user-facing launcher під кожну R-версію.
- Persistent evidence: `RUNTIME_AUDIT_2026-08-21.md`, `LEGACY_BLOCKOUT_AUDIT_2026-08-21.md`, `RUNTIME_PLAYTEST_AUDIT_2026-08-21_1744.md`, `RUNTIME_PLAYTEST_AUDIT_2026-08-22.md`, `RUNTIME_PLAYTEST_AUDIT_2026-08-23_PASS35.md`, `OsterConflict/Docs/WorkReports/RUNTIME_PLAYTEST_AUDIT_2026-08-23_PASS37.md`, `OsterConflict/Docs/WorkReports/RUNTIME_PLAYTEST_AUDIT_2026-08-24_PASS43.md`.
- Latest user playtest 2026-08-24 after Pass 42 is authoritative: normal game crashes during real frontend renderer startup with `Assertion failed: Texture` in `RenderTargetPool.cpp:95`; Crash Reporter stack is dominated by `RenderCore` and repeated `SlateRHIRenderer` frames. The attached launcher transcript proves editor build and isolated 11-weapon `-nullrhi` preflight completed before the failing `-game -Frontend` process.
- Pass 38 is merged as `f622b3dd04debe8aad78621d731ba15e7e3802f1`; Pass 39 as `827d586b882dc56242044cc4d4af66133a6b2db2`; Pass 40 as `3bfd2a7e9f1a22412edcd6a18d380a2efd1eaf44`; Pass 42 as `1654c746a22ef176c7c82ba38fdb3e3d42791342`. All runtime-sensitive claims remain `CODED_UNTESTED` unless explicitly proven by a later local UE run.
- Pass 43 priority is frontend renderer stability: automatic graphics migrations must not live-apply during Slate `NativeConstruct`, and minimap SceneCapture/render-target/Slate-brush creation must not occur before an actual unblocked gameplay Pawn exists.
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
| UI-BOOT-001 | Splash → main menu без чорної паузи | 2 | IN_PROGRESS | Pass 42 normal launch now crashes in real frontend renderer startup at `RenderTargetPool.cpp:95`; Pass 43 removes two early render-target hazards. Next UE 5.8 normal launch is authoritative. |
| UI-MENU-001 | Головне меню стабільне | ≥8 | IN_PROGRESS | Current Pass 42 launch fails before a usable frontend due to `Texture` assertion in `RenderTargetPool.cpp:95` with repeated `SlateRHIRenderer`. Pass 43 is source-fixed but runtime-unverified. |
| UI-TRAVEL-001 | Deployment START без freeze/layout jump, 0–100 loading → gameplay | ≥5 | CODED_UNTESTED | Blocking loading + batched startup збережені. Pass 40 не змінює flow, лише переводить viewport/deployment presentation зі щокадрових global scan/layout writes на cached 10 Hz observation + transition-only mutation. |
| UI-PERF-001 | UI helper subsystems не витрачають render-frame budget у gameplay | 1 | CODED_UNTESTED | Pass 40 caches viewport/deployment presentation roots, caps observation at 10 Hz and dedupes structural/visibility writes. Runtime acceptance pending. |
| UI-CHAT-001 | Team chat `Y`, global chat `U`, панель прихована без вводу | 1 | CODED_UNTESTED | Runtime chat layer coded; acceptance pending. |
| GAME-SPAWN-001 | Фактичний spawn біля Museum, не порожнє поле | ≥8 | CODED_UNTESTED | Primary BASE remains ≈27.8 m from `MuseumAnchor`. Pass 42 schedules R13.7 exterior at 0.75 s and R13.8 architecture at 1.10 s, then starts visibility proof at 1.45 s. Current Pass 42 runtime crashes before this can be accepted. |
| GAME-WEAPONS-001 | 11 pickup classes біля фактичного spawn | ≥8 | CODED_UNTESTED | 11-class rack physically exists; Pass 42 grounds every rack location through walkable-surface trace with 12 cm clearance. Current Pass 42 runtime crashes before visual acceptance. Exact missing texture payload remains a separate content gap. |
| HUD-MINIMAP-001 | Постійна minimap на HUD + `M` full tactical map | 2 | CODED_UNTESTED | Pass 43 defers the minimap 1600×900 SceneCapture/render target/Slate brush until an actual unblocked gameplay Pawn exists; frontend/deployment/settings/no-Pawn state cannot call `EnsureMapSnapshot()`. One-shot capture + 10 Hz marker/visibility budget retained. |
| UI-TACTICAL-MAP-001 | `M` tactical map без конфлікту та з видимим player marker | 2 | CODED_UNTESTED | Pass 35 піднімає існуючий player marker до Z=60/size26; Pass 43 does not alter explicit M-map capture behavior once gameplay is active. |
| GAME-VEHICLE-INPUT-001 | Після exit з авто повертаються WASD/sprint/mouse | 1 | IN_PROGRESS | Recovery logic збережена. Pass 41 замінює permanent 20 Hz repeating timer на one-shot adaptive polling: 20 Hz лише під час vehicle/UI transitions, 10 Hz у стабільному gameplay. Новий runtime acceptance pending. |
| VEH-REVERSE-STEER-001 | Нормальний руль на малому ходу і заднім ходом | 1 | CODED_UNTESTED | 2026-08-22 runtime: reverse майже прямо. Root cause: steering authority → 0 при low speed. Доданий мінімальний steering authority, stronger reverse floor + reverse torque boost. |
| VEH-PICKUP-001 | Pickup/HMMWV має M2 Browning без proxy geometry | ≥4 | CODED_UNTESTED / ASSET CHECK | Pass 42 normal launcher attempts exact canonical HMMWV + M2 production intake and runtime classes request `/Game/Production/Vehicles/HMMWV/SM_HMMWV_UA` + `/Game/Production/Weapons/M2/SM_M2_Browning`; authored production materials are restored after legacy VehicleBase recolouring. |
| VEH-PICKUP-GUNNER-001 | Зрозумілий rear-side gunner entry, solo gunner працює | 1 | CODED_UNTESTED | Rear/turret-side `E` вибирає gunner, front-side — driver; gunner operation не вимагає driver seat. |
| VEH-PICKUP-SPEED-001 | Pickup max speed 120 км/год | 1 | CODED_UNTESTED | Server/standalone speed contract coded; speed test pending. |
| ASSET-BTR-001 | BTR production model без proxy | ≥4 | CODED_UNTESTED / ASSET IMPORT CHECK | Pass 42 normal launcher attempts canonical `/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus`; authored production materials are restored after legacy VehicleBase recolouring. Runtime scale/material/ground-contact acceptance still required. |
| VEH-BTR-SPEED-001 | BTR max speed 90 км/год | 1 | CODED_UNTESTED | Runtime speed contract coded; speed test pending. |
| VIS-FP-001 | Production/real weapon visuals без primitive/white material presentation | ≥8 | CODED_UNTESTED / ASSET PREFLIGHT | Latest preflight in the Pass 42 launch successfully fresh-loaded all 11 required weapon visuals in isolated NullRHI. Runtime authored-material fidelity remains unaccepted because the real frontend crashed before gameplay. |
| VIS-GRAPHICS-QUALITY-001 | Графіка не розмита/спрощена автоматично під час gameplay | ≥3 | CODED_UNTESTED | Pass 42 keeps native 100% scale + Texture 3 for automatic profiles. Pass 43 keeps the same values but automatic first-run/Pass39/Pass42 migrations are persistence-only during frontend construction; live `ApplySettings(false)` remains explicit Settings-UI action only. |
| WEAPON-MUZZLE-001 | Visible shot FX стартує з дула | 2 | CODED_UNTESTED | Muzzle/socket rebase coded; runtime acceptance pending. |
| WEAPON-TRACER-001 | Немає жовтої круглої «кулі» | 1 | CODED_UNTESTED | Thin directional tracer coded; runtime acceptance pending. |
| ASSET-CHARACTER-001 | Production character/skins | ≥2 | IN_PROGRESS | Real character model є, final combat profile/skins pending. |
| DEBUG-FLIGHT-001 | Керований spectator/free-fly test mode | 1 | IN_PROGRESS | Not final. |
| LOC-MUSEUM-001 | Museum окремо від Silpo/Culture і реально присутній у runtime | ≥8 | IN_PROGRESS | Pass 42 keeps Pass 38 single-rebuild safety and earlier R13.7/R13.8 build. Current Pass 42 run crashed in frontend before Museum runtime acceptance. Broader photo fidelity remains IN_PROGRESS. |
| LOC-SILPO-001 | Silpo лише на своїй реальній локації | ≥5 | IN_PROGRESS | 2026-08-22 still wrong/under-detailed. Canonical geo retained; detailed rebuild і runtime transform acceptance pending. |
| LOC-CULTURE-001 | Culture House лише на своїй реальній локації | ≥5 | IN_PROGRESS | 2026-08-22 still overlaps/wrong. Canonical geo retained; dedicated detail branch + runtime acceptance pending. |
| LOC-STADIUM-001 | Stadion Oster georeferenced, правильно орієнтований | ≥4 | IN_PROGRESS | Existing geo anchor remains authority; detailed stadium reconstruction needs dedicated branch and real-site acceptance. |
| LOC-TERRAIN-001 | Реальний relief, не плоска площина | ≥3 | IN_PROGRESS / DATA BLOCKED | Base still lacks verified terrain heightmap/Landscape elevation data. Не вигадувати relief формулою. |
| VIS-HOUSES-001 | Реальні Oster houses, не однакові huts | ≥4 | IN_PROGRESS | Requires broader real-house content pass and placement variation. |
| VIS-GRASS-001 | Натуральне покриття травою без progressive FPS collapse | 4 | CODED_UNTESTED | Pass 42 keeps LowCPU foliage bounded but expands the useful Museum/BASE area to 200×200 m with 85 m grass cull. Foliage validation now samples at 4 Hz and stops rescanning retired proxies. Current run crashed before FPS acceptance. |
| VIS-FLICKER-001 | Без distant flicker/z-fighting/late rebuild | ≥4 | IN_PROGRESS | Duplicate/late-owner cleanup remains suspect; Pass 38 forbids repeated museum rebuild and only allows one duplicate cleanup after delayed startup settle. |
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
| CRASH-WEAPON-FALLBACK-001 | `Pure virtual` у `ApplyRealFallback()` | RUNTIME DID NOT RECUR IN LATEST GAMEPLAY RUN | GC-safe refs + guards; current Pass 42 run crashes earlier in frontend, so no new gameplay proof. |
| CRASH-FRONTEND-SLATE-20260823 | Frontend interaction → Slate/SlateCore array assertion | SUPERSEDED BY NEW FRONTEND CRASH | Pass 29 removed the old page-transition crash, but Pass 42 now exposes a different RenderTargetPool/SlateRHI startup assertion. |
| CRASH-SLATE-RENDERTARGET-PASS43-20260824 | Normal-game frontend → `RenderTargetPool.cpp:95` `Texture` assertion with repeated `SlateRHIRenderer` | CODED_UNTESTED | Pass 43 removes live automatic `GameSettings->ApplySettings` during `NativeConstruct` and defers minimap SceneCapture/render target/Slate brush until stable gameplay Pawn. Dedicated audit + verifier + CI added. |
| VEHICLE-EXIT-RECOVERY-001 | Restore input stack after vehicle exit | CODED_UNTESTED | Existing source recovery. |
| TACTICAL-MAP-SOURCE-001 | `M` map / `V` trap canonical | CODED_UNTESTED | Pass 35 marker foreground fix; runtime acceptance pending. |
| LANDMARK-STARTUP-001 | Museum/Silpo/Culture без late startup rebuild | CODED_UNTESTED | Pass 42 schedules normal Museum exterior/architecture before 1.10 s and starts visibility proof after that; Silpo/Culture ownership still pending. |
| DEPLOY-LOADING-20260822 | Deployment START → blocking 0–100 overlay | CODED_UNTESTED | `OCDeploymentLoadingSubsystem`; routed from `UICommitDeployment()`. |
| START-FOLIAGE-BATCH-20260822 | Dense foliage не блокує один deployment frame | RUNTIME INSUFFICIENT | Superseded for LowCPU by bounded Pass 36/42 scope. |
| BASE-SPAWN-MUSEUM-20260822 | BASE spawn near canonical Museum | CODED_UNTESTED | Pass 37 primary ≈27.8 m, secondary ≈38.6 m, primary yaw faces Museum; 20–45 m guard band. |
| HUD-MINIMAP-20260822 | Runtime minimap | CODED_UNTESTED | Pass 43 blocks hidden minimap render-target creation in frontend/deployment/settings/no-Pawn state; capture remains one-shot in gameplay, Slate updates 10 Hz. |
| VEH-REVERSE-20260822 | Low-speed/reverse steering authority | CODED_UNTESTED | `OCVehicleBase.cpp` steering floor/boost. |
| VEH-GUNNER-20260822 | Rear-side gunner entry + solo operation | CODED_UNTESTED | `OCArmedVehicleBase.cpp`. |
| MOUNTED-GUN-PROXY-20260822 | Primitive fake Browning hidden | CODED_UNTESTED | `OCPickupGunTruck.cpp`. |
| VEHICLE-ASSET-FRESHLOAD-20260822 | HMMWV/M2/BTR fresh-process gate | CODED_UNTESTED | Fresh UE open required before normal launch. |
| WEAPON-ASSET-GATE-20260822 | Required real weapon asset fresh UE gate | PREFLIGHT VERIFIED / RUNTIME UNTESTED | Current Pass 42 launcher transcript shows all 11 required weapon assets fresh-load successfully under isolated NullRHI before real frontend launch. Authored runtime material fidelity remains unproven. |
| FOLIAGE-DENSITY-20260822 | Denser real grass HISM | SUPERSEDED FOR LOWCPU | Full-sector density replaced by bounded LowCPU recovery scope. |
| WEAPON-FX-20260822 | Thin tracer + directional muzzle presentation | CODED_UNTESTED | Existing source fix. |
| SOURCE-R10-FALSEPOSITIVE-20260822 | R10 verifier false positive removed | CODED_UNTESTED | Dedicated UI shadow check retained. |
| LAUNCHER-UX-001 | Один user launcher | VERIFIED ENTRY POINT | `START_HERE.cmd` remains only user-facing entry point. |
| MUSEUM-CORE-PRESENCE-20260823 | Empty field despite near-museum BASE | CODED_UNTESTED | Pass 42 makes normal R13.7/R13.8 construction early; Pass 38 single destructive rebuild remains the recovery ceiling. |
| TACTICAL-MAP-MARKER-20260823 | Player marker hidden under objective A | CODED_UNTESTED | Pass 35 raises canonical player marker to Z60/size26. |
| WEAPON-MATERIAL-20260823 | Real weapon silhouettes render white/grey | CODED_UNTESTED | Pass 38 supersedes Pass 37 forced recolour: preserve any non-placeholder imported assignment; only explicit placeholder slots receive recovery. |
| PERF-PROGRESSIVE-FOLIAGE-20260823 | FPS ~32 → 8 → 7–4 during old ongoing population | CODED_UNTESTED | Pass 42 keeps LowCPU bounded at 200×200 m and throttles foliage acceptance scans to 4 Hz; current Pass 42 run crashes before new FPS evidence. |
| MUSEUM-VISIBLE-CORE-PASS37-20260823 | Owner tags without visible Museum are no longer accepted | CODED_UNTESTED | Visible `MuseumStructural` proof retained; Pass 42 starts proof after the earlier one-shot normal build and keeps Pass 38 one-rebuild ceiling. |
| WEAPON-PALETTE-PASS37-20260823 | Non-null blank Stein materials no longer accepted as good presentation | SUPERSEDED BY PASS38 | Forced full-slot palette produced flat/orange presentation on valid imported material assignments. |
| BASE-VISIBLE-APPROACH-PASS37-20260823 | Spawn must visually read as “біля музею” | CODED_UNTESTED | Primary BASE remains ≈27.8 m and faces Museum; missing visible building must fail instead of being hidden by distance-only acceptance. |
| PERF-RUNAWAY-RECOVERY-PASS38-20260824 | Rapid FPS/heat collapse from repeated recovery/scanning must stop | CODED_UNTESTED | Museum rebuild capped at one; fallback and palette world scans have finite 12-pass budgets and stop on convergence; acceptance fails on any budget exhaustion and retains >=30 FPS. |
| GRAPHICS-QUALITY-PASS39-20260824 | Remove automatic blurry/low graphics regression | CODED_UNTESTED | Superseded by Pass 42 clarity migration; Pass 43 changes only startup application lifecycle, not target quality values. |
| POSTSTART-TICK-BUDGET-PASS39-20260824 | Remove avoidable permanent/per-frame work | CODED_UNTESTED | Performance + foliage guards stop ticking when finished; minimap Slate update capped at 10 Hz; FP presentation resolves local pawn directly. Pass 43 additionally moves minimap render-target creation out of frontend. |
| POSTSTART-UI-FRAME-BUDGET-PASS40-20260824 | Remove render-frame UI root scans / repeated Slate writes | CODED_UNTESTED | Viewport stabilizer and deployment presentation cache `UOCGameUIRootWidget`, observe at 10 Hz, retry only until layout exists, and mutate layout/visibility on transitions instead of every frame. |
| INPUT-RECOVERY-POLL-BUDGET-PASS41-20260824 | Remove permanent fixed 20 Hz input recovery timer | CODED_UNTESTED | Repeating timer replaced by chained one-shot polls: 20 Hz around vehicle/UI transitions, 10 Hz after stable character recovery; `PASS31_GAMEPLAY_INPUT_READY` contract preserved. |
| PRODUCTION-VEHICLE-GROUND-RACK-PASS42-20260824 | Launch-ready production vehicles, grounded rack, clarity/foliage/Museum timing | RUNTIME REJECTED FOR LAUNCH READINESS | PR #76/source CI green, but first normal-game runtime after merge crashes in frontend at `RenderTargetPool.cpp:95` before gameplay. Pass 42 feature contracts remain CODED_UNTESTED behind Pass 43 startup recovery. |

## 5. Останній фактичний user run — 2026-08-24

Підтверджено runtime після merge Pass 42 (`1654c746a22ef176c7c82ba38fdb3e3d42791342`):
- `START_HERE.cmd → 1. ЗВИЧАЙНА ГРА` використано як єдиний user-facing launch route;
- safe renderer contract у launcher: DirectX 11 + Shader Model 5, flags `-d3d11 -sm5 -nohdr -norhithread`;
- production-source intake не знайшов локальний BTR-4 FBX, але це не місце crash;
- `OsterConflictEditor` build завершився `Result: Succeeded`;
- isolated weapon fresh-load process працював з `-run=pythonscript ... -nullrhi` і успішно відкрив усі 11 required weapon visuals;
- після завершення preflight launcher запустив реальний `UnrealEditor.exe ... -game -Frontend -d3d11 -sm5 -nohdr -norhithread ...`;
- real frontend process впав з `Assertion failed: Texture [RenderTargetPool.cpp] [Line: 95]`;
- Crash Reporter stack містить `RenderCore` та багато повторів `SlateRHIRenderer`;
- gameplay/spawn/Museum/FPS/vehicles цього run не приймаються як перевірені, бо crash стався раніше.

Source diagnosis after this run:
- `UOCGameUIRootWidget::NativeConstruct()` одразу викликає `SyncSettingsWidgetsFromBackend()`, яка доходить до `UOCPlayerUserSettings::Get()` → `EnsureInitialGraphicsProfile()`;
- Pass 42 automatic first-run/legacy migration path всередині цієї startup call chain робив live `UGameUserSettings::ApplySettings(false)`, тобто міг перебудувати viewport/backbuffer під час побудови Slate frontend;
- `UOCMinimapSubsystem::Tick()` до Pass 43 викликав `EnsureMinimap()` до перевірки `Pawn`/frontend/deployment/settings blocking state;
- `EnsureMinimap()` викликав tactical `EnsureMapSnapshot()`, який створював 1600×900 `UTextureRenderTarget2D`, `UpdateResourceImmediate(true)`, SceneCapture і потім публікував render target як Slate `UImage` brush;
- отже hidden minimap render-target path міг працювати ще у frontend без gameplay Pawn;
- обидва source paths напряму перетинаються з reported `RenderTargetPool` + `SlateRHIRenderer` crash class. Це локалізація причини, але не runtime verification;
- Pass 43 прибирає live automatic ApplySettings з `EnsureInitialGraphicsProfile()` і залишає лише persistence; explicit user Settings UI зберігає live apply;
- Pass 43 блокує minimap SceneCapture/render-target/Slate-brush creation до фактичного unblocked gameplay Pawn;
- Pass 23 DX11/SM5/no-HDR/no-RHI-thread isolation не послаблюється.

Pass 43 remains **CODED_UNTESTED** until a new UE 5.8 normal-game run confirms the `RenderTargetPool.cpp:95` assertion no longer occurs.

## 6. Наступна черга

1. Merge Pass 43 only after its dedicated CI, Pass 16/39/42 regressions and full `Source verification` are green.
2. Після merge оновити локальний `main` і спочатку запустити `START_HERE.cmd → 1. ЗВИЧАЙНА ГРА`; перша acceptance-умова — frontend відкривається без `RenderTargetPool.cpp:95` crash.
3. У логах очікувати `PASS43_STARTUP_GRAPHICS_PERSIST_ONLY_READY`; до gameplay цей шлях не повинен live-apply renderer settings.
4. `PASS43_MINIMAP_RENDER_TARGET_GAMEPLAY_ONLY_READY` має з'явитися тільки після реального gameplay Pawn, не в головному меню/deployment/settings.
5. Якщо normal frontend стабільний, тоді `START_HERE.cmd → 2. ПОВНИЙ RUNTIME-ТЕСТ` для Pass 38–42 feature/FPS acceptance.
6. Keep gameplay >=20 s only if thermals/FPS remain sane. If FPS rapidly falls or the machine heats sharply, exit immediately; a failed runtime is sufficient evidence.
7. Verify normal WASD/mouse after spawn and after vehicle exit; `PASS31_GAMEPLAY_INPUT_READY` must still report `moveIgnored=0 lookIgnored=0`.
8. Require `PASS38_WEAPON_FALLBACK_SCAN_STOPPED`, `PASS38_WEAPON_PALETTE_SCAN_STOPPED`, `PASS42_FOLIAGE_GUARD_THROTTLED_READY` and `PASS42_MUSEUM_EARLY_VISIBILITY_READY`; bounded-stop/budget-fail markers are acceptance failures.
9. Confirm `PASS42_PRODUCTION_VEHICLE_VISUALS_READY`, grounded 11-class rack, HMMWV + M2 + BTR production meshes, authored materials, scale and ground contact.
10. FPS acceptance stays >=30. Do not lower the threshold to make a bad run green.
11. Exact authored textures for incomplete restored weapon payloads remain a separate content gap; fallback colour is not exact skin restoration. M16/M4 are not present in the currently verified GitHub repository payload and must not be claimed as connected until a real source asset is found/imported.
12. Global landmark separation acceptance at canonical Museum/Silpo/Culture anchors.
13. Separate detail branches: Museum, Stadium, Silpo, Culture House; merge without moving canonical geo anchors.
14. Distant flicker/duplicate geometry, roads/sidewalk geometry, real Oster house variation and large-building/stairs detail remain after the current regression is closed.

**Заборона:** ніяких нових декоративних R15/R16 layers до закриття цього backlog.

## 2026-08-23 — Pass 29 frontend crash localization

- Pass 28 runtime again reproduced the exact Slate/SlateCore `Array index out of bounds: -808103970 into an array of size 0` immediately after pressing main-menu START.
- Pass 29 removed runtime page transitions from the startup shell; later gameplay runs reached gameplay through this route.
- Keep status conservative because full frontend regression acceptance is broader than one successful START.

## 2026-08-23 — Pass 30 museum spawn / overlap correction

- Pass 30 moved primary BASE to ~41 m exterior positions, removed distorted window-frame geometry and speculative interior slabs, and widened old shell cleanup.
- Subsequent runtime rejected 41 m as visually too far and still showed an empty museum site; Pass 30 is not runtime-verified as a complete solution.

## 2026-08-23 — Pass 35 museum presence / tactical-map marker

- Pass 35 recovered a missing R13.7 carrier and invoked R13.8 when no architecture owner existed; Tactical Map marker was raised above objective labels.
- PR #69 merged as `ce39a101735a1b75c87c1f2f2ba4bb665b972299` with green source CI.
- Latest gameplay-capable runtime proved owner-count-only Museum acceptance was insufficient. Status remains CODED_UNTESTED.

## 2026-08-23 — Pass 36 weapon materials / progressive FPS recovery

- Pass 36 added default-material audit and bounded LowCPU foliage; PR #70 merged as `ea81b01d28bd0a49a0333afa5ebe753ff0e73c10` with green source CI.
- Later gameplay runtime still showed grey/white rack weapons and sub-target FPS. Green source CI did not constitute visual runtime acceptance.
- Status remains CODED_UNTESTED.

## 2026-08-23 — Pass 37 visible Museum / closer BASE / weapon presentation

- User repeated visual regressions after Pass 36: no visible Museum in the spawn view, BASE reads as too far, and most rack weapons remain grey/white.
- Primary canonical BASE moved to approximately 27.8 m from `MuseumAnchor`; camera/base yaw faces the Museum. Guard band became 20–45 m.
- Museum guard validated registered visible `MuseumStructural` components near the anchor, but its recovery branch was not bounded and could repeatedly rebuild the full architecture while evidence remained false.
- Weapon presentation pass forced a deterministic palette on known restored Stein payloads. Latest gameplay runtime proved this overreach visually damaged valid assignments.
- PR #71 merged as `3041fd94d4c4ae560aa82bd92513b0312bc626cd`; later gameplay runtime rejected it as a complete fix.

## 2026-08-24 — Pass 38 runtime runaway / heat stabilization

- User runtime after Pass 37 shows empty Museum view, mixed/flat weapon presentation, FPS screenshots `26 → 10 → 8`, plus reported `60 → 5` in about five seconds and rapid laptop heating.
- Root lifecycle defect localized in Pass 37 museum guard: destructive full-architecture recovery could repeat every 0.35 s while visible-core evidence stayed below threshold.
- Pass 38 caps Museum destructive recovery at exactly one attempt; later polling is observational and final duplicate retirement occurs once after the historical delayed startup window.
- `OCRealWeaponFallbackSubsystem` no longer scans every weapon forever at 4 Hz: finite 12-pass / 0.5 s warm-up, stop on convergence, fail marker on budget exhaustion.
- `OCWeaponPalettePass37Subsystem` gets the same finite startup budget and no longer overwrites any non-placeholder material. Only explicit placeholder slots may receive fallback colour.
- Full runtime acceptance requires all Pass 38 stop/budget markers, fails on any bounded-stop/budget-fail marker, and keeps the existing >=30 FPS threshold.
- PR #72 merged as `f622b3dd04debe8aad78621d731ba15e7e3802f1` with green source/regression CI.
- Status: CODED_UNTESTED pending UE 5.8 runtime.

## 2026-08-24 — Pass 39 visual quality / post-start tick budget

- Source audit found that poor graphics were not only missing weapon materials: Pass 16 itself persisted an aggressive 75% mostly-Low/Off profile, and Pass 15 could later reduce the session to 65% screen percentage after a low FPS probe.
- Pass 39 changes the first-run ceiling to a conservative balanced profile: 85% scale, medium-ish view/texture/AA/landscape, Low expensive shadows/GI/reflections/foliage. Existing users migrate only if their current settings still match the recognizable legacy Pass 16 signature; any customized profile is preserved.
- Low-FPS probe is now diagnostic-only (`quality_mutation=0`); it cannot execute hidden scalability/LOD commands.
- Completed performance and foliage guards leave the tick manager through `IsTickable() == false` after finishing.
- Minimap kept the existing one-shot tactical-map render target and reduced marker/Slate visibility updates to 10 Hz. Pass 43 later adds frontend-safe render-target creation timing.
- First-person weapon presentation resolves the single local pawn directly and no longer iterates all `AOCCharacter` actors every frame.
- Full runtime acceptance rejects stale `PASS15_EMERGENCY_PERF_PROFILE_APPLIED`, requires Pass 39 graphics/minimap/FP/sampler markers, and keeps >=30 FPS.
- PR #73 merged as `827d586b882dc56242044cc4d4af66133a6b2db2` with green source/regression CI.
- Status: CODED_UNTESTED pending UE 5.8 runtime.

## 2026-08-24 — Pass 40 post-start UI frame budget

- Follow-up audit after Pass 39 found that two UI helper subsystems still ran at render-frame cadence even during steady gameplay.
- `OCR13UIViewportStabilizerSubsystem` previously performed a global `TObjectIterator<UOCGameUIRootWidget>` every frame, then repeatedly rewrote deployment clipping/column sizing and startup menu visibility/Z-order even when state had not changed.
- Pass 40 caches the current UI root/controller, only falls back to the global iterator on cache miss/root replacement, observes state at 10 Hz, retries deployment stabilization only until the panel exists or reopens, and applies startup isolation only on transitions.
- `OCR13DeploymentPresentationSubsystem` now uses the same cache-miss root strategy, 10 Hz observation, one style pass per root and deduplicated presentation visibility writes.
- Full runtime acceptance requires both Pass 40 budget markers and still keeps every Pass 38/39 gate plus >=30 FPS.
- PR #74 merged as `3bfd2a7e9f1a22412edcd6a18d380a2efd1eaf44` with green source/regression CI.
- Status: CODED_UNTESTED pending UE 5.8 runtime.

## 2026-08-24 — Pass 41 adaptive input recovery poll budget

- Follow-up audit found `UOCVehicleExitInputRecoverySubsystem` still ran a fixed repeating 20 Hz timer for the complete session, even when a character had already been recovered and no vehicle/UI transition was happening.
- Pass 41 replaces that repeating timer with chained one-shot polls.
- Vehicle possession and intentional UI locks retain 20 Hz checks for responsive restoration; stable recovered gameplay runs the guard at 10 Hz.
- The existing `PASS31_GAMEPLAY_INPUT_READY` marker and GameOnly/move/look recovery behavior are preserved.
- Full runtime acceptance requires `PASS41_INPUT_RECOVERY_POLL_BUDGET_READY` in addition to Pass 38/39/40 markers and >=30 FPS.
- Status: CODED_UNTESTED pending UE 5.8 runtime.

## 2026-08-24 — Pass 42 production vehicles / grounded BASE rack / launch clarity recovery

- Normal `START_HERE.cmd` invokes a production-vehicle intake helper that checks/imports canonical HMMWV, M2 Browning and BTR-4 assets when matching local source files exist.
- Runtime HMMWV/M2/BTR classes request the canonical production assets directly. A finite production-visual guard clears legacy material overrides so authored production material slots can render instead of BasicShape tinting.
- Museum BASE remains ≈27.8 m from `MuseumAnchor`; all 11 rack placements trace to walkable ground with 12 cm clearance instead of the old +72 cm offset.
- Unconfigured vehicle audio components disable their tick until an audio profile exists.
- Automatic graphics profiles target native 100% resolution scale and Texture Quality 3 while preserving conservative expensive lighting and genuinely customized user profiles.
- LowCPU foliage stays bounded but expands to 200×200 m around Museum/BASE with 85 m grass cull; the foliage acceptance guard samples at 4 Hz and stops rescanning retired source proxies.
- R13.7 Museum exterior schedules at 0.75 s, R13.8 architecture at 1.10 s, and the visibility guard starts at 1.45 s with a 2.20 s settle while keeping Pass 38's maximum one destructive rebuild.
- Pass 36/37/39 historical source verifiers were forward-ported so CI checks the current Pass 42 contract instead of obsolete 150×150 m / 5.8 s / 85% Texture 2 constants.
- M16/M4 still have no verified production payload in the current GitHub repository/tree/history; do not claim them as connected until a real source asset is located and imported.
- PR #76 merged as `1654c746a22ef176c7c82ba38fdb3e3d42791342` with green source/regression CI.
- Runtime status: launch-readiness rejected by the subsequent `RenderTargetPool.cpp:95` frontend crash; individual Pass 42 feature contracts remain CODED_UNTESTED.

## 2026-08-24 — Pass 43 Slate render-target startup recovery

- Authoritative Pass 42 normal-game run crashes in the real `-game -Frontend` process with `Assertion failed: Texture` at `RenderTargetPool.cpp:95`; stack repeatedly enters `SlateRHIRenderer`.
- The launcher transcript separately proves current editor build succeeded and all 11 required weapon visuals fresh-loaded under isolated NullRHI before the real frontend process started.
- Startup graphics call chain localized: `UOCGameUIRootWidget::NativeConstruct()` → `SyncSettingsWidgetsFromBackend()` → `UOCPlayerUserSettings::Get()` → `EnsureInitialGraphicsProfile()`.
- Pass 42 automatic migrations inside that call chain used live `UGameUserSettings::ApplySettings(false)`. Pass 43 keeps the target settings but makes automatic migration persistence-only; no live `GameSettings->ApplySettings` remains inside `EnsureInitialGraphicsProfile()`.
- Explicit user Settings UI still owns `GU->ApplySettings(false)` + `ConfirmVideoMode()` after the viewport is established.
- Second early RHI hazard localized: old minimap Tick called `EnsureMinimap()` before checking no-Pawn/frontend/deployment/settings state. That path allocated/captured a 1600×900 `UTextureRenderTarget2D` and published it into Slate as an image brush while the frontend could still be active.
- Pass 43 rejects all blocked/no-Pawn states before `EnsureMapSnapshot()` and before `EnsureMinimap()` in Tick. First minimap render target is allowed only with an actual unblocked gameplay Pawn.
- Existing one-shot tactical-map capture and Pass 39 10 Hz minimap UI budget remain intact.
- Pass 23 safe renderer contract remains DX11 + SM5 + `-nohdr -norhithread`.
- Runtime markers: `PASS43_STARTUP_GRAPHICS_PERSIST_ONLY_READY`, `PASS43_MINIMAP_RENDER_TARGET_GAMEPLAY_ONLY_READY`.
- Dedicated `VERIFY_SLATE_RENDER_TARGET_STARTUP_PASS_43.py`, workflow and full-suite integration added.
- Status: **CODED_UNTESTED** until the next local UE 5.8 normal-game launch proves `RenderTargetPool.cpp:95` no longer recurs.
