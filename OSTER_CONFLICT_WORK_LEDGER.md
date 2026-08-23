# OSTER CONFLICT — WORK LEDGER

> Постійний журнал фактичного стану `main`. Runtime-скрін/лог/playtest завжди має пріоритет над code-only твердженням.

## 1. Поточний контекст

- Repository: `valentronus95/OsterConflict`
- Active correction branch: `fix/weapon-material-lowcpu-perf-pass-36-20260823` → `main`
- UE target: 5.8.x Windows
- Project: `OsterConflict/OsterConflict.uproject`
- User-facing launcher: **тільки `START_HERE.cmd`**.
- `RUN_*.cmd` — внутрішні helper scripts. Не створювати новий user-facing launcher під кожну R-версію.
- Persistent evidence: `RUNTIME_AUDIT_2026-08-21.md`, `LEGACY_BLOCKOUT_AUDIT_2026-08-21.md`, `RUNTIME_PLAYTEST_AUDIT_2026-08-21_1744.md`, `RUNTIME_PLAYTEST_AUDIT_2026-08-22.md`, `RUNTIME_PLAYTEST_AUDIT_2026-08-23_PASS35.md`.
- Latest user playtest 2026-08-23 reached gameplay after the frontend crash work, but showed a new authoritative failure set: BASE weapon rack in an apparently empty field instead of a visible nearby museum, Tactical Map local-player marker not visibly distinguishable, most rack weapons white/grey while AK retained authored color, and FPS falling from about 32 to 8 with user-observed lows of 7–4.
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
| UI-TRAVEL-001 | Deployment START без freeze/layout jump, 0–100 loading → gameplay | ≥5 | CODED_UNTESTED | Останній run дійшов до gameplay. Blocking loading + batched startup збережені; Pass 36 прибирає довгу progressive full-sector foliage population у normal LowCPU flow. |
| UI-CHAT-001 | Team chat `Y`, global chat `U`, панель прихована без вводу | 1 | CODED_UNTESTED | Runtime chat layer coded; acceptance pending. |
| GAME-SPAWN-001 | Фактичний spawn біля Museum, не порожнє поле | ≥6 | CODED_UNTESTED | 2026-08-23 user знову бачить «далеко від музею»: source BASE фактично ≈41 m від `MuseumAnchor`, але R13.7 міг повністю abort-ити через optional roof asset, після чого R13.8 не створював музей. Pass 35 додає late owner/core recovery без руху canonical anchor; runtime acceptance pending. |
| GAME-WEAPONS-001 | 11 pickup classes біля фактичного spawn | ≥6 | CODED_UNTESTED | 11-class rack фізично видно в latest run, але більшість real meshes білі/сірі. Pass 36 аудіює material slots, не чіпає valid authored materials і ремонтує лише null/Engine DefaultMaterial slots; runtime acceptance pending. |
| HUD-MINIMAP-001 | Постійна minimap на HUD + `M` full tactical map | 1 | CODED_UNTESTED | Доданий `OCMinimapSubsystem`; використовує той самий `OCTacticalMapSubsystem` render target/projection, player heading marker, приховується при blocking UI/full map. |
| UI-TACTICAL-MAP-001 | `M` tactical map без конфлікту та з видимим player marker | 2 | CODED_UNTESTED | Latest runtime map відкривається, але local marker візуально губиться біля Museum/A: canonical player marker був Z=20, objective marker Z=22. Pass 35 піднімає існуючий player marker до Z=60 і збільшує його, не дублюючи map projection owner. |
| GAME-VEHICLE-INPUT-001 | Після exit з авто повертаються WASD/sprint/mouse | 1 | IN_PROGRESS | Recovery coded; новий acceptance pending. |
| VEH-REVERSE-STEER-001 | Нормальний руль на малому ходу і заднім ходом | 1 | CODED_UNTESTED | 2026-08-22 runtime: reverse майже прямо. Root cause: steering authority → 0 при low speed. Доданий мінімальний steering authority, stronger reverse floor + reverse torque boost. |
| VEH-PICKUP-001 | Pickup/HMMWV має M2 Browning без proxy geometry | ≥4 | CODED_UNTESTED / ASSET CHECK | User повторно показав/описав відсутність production models. Old TurretBase/Barrel proxy hidden; normal launcher відновлює local HMMWV/M2 source, імпортує їх і робить fresh-process load verification. Якщо HMMWV/M2 не відкриваються, normal playtest тепер не повинен стартувати. |
| VEH-PICKUP-GUNNER-001 | Зрозумілий rear-side gunner entry, solo gunner працює | 1 | CODED_UNTESTED | Rear/turret-side `E` тепер вибирає gunner, front-side — driver; gunner operation більше не вимагає driver seat. |
| VEH-PICKUP-SPEED-001 | Pickup max speed 120 км/год | 1 | CODED_UNTESTED | Server/standalone speed contract coded; speed test pending. |
| ASSET-BTR-001 | BTR production model без proxy | ≥4 | CODED_UNTESTED / ASSET IMPORT CHECK | User повторно показав proxy BTR. Normal launcher тепер не допускає звичайний playtest, доки local BTR source не імпортований і canonical `/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus` не відкрився у другому fresh UE process. Runtime scale/material/ground contact acceptance ще потрібний. |
| VEH-BTR-SPEED-001 | BTR max speed 90 км/год | 1 | CODED_UNTESTED | Runtime speed contract coded; speed test pending. |
| VIS-FP-001 | Production/real weapon visuals без primitive/white material presentation | ≥6 | CODED_UNTESTED / ASSET PREFLIGHT | Latest runtime confirms real silhouettes are present but most rack meshes render white/grey. Existing validator only proved mesh load/identity, not material usability. Pass 36 adds runtime material-slot audit/recovery and preserves every non-default authored material. Exact authored texture fidelity for incomplete imported payloads remains a content acceptance item. |
| WEAPON-MUZZLE-001 | Visible shot FX стартує з дула | 2 | CODED_UNTESTED | Знайдений конкретний дефект: tracer передавався в FX як target-side partial streak, тому старий camera-distance guard відмовлявся rebase-ити його на muzzle. Pass 4 приймає local aim-ray, пріоритетно бере Muzzle/Barrel socket або barrel-named visible component і перебазовує local streak на кінець ствола. Runtime acceptance pending. |
| WEAPON-TRACER-001 | Немає жовтої круглої «кулі» | 1 | CODED_UNTESTED | Muzzle sphere замінено directional cone/cylinder; tracer radius обмежений до thin streak. Після muzzle rebase streak також обмежений 9 m, щоб не стати суцільним laser beam. |
| ASSET-CHARACTER-001 | Production character/skins | ≥2 | IN_PROGRESS | Real character model є, final combat profile/skins pending. |
| DEBUG-FLIGHT-001 | Керований spectator/free-fly test mode | 1 | IN_PROGRESS | Not final. |
| LOC-MUSEUM-001 | Museum окремо від Silpo/Culture і реально присутній у runtime | ≥6 | IN_PROGRESS | 2026-08-23 user бачить empty field біля правильного BASE. Concrete chain: R13.7 `BuildMuseum()` abort на optional roof → немає owner actor → R13.8 refuses architecture build. Pass 35 recovers only missing carrier, invokes authoritative R13.8 and replays R13.9–R14.5. Broader photo fidelity still IN_PROGRESS. |
| LOC-SILPO-001 | Silpo лише на своїй реальній локації | ≥5 | IN_PROGRESS | 2026-08-22 still wrong/under-detailed. Canonical geo retained; detailed rebuild і runtime transform acceptance pending. |
| LOC-CULTURE-001 | Culture House лише на своїй реальній локації | ≥5 | IN_PROGRESS | 2026-08-22 still overlaps/wrong. Canonical geo retained; dedicated detail branch + runtime acceptance pending. |
| LOC-STADIUM-001 | Stadion Oster georeferenced, правильно орієнтований | ≥4 | IN_PROGRESS | 2026-08-22 user says stadium still wrong. Existing geo anchor remains authority; detailed stadium reconstruction needs dedicated branch and real-site acceptance. |
| LOC-TERRAIN-001 | Реальний relief, не плоска площина | ≥3 | IN_PROGRESS / DATA BLOCKED | Base still lacks verified terrain heightmap/Landscape elevation data. Не вигадувати relief формулою. |
| VIS-HOUSES-001 | Реальні Oster houses, не однакові huts | ≥4 | IN_PROGRESS | 2026-08-22 distant/local houses still read as repeated generic huts. Requires broader real-house content pass and placement variation. |
| VIS-GRASS-001 | Натуральне покриття травою без progressive FPS collapse | 3 | CODED_UNTESTED | Latest run починається ~32 FPS і далі падає до 8 / user reports 7–4 while old dense foliage continues full-sector HISM population. Pass 36 LowCPU scopes real imported grass to ±75 m around Museum, 15 m grid, 8 cells/batch and stops instead of filling all 1.92 km. |
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
| CRASH-FRONTEND-SLATE-20260823 | Frontend interaction → Slate/SlateCore array assertion | RUNTIME DID NOT RECUR IN LATEST GAMEPLAY RUN | Pass 29 static START route reached gameplay in latest run. Keep full acceptance gate; do not regress to live page mutation. |
| VEHICLE-EXIT-RECOVERY-001 | Restore input stack after vehicle exit | CODED_UNTESTED | Existing source recovery. |
| TACTICAL-MAP-SOURCE-001 | `M` map / `V` trap canonical | CODED_UNTESTED | Latest run proves `M` map opens; Pass 35 fixes marker foreground priority. |
| LANDMARK-STARTUP-001 | Museum/Silpo/Culture без late startup rebuild | CODED_UNTESTED | Coordinator/source cleanup exists; Pass 35 adds missing museum-core recovery only when owner/architecture is absent. |
| DEPLOY-LOADING-20260822 | Deployment START → blocking 0–100 overlay | CODED_UNTESTED | `OCDeploymentLoadingSubsystem`; routed from `UICommitDeployment()`. |
| START-FOLIAGE-BATCH-20260822 | Dense foliage не блокує один deployment frame | RUNTIME INSUFFICIENT | Batch split removed one-frame stall but latest run proves progressive full-sector population still destroys FPS over time. Replaced by Pass 36 bounded LowCPU scope. |
| BASE-SPAWN-MUSEUM-20260822 | BASE spawn near canonical Museum | CODED_UNTESTED | Pass 30 current primary ≈41 m from Museum anchor + ground snap; Pass 35 validates 30–60 m band. |
| HUD-MINIMAP-20260822 | Runtime minimap | CODED_UNTESTED | `OCMinimapSubsystem` + Tactical Map render/projection getters. |
| VEH-REVERSE-20260822 | Low-speed/reverse steering authority | CODED_UNTESTED | `OCVehicleBase.cpp` steering floor/boost. |
| VEH-GUNNER-20260822 | Rear-side gunner entry + solo operation | CODED_UNTESTED | `OCArmedVehicleBase.cpp`. |
| MOUNTED-GUN-PROXY-20260822 | Primitive fake Browning hidden | CODED_UNTESTED | `OCPickupGunTruck.cpp`. |
| VEHICLE-ASSET-FRESHLOAD-20260822 | HMMWV/M2/BTR fresh-process gate | CODED_UNTESTED | Production importer must import all 3 canonical assets, then second Unreal process must reopen them before normal game launch. |
| WEAPON-ASSET-GATE-20260822 | Required real weapon asset fresh UE gate | CODED_UNTESTED | `verify_required_weapon_assets.py`; proves asset load, not authored material fidelity. Pass 36 adds runtime material audit. |
| FOLIAGE-DENSITY-20260822 | Denser real grass HISM | SUPERSEDED FOR LOWCPU | Full-sector density caused progressive runtime collapse; normal LowCPU now uses bounded museum-area recovery scope. |
| WEAPON-FX-20260822 | Thin tracer + directional muzzle presentation | CODED_UNTESTED | `OCTransientVisualFX.cpp`; pass 4 fixes target-side tracer rebase, local aim-ray ownership check and barrel/socket preference. |
| SOURCE-R10-FALSEPOSITIVE-20260822 | R10 verifier no longer treats unrelated namespace `Slot` locals as old UI shadow blocker | CODED_UNTESTED | Dedicated `OCGameUIRootWidget.cpp` shadow check retained; project-wide false-positive token removed. |
| LAUNCHER-UX-001 | Один user launcher | VERIFIED ENTRY POINT | `START_HERE.cmd` remains only user-facing entry point. |
| MUSEUM-CORE-PRESENCE-20260823 | Empty field despite near-museum BASE | CODED_UNTESTED | Pass 35: missing R13.7 owner carrier recovered only when absent; existing R13.8 authoritative architecture + R13.9–R14.5 detail stages replayed; canonical geo unchanged. PR #69 merged as `ce39a101...`. |
| TACTICAL-MAP-MARKER-20260823 | Player marker hidden under objective A | CODED_UNTESTED | Pass 35 raises existing `TacticalMapPlayerMarker` from presentation Z20 conflict to Z60/size26; map projection remains canonical owner. |
| WEAPON-MATERIAL-20260823 | Real weapon silhouettes render white/grey | CODED_UNTESTED | Pass 36 audits production/fallback mesh slots. Only null/Engine DefaultMaterial receives per-weapon MID recovery; valid authored material is never overwritten. |
| PERF-PROGRESSIVE-FOLIAGE-20260823 | FPS ~32 → 8 → 7–4 during ongoing population | CODED_UNTESTED | Pass 36 normal `PerfProfile=LowCPU`: foliage bounds ±75 m around Museum, 15 m grid, 8 cells/batch, finite completion; full profile remains available outside LowCPU. |

