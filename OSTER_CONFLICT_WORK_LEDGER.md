# OSTER CONFLICT — WORK LEDGER

> Постійний журнал фактичного стану `main`. Runtime-скрін/лог/playtest завжди має пріоритет над code-only твердженням.

## 1. Поточний контекст

- Repository: `valentronus95/OsterConflict`
- Active correction branch: `fix/input-recovery-poll-budget-pass-41-20260824` → `main`
- UE target: 5.8.x Windows
- Project: `OsterConflict/OsterConflict.uproject`
- User-facing launcher: **тільки `START_HERE.cmd`**.
- `RUN_*.cmd` — внутрішні helper scripts. Не створювати новий user-facing launcher під кожну R-версію.
- Persistent evidence: `RUNTIME_AUDIT_2026-08-21.md`, `LEGACY_BLOCKOUT_AUDIT_2026-08-21.md`, `RUNTIME_PLAYTEST_AUDIT_2026-08-21_1744.md`, `RUNTIME_PLAYTEST_AUDIT_2026-08-22.md`, `RUNTIME_PLAYTEST_AUDIT_2026-08-23_PASS35.md`, `OsterConflict/Docs/WorkReports/RUNTIME_PLAYTEST_AUDIT_2026-08-23_PASS37.md`.
- Latest user playtest 2026-08-24 is authoritative over green Pass 37 source CI: gameplay still opens into a flat/empty field with no visible Museum; rack weapons have mixed presentation (some textured, some grey/blank, some flat fallback colours); screenshots show FPS falling `26 → 10 → 8`, and the user reports roughly `60 → 5` within about five seconds together with rapid laptop heating.
- Pass 38 is merged as `f622b3dd04debe8aad78621d731ba15e7e3802f1`; Pass 39 as `827d586b882dc56242044cc4d4af66133a6b2db2`; Pass 40 as `3bfd2a7e9f1a22412edcd6a18d380a2efd1eaf44`. All remain `CODED_UNTESTED` until local UE runtime.
- Pass 41 priority is a remaining permanent gameplay poll: make vehicle/deployment input recovery adaptive instead of a fixed 20 Hz repeating timer, without weakening input restoration or the >=30 FPS acceptance floor.
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
| UI-TRAVEL-001 | Deployment START без freeze/layout jump, 0–100 loading → gameplay | ≥5 | CODED_UNTESTED | Blocking loading + batched startup збережені. Pass 40 не змінює flow, лише переводить viewport/deployment presentation зі щокадрових global scan/layout writes на cached 10 Hz observation + transition-only mutation. |
| UI-PERF-001 | UI helper subsystems не витрачають render-frame budget у gameplay | 1 | CODED_UNTESTED | Pass 40 caches viewport/deployment presentation roots, caps observation at 10 Hz and dedupes structural/visibility writes. Runtime acceptance pending. |
| UI-CHAT-001 | Team chat `Y`, global chat `U`, панель прихована без вводу | 1 | CODED_UNTESTED | Runtime chat layer coded; acceptance pending. |
| GAME-SPAWN-001 | Фактичний spawn біля Museum, не порожнє поле | ≥8 | CODED_UNTESTED | Pass 37 runtime знову відхилений: координатно BASE лишається ≈27.8 m від `MuseumAnchor`, але Museum в кадрі відсутній. Pass 38 не маскує це новим offset: він обмежує R13.8 recovery одним rebuild і fail-closed, щоб зупинити destructive churn та отримати чесний visible-core результат. |
| GAME-WEAPONS-001 | 11 pickup classes біля фактичного spawn | ≥8 | CODED_UNTESTED | 11-class rack фізично є. Pass 37 forced-palette покращив частину grey meshes, але зіпсував інші flat-colour presentation (зокрема Lever Action). Pass 38 більше не перезаписує non-placeholder imported materials; only explicit placeholder slots receive fallback. Exact missing texture payload лишається окремим content gap. |
| HUD-MINIMAP-001 | Постійна minimap на HUD + `M` full tactical map | 1 | CODED_UNTESTED | Pass 39 зберігає one-shot tactical map capture, але throttles minimap marker/Slate updates до 10 Hz і dedupes visibility writes. Runtime acceptance pending. |
| UI-TACTICAL-MAP-001 | `M` tactical map без конфлікту та з видимим player marker | 2 | CODED_UNTESTED | Pass 35 піднімає існуючий player marker до Z=60/size26; новий runtime acceptance лишається обов’язковим. |
| GAME-VEHICLE-INPUT-001 | Після exit з авто повертаються WASD/sprint/mouse | 1 | IN_PROGRESS | Recovery logic збережена. Pass 41 замінює permanent 20 Hz repeating timer на one-shot adaptive polling: 20 Hz лише під час vehicle/UI transitions, 10 Hz у стабільному gameplay. Новий runtime acceptance pending. |
| VEH-REVERSE-STEER-001 | Нормальний руль на малому ходу і заднім ходом | 1 | CODED_UNTESTED | 2026-08-22 runtime: reverse майже прямо. Root cause: steering authority → 0 при low speed. Доданий мінімальний steering authority, stronger reverse floor + reverse torque boost. |
| VEH-PICKUP-001 | Pickup/HMMWV має M2 Browning без proxy geometry | ≥4 | CODED_UNTESTED / ASSET CHECK | Old TurretBase/Barrel proxy hidden; normal launcher відновлює local HMMWV/M2 source, імпортує їх і робить fresh-process load verification. |
| VEH-PICKUP-GUNNER-001 | Зрозумілий rear-side gunner entry, solo gunner працює | 1 | CODED_UNTESTED | Rear/turret-side `E` вибирає gunner, front-side — driver; gunner operation не вимагає driver seat. |
| VEH-PICKUP-SPEED-001 | Pickup max speed 120 км/год | 1 | CODED_UNTESTED | Server/standalone speed contract coded; speed test pending. |
| ASSET-BTR-001 | BTR production model без proxy | ≥4 | CODED_UNTESTED / ASSET IMPORT CHECK | Normal launcher не допускає playtest, доки canonical BTR asset не відкрився у fresh UE process; runtime scale/material/ground-contact acceptance ще потрібний. |
| VEH-BTR-SPEED-001 | BTR max speed 90 км/год | 1 | CODED_UNTESTED | Runtime speed contract coded; speed test pending. |
| VIS-FP-001 | Production/real weapon visuals без primitive/white material presentation | ≥8 | CODED_UNTESTED / ASSET PREFLIGHT | Latest Pass 37 runtime shows mixed results: AK and some models carry usable presentation, several remain grey/blank, while forced palette made Lever Action visibly flat/orange. Pass 38 removes forced overwrite of valid materials, bounds palette/fallback scans, and keeps exact authored texture restoration separate from fallback colour. Pass 39 also removes the per-frame all-character scan from local first-person presentation. |
| VIS-GRAPHICS-QUALITY-001 | Графіка не розмита/спрощена автоматично під час gameplay | ≥3 | CODED_UNTESTED | Root source found: Pass 16 persisted 75% resolution + mostly Low/Off quality, and Pass 15 could drop runtime again to 65% with shadows/GI/reflections/foliage disabled after a low probe. Pass 39 replaces first-run ceiling with conservative balanced quality, migrates only recognizable legacy Pass 16 profile once, preserves custom user profile, and makes low-FPS probe diagnostic-only. |
| WEAPON-MUZZLE-001 | Visible shot FX стартує з дула | 2 | CODED_UNTESTED | Muzzle/socket rebase coded; runtime acceptance pending. |
| WEAPON-TRACER-001 | Немає жовтої круглої «кулі» | 1 | CODED_UNTESTED | Thin directional tracer coded; runtime acceptance pending. |
| ASSET-CHARACTER-001 | Production character/skins | ≥2 | IN_PROGRESS | Real character model є, final combat profile/skins pending. |
| DEBUG-FLIGHT-001 | Керований spectator/free-fly test mode | 1 | IN_PROGRESS | Not final. |
| LOC-MUSEUM-001 | Museum окремо від Silpo/Culture і реально присутній у runtime | ≥8 | IN_PROGRESS | Pass 37 visible-component guard still failed user runtime. Root lifecycle bug found in source: while evidence stayed below threshold it could retire and rebuild the complete R13.8 architecture every 0.35 s, up to 24 polls. Pass 38 caps destructive recovery at one rebuild, observes through the late-start window, then fails closed instead of rebuilding again. Broader photo fidelity remains IN_PROGRESS. |
| LOC-SILPO-001 | Silpo лише на своїй реальній локації | ≥5 | IN_PROGRESS | 2026-08-22 still wrong/under-detailed. Canonical geo retained; detailed rebuild і runtime transform acceptance pending. |
| LOC-CULTURE-001 | Culture House лише на своїй реальній локації | ≥5 | IN_PROGRESS | 2026-08-22 still overlaps/wrong. Canonical geo retained; dedicated detail branch + runtime acceptance pending. |
| LOC-STADIUM-001 | Stadion Oster georeferenced, правильно орієнтований | ≥4 | IN_PROGRESS | Existing geo anchor remains authority; detailed stadium reconstruction needs dedicated branch and real-site acceptance. |
| LOC-TERRAIN-001 | Реальний relief, не плоска площина | ≥3 | IN_PROGRESS / DATA BLOCKED | Base still lacks verified terrain heightmap/Landscape elevation data. Не вигадувати relief формулою. |
| VIS-HOUSES-001 | Реальні Oster houses, не однакові huts | ≥4 | IN_PROGRESS | Requires broader real-house content pass and placement variation. |
| VIS-GRASS-001 | Натуральне покриття травою без progressive FPS collapse | 4 | CODED_UNTESTED | Pass 36 LowCPU foliage already bounded population, yet Pass 37 runtime still collapses to single-digit FPS. Pass 38 treats museum rebuild churn and permanent weapon scans as higher-confidence new sources; Pass 39 also stops foliage validation tick after success/fail. Foliage threshold remains unchanged until runtime isolates it. |
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
| CRASH-WEAPON-FALLBACK-001 | `Pure virtual` у `ApplyRealFallback()` | RUNTIME DID NOT RECUR IN LATEST RUN | GC-safe refs + guards. |
| CRASH-FRONTEND-SLATE-20260823 | Frontend interaction → Slate/SlateCore array assertion | RUNTIME DID NOT RECUR IN LATEST GAMEPLAY RUN | Pass 29 static START route reached gameplay in latest run. Keep full acceptance gate. |
| VEHICLE-EXIT-RECOVERY-001 | Restore input stack after vehicle exit | CODED_UNTESTED | Existing source recovery. |
| TACTICAL-MAP-SOURCE-001 | `M` map / `V` trap canonical | CODED_UNTESTED | Pass 35 marker foreground fix; runtime acceptance pending. |
| LANDMARK-STARTUP-001 | Museum/Silpo/Culture без late startup rebuild | CODED_UNTESTED | Pass 38 bounds Museum destructive recovery to one rebuild + one late duplicate cleanup. Silpo/Culture ownership still pending. |
| DEPLOY-LOADING-20260822 | Deployment START → blocking 0–100 overlay | CODED_UNTESTED | `OCDeploymentLoadingSubsystem`; routed from `UICommitDeployment()`. |
| START-FOLIAGE-BATCH-20260822 | Dense foliage не блокує один deployment frame | RUNTIME INSUFFICIENT | Superseded for LowCPU by bounded Pass 36 scope. |
| BASE-SPAWN-MUSEUM-20260822 | BASE spawn near canonical Museum | CODED_UNTESTED | Pass 37 primary ≈27.8 m, secondary ≈38.6 m, primary yaw faces Museum; 20–45 m guard band. |
| HUD-MINIMAP-20260822 | Runtime minimap | CODED_UNTESTED | Pass 39 marker UI update budget 10 Hz; Tactical Map capture remains `bCaptureEveryFrame=false`. |
| VEH-REVERSE-20260822 | Low-speed/reverse steering authority | CODED_UNTESTED | `OCVehicleBase.cpp` steering floor/boost. |
| VEH-GUNNER-20260822 | Rear-side gunner entry + solo operation | CODED_UNTESTED | `OCArmedVehicleBase.cpp`. |
| MOUNTED-GUN-PROXY-20260822 | Primitive fake Browning hidden | CODED_UNTESTED | `OCPickupGunTruck.cpp`. |
| VEHICLE-ASSET-FRESHLOAD-20260822 | HMMWV/M2/BTR fresh-process gate | CODED_UNTESTED | Fresh UE open required before normal launch. |
| WEAPON-ASSET-GATE-20260822 | Required real weapon asset fresh UE gate | CODED_UNTESTED | Asset load is proven, authored material fidelity is not. |
| FOLIAGE-DENSITY-20260822 | Denser real grass HISM | SUPERSEDED FOR LOWCPU | Full-sector density replaced by bounded LowCPU recovery scope. |
| WEAPON-FX-20260822 | Thin tracer + directional muzzle presentation | CODED_UNTESTED | Existing source fix. |
| SOURCE-R10-FALSEPOSITIVE-20260822 | R10 verifier false positive removed | CODED_UNTESTED | Dedicated UI shadow check retained. |
| LAUNCHER-UX-001 | Один user launcher | VERIFIED ENTRY POINT | `START_HERE.cmd` remains only user-facing entry point. |
| MUSEUM-CORE-PRESENCE-20260823 | Empty field despite near-museum BASE | CODED_UNTESTED | Pass 37 visible-component proof remains, but Pass 38 stops repeated destructive rebuild when that proof fails. |
| TACTICAL-MAP-MARKER-20260823 | Player marker hidden under objective A | CODED_UNTESTED | Pass 35 raises canonical player marker to Z60/size26. |
| WEAPON-MATERIAL-20260823 | Real weapon silhouettes render white/grey | CODED_UNTESTED | Pass 38 supersedes Pass 37 forced recolour: preserve any non-placeholder imported assignment; only explicit placeholder slots receive recovery. |
| PERF-PROGRESSIVE-FOLIAGE-20260823 | FPS ~32 → 8 → 7–4 during old ongoing population | CODED_UNTESTED | Pass 36 bounded LowCPU source fix exists; Pass 37 runtime still collapses, so foliage is no longer assumed to be the only cause. |
| MUSEUM-VISIBLE-CORE-PASS37-20260823 | Owner tags without visible Museum are no longer accepted | CODED_UNTESTED | Visible `MuseumStructural` proof retained; Pass 38 caps R13.8 recovery to one attempt and records destructive-loop budget. |
| WEAPON-PALETTE-PASS37-20260823 | Non-null blank Stein materials no longer accepted as good presentation | SUPERSEDED BY PASS38 | Forced full-slot palette produced flat/orange presentation on valid imported material assignments. |
| BASE-VISIBLE-APPROACH-PASS37-20260823 | Spawn must visually read as “біля музею” | CODED_UNTESTED | Primary BASE remains ≈27.8 m and faces Museum; missing visible building must fail instead of being hidden by distance-only acceptance. |
| PERF-RUNAWAY-RECOVERY-PASS38-20260824 | Rapid FPS/heat collapse from repeated recovery/scanning must stop | CODED_UNTESTED | Museum rebuild capped at one; fallback and palette world scans have finite 12-pass budgets and stop on convergence; acceptance fails on any budget exhaustion and retains >=30 FPS. |
| GRAPHICS-QUALITY-PASS39-20260824 | Remove automatic blurry/low graphics regression | CODED_UNTESTED | First-run ceiling now balanced (85% scale, medium-ish view/texture/AA, low expensive lighting); exact old Pass 16 signature migrates once; custom profile preserved; low-FPS sampler cannot mutate graphics. |
| POSTSTART-TICK-BUDGET-PASS39-20260824 | Remove avoidable permanent/per-frame work | CODED_UNTESTED | Performance + foliage guards stop ticking when finished; minimap Slate update capped at 10 Hz; FP presentation resolves local pawn directly instead of `TActorIterator<AOCCharacter>` every frame. |
| POSTSTART-UI-FRAME-BUDGET-PASS40-20260824 | Remove render-frame UI root scans / repeated Slate writes | CODED_UNTESTED | Viewport stabilizer and deployment presentation cache `UOCGameUIRootWidget`, observe at 10 Hz, retry only until layout exists, and mutate layout/visibility on transitions instead of every frame. |
| INPUT-RECOVERY-POLL-BUDGET-PASS41-20260824 | Remove permanent fixed 20 Hz input recovery timer | CODED_UNTESTED | Repeating timer replaced by chained one-shot polls: 20 Hz around vehicle/UI transitions, 10 Hz after stable character recovery; `PASS31_GAMEPLAY_INPUT_READY` contract preserved. |

