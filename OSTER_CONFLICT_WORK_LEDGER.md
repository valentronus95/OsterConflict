# OSTER CONFLICT — WORK LEDGER

> Current authoritative work state. Latest user runtime/build evidence overrides source-only claims and historical pass notes.
> Historical Pass 1–44 details remain preserved in Git history/reports; they are chronology, not current rules.

## 1. Current context

- Repository: `valentronus95/OsterConflict`
- Current `main` baseline before this corrective branch: `f789c42935fd7c90c7dfb4777e794e5cfecc1687` (PR #81 merged Pass 45 completion audit).
- Active local-build/import fix branch: `fix/pass45-local-build-import-regression-20260825` → `main`.
- Pass state token: **PASS 45 ACTIVE / LOCAL UE BUILD REJECTED / BUILD+IMPORT FIX CODED_UNTESTED**.
- Canonical TZ: `PASS45_RUNTIME_RECOVERY_TZ.md`.
- Completion audit: `OsterConflict/Docs/WorkReports/PASS45_COMPLETION_AUDIT_2026-08-25.md`.
- Latest factual evidence: user normal-route launch/build transcript from **2026-08-25**.
- Previous runtime rejection pack: `RUNTIME_EVIDENCE/2026-08-24_PASS44_REJECTED/`.
- UE target: 5.8.x Windows; factual failed build used UE **5.8.1**.
- Project: `OsterConflict/OsterConflict.uproject`.
- User launcher: **only `START_HERE.cmd`**.
- Hard map reference: `REFERENCE_PHOTOS/map_extent/oster_central_playable_area_20260824.jpg`.

## 2. Status rules

- `IN_PROGRESS` — source implementation or required content is incomplete.
- `CODED_UNTESTED` — source correction exists but local UE 5.8 has not accepted it.
- `CONTENT GAP` — required matching production content is absent/unverified; never fake READY.
- `LOCAL UE BUILD REJECTED` — a factual local UnrealBuildTool run rejected the current source before gameplay.
- `RUNTIME REJECTED` — factual local gameplay disproved the claimed result.
- `VERIFIED BUILD` — a later factual UE build removes a build blocker.
- `VERIFIED RUNTIME` — only factual local UE/user playtest can assign this.
- Green source CI is never UE compile/runtime acceptance.
- Latest screenshot/log/build transcript outranks historical source/verifier assumptions.
- Mesh load success is weaker than authored material/texture truth.
- Stale verifier expectations must be retired instead of restoring known regressions.

## 3. Latest authoritative local UE evidence — 2026-08-25

The user launched `START_HERE.cmd` and chose **1. ЗВИЧАЙНА ГРА** after pulling the Pass 45 completion merge.

### 3.1 Production source intake

The launcher/source recovery found the local HMMWV, M2 Browning and BTR-4 sources.

Factual import result:

- **BTR-4 imported successfully** as `/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus`.
- HMMWV import failed before asset creation because UE 5.8 rejected deprecated Interchange property `auto_detect_mesh_type` / `bAutoDetectMeshType`.
- M2 import failed for the same deprecated-property reason.
- launcher truthfully reported `HMMWV=0 M2=0 BTR4=1` and did not call the two failed assets production-ready.

Therefore:

- `ASSET-BTR-001`: production import is **LOCALLY CONFIRMED**, gameplay/presentation still unverified.
- HMMWV/M2 are **not missing-source gaps in this run**; they are a concrete UE 5.8 importer API regression.
- active source correction replaces the deprecated property with `convert_statics_with_animated_transform_to_skeletals=false` while retaining explicit `IFMT_STATIC_MESH`, static import enabled and skeletal import disabled.
- this HMMWV/M2 fix is **CODED_UNTESTED** until a later local import succeeds.

### 3.2 Factual C++ build blocker

UnrealBuildTool reached `OCTacticalMapVisual.cpp` and failed with:

`error C2131: expression did not evaluate to a constant`

Root cause: `Pass45ReferenceRoads` was declared `constexpr` even though its `FVector2D` members use a constructor that this UE 5.8/MSVC toolchain does not accept in that constant-expression context.

Consequences:

- UBT result: `Failed (OtherCompilationError)`.
- launcher build exit code: **6**.
- gameplay did **not** start, so this run cannot accept FPS, spawn, tactical-map visual appearance, landmarks, B2 visuals, trees or weapon materials.
- source CI from PR #81 is superseded as build evidence because it did not run this factual UE C++ compilation path.

Active correction:

- `Pass45ReferenceRoads` is now a normal namespace-scope `const` table.
- topology data, iteration and `UE_ARRAY_COUNT` behavior are unchanged.
- regression verifier forbids restoring the `constexpr` declaration.
- status remains **CODED_UNTESTED** until a later local UE build completes.

## 4. Previous authoritative runtime — 2026-08-24 after Pass 44

Until gameplay starts successfully again, the 2026-08-24 run remains the latest factual gameplay/runtime authority. It proved:

- menu about **8 FPS**;
- gameplay about **8–12 FPS** with heavy lag/stutter;
- slowdown also in visually empty areas;
- malformed procedural tactical topology;
- multiple white/missing-material rack weapons;
- primitive/fantasy tree silhouettes;
- blockout-grade flat environment visuals;
- Museum / Culture House visual identity not acceptably separated;
- stale/over-layered Silpo/Museum/Culture presentation.

**Pass 44 gameplay verdict remains RUNTIME REJECTED.**

## 5. Pass 45 B2 completion state

PR #81 merged the source-level B2 completion audit at `f789c42935fd7c90c7dfb4777e794e5cfecc1687`.

Current B2 source ownership:

- `UOCWorldProductionVisualsSubsystem` is the single generic environment visual owner;
- conversion occurs after source actor `BeginPlay`;
- source boxes remain hidden collision/backstop only where required;
- visible generic houses use `SM_House_Var01/02`;
- public/wood/metal/sheet fences use `SM_Fence_Var04/01/02/03`;
- ground uses `M_Inst_Landscape`;
- roads use `MI_Urb_Roa_Asphalt_01`;
- sidewalks use `MI_Urb_Roa_Sidewalk_01`;
- visual layer collision/navigation disabled;
- house cull 300–650 m; fence cull 60–280 m;
- no permanent successful-state polling.

Explicit B2 content gaps remain:

- College/civic photo-faithful production art;
- complete park detail set;
- suitable verified oak asset.

Because the 2026-08-25 C++ build failed before gameplay, all B2 visual placement/scale/material acceptance remains **CODED_UNTESTED**.

## 6. Active requirements

| ID | Requirement | Repeat | Status | Current action |
|---|---|---:|---|---|
| BUILD-P45-001 | Normal route must compile after Pass 45 tactical-map changes | 1 | CODED_UNTESTED after LOCAL UE BUILD REJECTED | Remove invalid `constexpr` FVector2D road table; next local UBT must complete. |
| PERF-COLLAPSE-001 | Stop frontend/gameplay collapse to ~8–12 FPS | ≥6 | CODED_UNTESTED | Cannot remeasure until build succeeds. RHI A/B, perf markers, mutation cleanup and compact culls remain coded. |
| UI-MENU-001 | Frontend/menu stable and usable | ≥9 | CODED_UNTESTED | Current 25.08 attempt did not reach new frontend because compilation failed. |
| VIS-GRAPHICS-QUALITY-001 | Restore readable non-blockout visual quality without hidden resolution downgrade | ≥4 | CODED_UNTESTED / CONTENT GAP | B2 source correction merged; factual visual test blocked by build failure. |
| VIS-TREES-001 | Tall pine/conifer character + appropriate oak; no fantasy primitive trees | ≥2 | CODED_UNTESTED / CONTENT GAP | Primitive trees retired; real pines verified; oak remains gap. |
| UI-TACTICAL-MAP-001 | `M` matches compact central-Oster topology | ≥4 | CODED_UNTESTED | Reference-traced topology retained; compile blocker in its table fixed, runtime screenshot still required. |
| MAP-EXTENT-001 | Keep compact central Oster battlefield | ≥2 | CODED_UNTESTED / RETAIN | 960×940 m hard extent retained. Never restore 2.4 km map. |
| LOC-MUSEUM-001 | Museum visible/unique near actual live spawn | ≥10 | CODED_UNTESTED | R13.8 single shell owner; runtime identity still required. |
| LOC-CULTURE-001 | Culture House separate from Museum | ≥2 | CODED_UNTESTED | R14.6 single shell owner at Culture geo anchor. |
| LOC-SILPO-001 | Silpo one site owner, no stale duplicate | ≥2 | CODED_UNTESTED | R14.0 single shell owner; runtime inspection required. |
| WEAPON-MATERIAL-001 | 11 rack weapons have authored material/texture truth | ≥10 | CODED_UNTESTED / CONTENT CHECK | Fresh NullRHI dependency audit coded; gameplay acceptance still pending. |
| GAME-WEAPONS-001 | 11 grounded pickups near Museum spawn | ≥9 | CODED_UNTESTED | Grounding retained; factual gameplay blocked by build failure. |
| GAME-SPAWN-001 | Live pawn spawns near Museum BASE | ≥9 | CODED_UNTESTED | Actual-pawn distance correction retained; runtime proof pending. |
| VIS-GRASS-001 | Natural grass without FPS collapse | ≥6 | CODED_UNTESTED | Density constrained; runtime baseline required. |
| VEH-PICKUP-001 | Real HMMWV + M2 | ≥5 | CODED_UNTESTED IMPORT API FIX | Local sources found; imports failed only on deprecated UE 5.8 property. Updated Interchange property needs local retest. |
| ASSET-BTR-001 | Real BTR-4/Bucephalus | ≥5 | LOCAL IMPORT CONFIRMED / RUNTIME UNTESTED | BTR canonical production asset imported in 25.08 run; presentation/vehicle integration still requires gameplay. |
| ASSET-M16-M4-001 | M16/M4 production visuals | ≥2 | IN_PROGRESS / CONTENT GAP | No verified payload; do not claim connected. |
| GAME-VEHICLE-INPUT-001 | WASD/mouse after vehicle exit | 1 | CODED_UNTESTED | Pass41 source recovery retained. |

## 7. Pass 44 behavior retained unless disproved

1. no implicit normal-local 16-bot autofill;
2. no 2.4 km playable/tactical map;
3. no old edge BASE/test-lane/vehicle spawn coordinates;
4. no ±920 m coordinate-based BASE role discriminator;
5. no grey BasicShape weapon-material repair;
6. no all-or-nothing vehicle import where one missing/failing item blocks independent assets;
7. no optimistic READY text for missing/failed production sources;
8. no stale verifier may force these regressions back.

## 8. Current execution state

1. Pass45 original source correction merged by PR #79. **DONE**.
2. Pass45 B2 completion correction merged by PR #81 as `f789c42935fd7c90c7dfb4777e794e5cfecc1687`. **DONE; SOURCE CI GREEN**.
3. User pulled/launched normal route on 2026-08-25. **DONE — LOCAL UE BUILD REJECTED**.
4. Diagnose C2131 tactical table compile blocker. **DONE**.
5. Diagnose HMMWV/M2 UE 5.8 deprecated Interchange property. **DONE**.
6. Tactical map `constexpr` → `const` build correction. **CODED_UNTESTED**.
7. HMMWV/M2 Interchange property correction. **CODED_UNTESTED**.
8. Add regression verifier + workflow; update aggregate verifier. **CODED**.
9. Current-head source CI for this corrective branch. **PENDING**.
10. Merge build/import corrective PR into `main` only after green current-head CI. **PENDING**.
11. Local test after pull: run `START_HERE.cmd` → `1. ЗВИЧАЙНА ГРА`; first acceptance is **successful UE build**, then frontend FPS, then gameplay. **PENDING**.

## 9. Next factual acceptance gates

### Build gate

- `OCTacticalMapVisual.cpp` compiles; no C2131.
- UBT exits 0.

### Production import gate

- BTR remains canonical and loadable.
- HMMWV import completes without deprecated-property exception.
- M2 import completes without deprecated-property exception.
- launcher reports each independently and truthfully.

### Runtime gates after successful build

- Frontend: no RenderTargetPool crash and **>=30 FPS minimum**.
- Gameplay: bots off, no progressive collapse, **>=30 FPS minimum**.
- Graphics/B2: real residential/fence visuals, real road/sidewalk/ground materials, no gross stretching/floating.
- Trees: no primitive fantasy forest; real pine/conifer character visible.
- Tactical map: recognizable compact Oster topology, no giant synthetic X/diagonals, player marker visible.
- Landmarks: Museum/Culture visibly separate; Silpo stale duplicate absent.
- Weapons: fresh 11-class dependency truth; no white/default slot accepted.

**Current overall status: PASS 45 ACTIVE / LOCAL UE BUILD REJECTED / BUILD+IMPORT FIX CODED_UNTESTED / SOURCE CI PENDING / LOCAL REBUILD REQUIRED.**