## 5. Останній фактичний user run — 2026-08-23

Підтверджено runtime:
- frontend START/travel route дійшов до gameplay; старий immediate Slate assertion у цьому run не показаний;
- player з'явився біля фізичного 11-weapon BASE rack, але музей в кадрі повністю відсутній, тому spawn візуально виглядає як «десь далеко в полі»;
- source після аналізу показує current BASE ≈41 m від `MuseumAnchor`, тож головний confirmed defect цього run — missing museum presentation/core, а не кілометровий spawn offset;
- Tactical Map відкривається через `M`, але local-player marker не видно/не відрізняється біля Museum/objective cluster;
- більшість rack weapon meshes мають реальну геометрію, але рендеряться білими/сірими; AK-47 зберігає authored color/material presentation;
- FPS на одному кадрі ~32, далі ~8; user повідомляє падіння до 7–4 FPS;
- old `OCDenseGroundFoliageSubsystem` на LowCPU все ще поступово проходив повний сектор -96000..96000 з 40 m grid і 4 cells/0.05 s, тобто додавав HISM приблизно 30 s після старту gameplay. Це добре узгоджується з progressive FPS collapse, але фактичний Pass 36 runtime ще не перевірений.

Нові Pass 35/36 зміни є лише `CODED_UNTESTED`, доки наступний UE 5.8 playtest не підтвердить музей у кадрі, видимий map marker, material presentation і стабільніший FPS.