## 5. Останній фактичний user run — 2026-08-24

Підтверджено runtime після merge Pass 37 (`3041fd94...`):
- gameplay відкривається і фізичний weapon BASE rack присутній;
- Museum знову не видно прямо перед spawn, попри nominal ≈27.8 m canonical BASE distance;
- weapon presentation змішана: AK і частина моделей виглядають краще, деякі лишаються grey/blank, а Lever Action отримав неприродний flat orange/brown вигляд від forced palette;
- screenshots show `FPS 26`, потім `10`, потім `8`; user reports приблизно `60 → 5` за ~5 секунд;
- user reports різке нагрівання комп'ютера разом із падінням FPS;
- user repeatedly rejects current graphics quality as visibly bad/degraded;
- цей runtime повністю відхиляє Pass 37 як runtime solution, незалежно від green source CI.

Source diagnosis after this run:
- `UOCMuseumVisibilityPass37Subsystem::ValidateVisibleMuseum()` міг заходити в destructive recovery кожні `0.35 s`, якщо visible-core threshold не досягався: retire all R13.8 owners → rebuild complete architecture → repeat. Максимум був 24 polls. Це найсильніший source-side збіг із швидким `60 → 5` FPS/heat collapse; Pass 38 bounds this path;
- `UOCRealWeaponFallbackSubsystem` додатково мав permanent `0.25 s` world-wide weapon scan навіть після convergence; Pass 38 bounds it;
- `UOCWeaponPalettePass37Subsystem` polling завершувався тільки при повному rack audit, без hard upper bound; Pass 38 bounds it;
- Pass 37 forced recolour intentionally replaced every material slot on known restored Stein payloads, що пояснює flat orange Lever Action. Pass 38 removes this overwrite;
- separate graphics regression found in `UOCPlayerUserSettings`: Pass 16 persisted a one-time ceiling of resolution scale `75`, shadow/foliage/GI/reflection `0` and most remaining groups `<=1`;
- separate mid-session graphics regression found in `UOCPerformanceSampleSubsystem`: when the 2 s probe fell below 20 FPS, Pass 15 executed another emergency profile (`r.ScreenPercentage 65`, shadows/GI/reflections/foliage off, reduced LOD/view distance). This could make the picture visibly worse after the FPS collapse without fixing its cause;
- minimap scene capture itself is one-shot (`bCaptureEveryFrame=false`), but marker/visibility were still mutating Slate every frame; FP presentation also scanned every `AOCCharacter` every frame. Pass 39 budgets these paths without deleting features;
- Pass 40 follow-up found two additional render-frame UI paths: `UOCR13UIViewportStabilizerSubsystem` globally searched all `UOCGameUIRootWidget` objects and rewrote deployment clipping/column sizing/startup visibility/Z-order every frame; `UOCR13DeploymentPresentationSubsystem` performed another global root search and repeated visibility calls every frame. These are source-side inefficiencies, not proof they alone caused the thermal collapse;
- Pass 41 follow-up found `UOCVehicleExitInputRecoverySubsystem` still used a permanent repeating `0.05 s` timer even after the player was stably recovered. It is now one-shot/adaptive, preserving 20 Hz only where quick recovery matters and using 10 Hz in steady gameplay;
- Pass 36 bounded LowCPU foliage лишається під підозрою лише якщо FPS після усунення lifecycle/tick/UI churn все одно падає; threshold не послаблювати.