## 6. Наступна черга

1. Завершити Pass 36 source/CI, merge лише після green checks.
2. `START_HERE.cmd → 2. ПОВНИЙ RUNTIME-ТЕСТ`: BASE 30–60 m від Museum, музей реально видимий, `M` показує зелений player marker поверх A, 11 weapons не white/DefaultMaterial, WASD/mouse працюють.
3. Тримати gameplay ≥16 s; acceptance має fail-closed, якщо `PASS14_PERF_BELOW_TARGET` або немає 30 FPS readiness marker.
4. Якщо FPS все ще падає після finite LowCPU foliage completion, наступним профілювати world/building/weapon draw cost, не зменшувати acceptance threshold заднім числом.
5. Exact M249/Remington production identity і повні authored textures для incomplete restored weapon payloads лишаються окремим content gap; runtime color recovery не називати exact texture restoration.
6. Global landmark separation acceptance at canonical Museum/Silpo/Culture anchors.
7. Separate detail branches: Museum, Stadium, Silpo, Culture House; merge without moving canonical geo anchors.
8. Distant flicker/duplicate geometry and roads/sidewalk geometry pass.
9. Real Oster house variation and large-building/stairs detail pass.

**Заборона:** ніяких нових декоративних R15/R16 layers до закриття цього backlog.

## 2026-08-23 — Pass 29 frontend crash localization

- Pass 28 runtime again reproduced the exact Slate/SlateCore `Array index out of bounds: -808103970 into an array of size 0` immediately after pressing main-menu START.
- The repeat isolated the failing pre-travel path to R13 `Page 0 -> Page 1`; Pass 29 removed runtime page transitions from the startup shell.
- START queues hosted travel directly from the static main menu; NETWORK queues direct connect from the saved/default address. `PendingPage` execution is fail-closed.
- Latest gameplay run reached gameplay through this route. Keep status conservative because full frontend regression acceptance is broader than one successful START.

## 2026-08-23 — Pass 30 museum spawn / overlap correction

- Pass 30 moved primary BASE spawns to ~41 m exterior front-side positions and secondary bases farther out, added a 30 m museum no-spawn exclusion radius, and recovery for any BASE deployment inside that radius.
- Distorted stretched rural-cabin window-frame meshes were removed in favor of lightweight clean frame geometry until a museum-specific authored frame exists.
- Unsupported R13.8 interior partition slabs were removed. Generic landmark shell cleanup around the museum was widened to 50 m.
- Runtime after these changes still showed an empty museum site and FPS collapse, so Pass 30 is not runtime-verified as a complete solution.

## 2026-08-23 — Pass 35 museum presence / tactical-map marker