Pass 38, Pass 39, Pass 40 and Pass 41 remain **CODED_UNTESTED** until a new UE 5.8 run confirms: no rapid heat/FPS collapse, no repeated museum rebuild, weapon scans stop, graphics no longer auto-degrade, UI remains stable with bounded presentation work, input recovery still restores WASD/mouse correctly, Museum visible, and >=30 FPS sustained acceptance.

## 6. Наступна черга

1. Pass 41 source/CI: adaptive one-shot input recovery polling, preserving Pass 31 input restoration. Merge only after all relevant checks are green.
2. `START_HERE.cmd → 2. ПОВНИЙ RUNTIME-ТЕСТ`: require Pass 38 lifecycle markers + Pass 39 graphics/minimap/FP/sampler + Pass 40 UI-budget markers + `PASS41_INPUT_RECOVERY_POLL_BUDGET_READY`.
3. Keep gameplay >=20 s only if thermals/FPS remain sane. If FPS rapidly falls or the machine heats sharply, exit immediately; a failed runtime is sufficient evidence.
4. Verify normal WASD/mouse after spawn and after vehicle exit; `PASS31_GAMEPLAY_INPUT_READY` must still report `moveIgnored=0 lookIgnored=0`.
5. Old `PASS15_EMERGENCY_PERF_PROFILE_APPLIED` in a new run is a hard stale-binary failure. Pass 39 must emit `PASS39_GRAPHICS_QUALITY_PROFILE_READY` and never mutate quality after a low probe.
6. Require `PASS38_WEAPON_FALLBACK_SCAN_STOPPED` and `PASS38_WEAPON_PALETTE_SCAN_STOPPED`; any `*_BOUNDED_STOP` is an acceptance failure, not a reason to extend polling.
7. Rack must still emit `PASS37_WEAPON_VISIBLE_PALETTE_READY`, but valid imported materials must not be overwritten by a forced flat palette.
8. FPS acceptance stays >=30. Do not lower the threshold to make a bad run green.
9. Exact authored textures for incomplete restored payloads remain a separate content gap; fallback colour is not exact skin restoration.
10. Global landmark separation acceptance at canonical Museum/Silpo/Culture anchors.
11. Separate detail branches: Museum, Stadium, Silpo, Culture House; merge without moving canonical geo anchors.
12. Distant flicker/duplicate geometry, roads/sidewalk geometry, real Oster house variation and large-building/stairs detail remain after the current regression is closed.

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
- Later runtime still showed grey/white rack weapons and sub-target FPS. Green source CI did not constitute visual runtime acceptance.
- Status remains CODED_UNTESTED.

## 2026-08-23 — Pass 37 visible Museum / closer BASE / weapon presentation

- User repeated visual regressions after Pass 36: no visible Museum in the spawn view, BASE reads as too far, and most rack weapons remain grey/white.
- Primary canonical BASE moved to approximately 27.8 m from `MuseumAnchor`; camera/base yaw faces the Museum. Guard band became 20–45 m.
- Museum guard validated registered visible `MuseumStructural` components near the anchor, but its recovery branch was not bounded and could repeatedly rebuild the full architecture while evidence remained false.
- Weapon presentation pass forced a deterministic palette on known restored Stein payloads. Latest runtime proved this overreach visually damaged valid assignments.
- PR #71 merged as `3041fd94d4c4ae560aa82bd92513b0312bc626cd`; latest runtime rejects it as a complete fix.

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
- Minimap keeps the existing one-shot tactical-map render target but updates marker/Slate visibility at 10 Hz instead of every rendered frame.
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