- Concrete missing-museum chain: R13.7 `BuildMuseum()` required an optional rural-cabin roof and returned before creating its owner when that asset could not load; R13.8 then refused to build because it requires the R13.7 owner actor.
- Pass 35 adds a late, idempotent recovery that creates only the missing owner carrier, invokes the already-authoritative R13.8 enterable architecture, then replays R13.9–R14.5 details. It first attempts the authored roof and uses only a minimal last-resort silhouette if that optional asset still cannot load.
- Tactical Map continues to own player projection; Pass 35 only raises the existing player marker above objective labels.
- PR #69 merged to `main` as `ce39a101735a1b75c87c1f2f2ba4bb665b972299` after all relevant CI checks passed.
- Status: CODED_UNTESTED.

## 2026-08-23 — Pass 36 weapon materials / progressive FPS recovery

- Latest runtime proved mesh load is not enough: most rack weapons had real silhouettes but white/grey material presentation. Existing fresh-load validation did not inspect material usability.
- Pass 36 extends `OCRealWeaponFallbackSubsystem`: every production/fallback mesh is audited once; only null or Engine DefaultMaterial slots receive a lightweight per-weapon recovery MID. Existing authored materials are explicitly preserved. Static BASE rack visual shadows are disabled.
- Latest runtime also showed FPS decay over time, matching the old LowCPU path continuing to populate the entire 1.92 km dense-foliage sector for roughly 30 seconds.
- Pass 36 changes only `PerfProfile=LowCPU`: real imported grass is bounded to ±75 m around Museum, 15 m grid, 8 cells/batch, finite completion. Full profile constants remain intact for non-LowCPU sessions.
- Full runtime acceptance is extended through Pass 36: museum core/distance, visible tactical player marker, material audit, bounded foliage and >=30 FPS sample are all required.
- Status: CODED_UNTESTED until UE 5.8 runtime.
