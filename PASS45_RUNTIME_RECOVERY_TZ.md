# OSTER CONFLICT — PASS 45 RUNTIME RECOVERY TZ

Date: 2026-08-24
Completion audit: 2026-08-25
Local build rejection: 2026-08-25
PR #82 merge: 2026-08-25
Status: **PR #82 MERGED / SOURCE CI 20/20 GREEN / LOCAL UE BUILD REJECTED / BUILD+IMPORT FIX CODED_UNTESTED**
Original source branch: `fix/runtime-recovery-pass-45-20260824` — merged by PR #79
Completion branch: `fix/pass45-completion-audit-20260825` — merged by PR #81
Build/import fix branch: `fix/pass45-local-build-import-regression-20260825` — merged by PR #82
Runtime-code `main` baseline: `1d6f57227bb15e84d7df911c192f53685b08544f`
Final PR #82 source head: `2079733e7e027587f1bc0925d7fffc5c44df69ed` — 20/20 source workflows green before merge
Target: UE 5.8.x Windows
User launcher: `START_HERE.cmd`

## 0. Purpose and authority

Pass 44 was source-green but factually rejected by the first local UE run. The user screenshots prove menu ~8 FPS, gameplay ~8–12 FPS, white weapon materials, wrong tactical topology, primitive/fantasy trees, blockout-grade world visuals and unresolved landmark identity.

**Pass 44 = RUNTIME REJECTED.**

Pass 45 is the corrective contract. Its merged source work is not runtime-verified. The 2026-08-25 completion audit found one genuine source omission that remained in this TZ: **B2 World proxy truth** still allowed visible BasicShape residential/environment families even though suitable imported assets existed. PR #81 merged that source correction.

The first factual local UE 5.8.1 build after PR #81 then found a second class of source defects that source-only CI did not expose: a real C++ compilation failure in the tactical-map table and a UE 5.8 Interchange API rejection for HMMWV/M2 GLB intake. PR #82 contains both corrections and passed 20/20 source workflows before merge. This still does **not** equal UE build/runtime verification; the next local build is authoritative.

Latest rejected gameplay evidence:
`RUNTIME_EVIDENCE/2026-08-24_PASS44_REJECTED/`

Pass 45 completion audit:
`OsterConflict/Docs/WorkReports/PASS45_COMPLETION_AUDIT_2026-08-25.md`

Local build/import rejection report:
`OsterConflict/Docs/WorkReports/PASS45_LOCAL_BUILD_IMPORT_REJECTION_2026-08-25.md`

Latest user-observed runtime/build evidence always overrides source/CI claims.

---

# 1. Authoritative 2026-08-25 local UE build rejection

The user launched `START_HERE.cmd` and selected **1. ЗВИЧАЙНА ГРА**.

## 1.1 Tactical-map compile blocker

UnrealBuildTool compiled the current source and rejected `OCTacticalMapVisual.cpp`:

`error C2131: expression did not evaluate to a constant`

Root cause:

- `Pass45ReferenceRoads` was declared `constexpr`;
- each entry contains `FVector2D`;
- this UE 5.8/MSVC toolchain does not accept that `FVector2D` constructor in the required constant-expression context.

Factual result:

- `Result: Failed (OtherCompilationError)`;
- UBT/launcher exit code **6**;
- gameplay did not start.

Merged PR #82 correction:

- table changed from `constexpr FPass45ReferenceRoadSegment[]` to namespace-scope `const FPass45ReferenceRoadSegment[]`;
- road data, iteration and `UE_ARRAY_COUNT` behavior are unchanged;
- regression verifier forbids restoring the invalid `constexpr` declaration;
- historical Pass44 verifier was forward-ported to semantic status checking so it cannot force old wording back into the ledger.

Status: **MERGED / CODED_UNTESTED** until a later local UE build exits 0.

## 1.2 HMMWV / M2 Interchange regression

The same normal launch found all local HMMWV, M2 and BTR-4 sources.

Factual import result:

- BTR-4 imported successfully to `/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus`;
- HMMWV failed because `InterchangeGenericCommonMeshesProperties.auto_detect_mesh_type` / `bAutoDetectMeshType` is deprecated/rejected by UE 5.8;
- M2 failed for the same reason;
- launcher correctly reported `HMMWV=0 M2=0 BTR4=1` and did not call the failed assets production-ready.

Merged PR #82 correction:

- deprecated `auto_detect_mesh_type` property removed;
- current `convert_statics_with_animated_transform_to_skeletals=false` policy used;
- importer still explicitly forces `IFMT_STATIC_MESH`, enables static import and disables skeletal import.

Status:

- BTR-4 production import: **LOCALLY CONFIRMED**, runtime/presentation still unverified;
- HMMWV/M2 importer correction: **MERGED / CODED_UNTESTED** until the next local import.

---

# 2. Authoritative unresolved gameplay/runtime facts

Because the 2026-08-25 attempt failed at compile time, it does not supersede the 2026-08-24 gameplay screenshots for visual/performance acceptance. Until a successful build reaches gameplay, these remain the runtime baseline:

- frontend/main menu about **8 FPS**;
- gameplay about **8–12 FPS** with severe lag/stutter;
- slowdown also visible in open/empty areas;
- multiple rack weapons white / missing intended authored materials;
- tactical `M` view based on wrong procedural topology;
- primitive/generic trees visibly inappropriate for Oster;
- flat/blockout environment presentation;
- Museum and Culture House not visually distinct enough;
- stale/over-layered Silpo/Museum/Culture presentation.

No source check may promote these to fixed until factual runtime proves it.

---

# 3. Root-cause and correction state

## 3.1 Performance

The bot population was a real unnecessary load but not the sole cause because the rejected menu itself was already ~8 FPS.

Current Pass 45 source corrections:

- normal route: DX11 + SM5 + HDR off + normal RHI threading;
- explicit compatibility route: adds `-norhithread` for factual A/B only;
- no implicit normal-game bot autofill;
- separate one-shot frontend/gameplay performance markers;
- no automatic resolution-scale downgrade to hide poor FPS;
- old 0.20 s × 40 full-world landmark reconciliation loop retired;
- compact 960×940 m render/cull budget applied;
- new B2 real-house/fence visual ISMs carry their own compact cull budget and no visual collision/navigation.

Required runtime markers include:

- `PASS45_RHI_MODE`
- `PASS45_FRONTEND_PERF_BASELINE`
- `PASS45_GAMEPLAY_PERF_BASELINE`

Acceptance target remains **>=30 FPS minimum** in frontend and gameplay, with no progressive collapse. This cannot be remeasured until the local build gate passes.

## 3.2 Primitive world / B2 World proxy truth

### Source completion state: **SOURCE INVENTORY CLOSED / CODED_UNTESTED**

The completion audit proved that `OCWorldSectorOster` still uses Engine BasicShapes as semantic placement/collision data. That is allowed only where the primitive is not the accepted player-facing production visual.

`UOCWorldProductionVisualsSubsystem` is now the single generic environment visual-conversion owner for B2.

It performs one bounded post-actor-BeginPlay conversion and then stops. It does **not** run another full-world repair loop.

Current ownership:

- `Ground`: compact cube remains physical carrier; player-facing material switches to imported `M_Inst_Landscape`.
- `Roads`: semantic slab/collision retained; visible material switches to imported `MI_Urb_Roa_Asphalt_01`.
- `Sidewalks`: semantic slab/collision retained; visible material switches to imported `MI_Urb_Roa_Sidewalk_01`.
- `Buildings`: source boxes become hidden collision/backstop; visible owner uses imported `SM_House_Var01` / `SM_House_Var02`.
- `ResidentialRoofs` / `ResidentialDetails`: hidden after complete residential conversion; imported house mesh owns visible roof/detail silhouette.
- `Fences`: visible public fence uses `SM_Fence_Var04`; source box remains hidden collision.
- `WoodFences`: visible owner `SM_Fence_Var01`.
- `MetalFences`: visible owner `SM_Fence_Var02`.
- `LightSheetFences`: visible owner `SM_Fence_Var03`.
- primitive trunk/crown tree families: hidden by current Pass 45 foliage guard; verified real pine candidates remain available.
- Stadion source proxies: already hidden; dedicated Stadion Oster presentation remains visual owner.
- Museum / Silpo / Culture House: dedicated shell owners remain authoritative and are not replaced by the generic B2 owner.
- `ReferenceMarkers` / developer labels: hidden from gameplay.
- compact hydrography/bridge blockout: currently creates no instances.

### Explicit remaining content gaps

- College / unrelated generic `LandmarkBlocks`, roofs/windows/details: no verified photo-faithful production College/civic mesh found. Do **not** disguise a random house asset as the College.
- `ParkGeometry` / `ParkDetails`: no verified complete park/plaza/bench/skate production set matching the current semantics. These remain non-production art gaps.
- suitable explicit oak asset remains unverified; do not fabricate an oak label.

These are now **classified content gaps**, not an un-audited source omission. They may not be called production-ready.

B2 runtime markers:

- `PASS45_B2_PRODUCTION_VISUALS_READY`
- `PASS45_B2_PRODUCTION_VISUALS_FAIL`
- `PASS45_B2_RESIDENTIAL_VISUAL_GAP`
- `PASS45_B2_FENCE_FAMILY_READY`
- `PASS45_B2_FENCE_FAMILY_GAP`
- `PASS45_B2_REMAINING_CONTENT_GAPS`

New real visual culls:

- houses: 300–650 m;
- fences: 60–280 m;
- visual collision/navigation disabled;
- hidden primitive collision/backstop retained.

## 3.3 Vegetation

Source state: **CODED_UNTESTED**.

- all eight Cylinder/Sphere tree proxy families are hidden from normal gameplay;
- real `SM_Pine_Tree_01` / `SM_Pine_Tree_03` candidates exist and are used by the Museum tree owner;
- required character: tall pine/conifer woodland plus appropriate oak where supported;
- oak remains explicit content gap until a suitable real asset is verified;
- no old birch/poplar/spherical proxy family may return merely for verifier compatibility.

## 3.4 Tactical map

Source state: **BUILD FIX MERGED / CODED_UNTESTED**.

The `M` map no longer treats procedural world ISMs as topology truth.

- compact 960×940 m north-up projection retained;
- dedicated 640×630 user-reference-traced street layer used;
- Museum / Culture House / Silpo / central park / Stadium use common `FOCGeoReference` authority;
- tactical-polish icons use the same authority;
- old Z=2 residential dimmer retired;
- giant procedural X/diagonal road pattern must not return;
- invalid `constexpr` reference-road table declaration found by local UE build has been replaced with a normal `const` table and merged by PR #82.

A successful local rebuild is now required before any tactical-map runtime screenshot can count.

## 3.5 Weapon material/texture truth

Audit implementation: **CODED_UNTESTED**.
Dependency closure: **PENDING FRESH LOCAL UE PREFLIGHT**.

All 11 required visuals now produce:

`weapon -> exact mesh -> material slots -> material assets -> used texture dependencies -> fresh dependency load result`

The NullRHI preflight writes `required_weapon_material_texture_dependencies.json` and separate mesh/material/texture statuses.

A white/default/null/BasicShape material, placeholder texture, missing texture dependency, or material with no discoverable texture dependency remains FAIL / CONTENT GAP.

M16/M4 are not claimed because no verified production payload exists.

## 3.6 Landmark ownership

Source state: **CODED_UNTESTED**.

Single visible shell owners:

- Museum: `R138_MuseumHighFidelityArchitecture`;
- Museum R13.7: reference/detail/interactivity parent only;
- Silpo: `R140_SilpoPhotoModel`;
- Culture House: `R146_CultureHouseAuthoritative`;
- Stadium authority unchanged.

Historical duplicate/recovery layers may not become second visible shell owners.

---

# 4. Implementation phases and current status

## Phase A — measurable performance baseline

- A1 RHI-thread A/B launcher: **MERGED / CODED_UNTESTED**.
- A2 frontend/gameplay performance instrumentation: **MERGED / CODED_UNTESTED**.
- A3 repeated full-world repair loop retirement / shell ownership: **MERGED / CODED_UNTESTED**.
- A4 compact sector render budget: **MERGED / CODED_UNTESTED**.

## Phase B — visible production proxy cleanup

- B1 primitive tree visuals retired: **MERGED / CODED_UNTESTED**.
- B2 World proxy truth inventory/ownership: **MERGED BY PR #81 / CODED_UNTESTED**.
- B2 remaining College/park/oak gaps: **EXPLICIT CONTENT GAP**, not production-ready.

## Phase C — tactical map topology

- C1 reference-traced compact topology: **MERGED; LOCAL BUILD REJECTED ON CONSTEXPR TABLE; PR #82 FIX MERGED / CODED_UNTESTED**.
- C2 runtime `M` screenshot: **PENDING SUCCESSFUL BUILD + RUNTIME**.

## Phase D — landmark ownership

- D1 Museum single-shell contract: **MERGED / CODED_UNTESTED**.
- D2 Culture House single-shell contract: **MERGED / CODED_UNTESTED**.
- D3 Silpo single-shell contract: **MERGED / CODED_UNTESTED**.

## Phase E — weapon material dependency closure

- audit implementation: **MERGED / CODED_UNTESTED**;
- actual 11-weapon binary-content gap closure: **PENDING FRESH LOCAL PREFLIGHT**.

## Phase F — production vehicle intake

- BTR-4 source/import: **LOCAL IMPORT CONFIRMED 2026-08-25; RUNTIME UNTESTED**.
- HMMWV source: **FOUND LOCALLY; UE 5.8 IMPORT API FIX MERGED / CODED_UNTESTED**.
- M2 source: **FOUND LOCALLY; UE 5.8 IMPORT API FIX MERGED / CODED_UNTESTED**.

---

# 5. Stale behavior forbidden from returning

1. implicit normal-game bot autofill;
2. 2.4 km procedural battlefield;
3. map auto-fit from arbitrary component extents;
4. grey BasicShape weapon-material repair;
5. primitive Cylinder/Sphere trees as accepted vegetation;
6. repeated competing landmark rebuild loops;
7. stale verifier requirements that restore a known regression;
8. READY claims contradicted by latest runtime/build evidence;
9. hidden resolution downgrade used to disguise FPS collapse;
10. visible generic residential/fence BasicShape art when verified imported replacements exist;
11. duplicate collision/navigation on the new B2 production visual layer;
12. permanent polling after B2 visual conversion succeeds;
13. `constexpr` Pass45 `FVector2D` road table that UE 5.8/MSVC factually rejects;
14. deprecated `auto_detect_mesh_type` / `bAutoDetectMeshType` in current UE 5.8 Interchange intake.

---

# 6. Acceptance gates

Pass 45 cannot be called `VERIFIED RUNTIME` until all applicable factual gates pass.

## Gate 0 — build

- `OCTacticalMapVisual.cpp` compiles without C2131;
- UBT exits 0;
- only then proceed to frontend/runtime acceptance.

## Gate 0B — vehicle import

- BTR canonical asset remains loadable;
- HMMWV import completes without deprecated-property exception;
- M2 import completes without deprecated-property exception;
- launcher reports each item independently and truthfully.

## Gate 1 — frontend

- menu reliably opens;
- no RenderTargetPool startup crash;
- **>=30 FPS minimum**;
- record `PASS45_FRONTEND_PERF_BASELINE` and selected RHI mode.

## Gate 2 — gameplay performance

- normal local game, bots off;
- no progressive 8–12 FPS collapse;
- **>=30 FPS minimum**;
- no hidden quality downgrade;
- stop immediately if thermals become unsafe.

## Gate 3 — environment / B2

- no fantasy primitive tree forest;
- residential houses are imported models rather than brown cubes;
- fence families use imported visual meshes;
- asphalt/sidewalk/ground no longer show the old flat BasicShape tint;
- no gross house stretching/floating or duplicated collision behavior;
- College/park gaps remain visibly non-production until real matching assets exist.

## Gate 4 — tactical map

- `M` resembles compact central Oster reference topology;
- synthetic giant X/diagonals gone;
- player marker visible;
- Museum / Culture House / Silpo / Park / Stadium distinct and co-located with semantic icons.

## Gate 5 — landmarks

- Museum and Culture House visibly separate and correctly located;
- Silpo stale/duplicate identity removed;
- one visible shell owner per landmark.

## Gate 6 — weapons

- fresh dependency audit completes for all 11 classes;
- no white/default rack slot accepted;
- missing dependencies are named explicitly;
- runtime rack screenshot required.

## Gate 7 — CI/evidence

- PR #82 source head `2079733e7e027587f1bc0925d7fffc5c44df69ed`: **20/20 workflows success**;
- source CI alone never promotes build/runtime status;
- latest user screenshot/log/build transcript overrides older source assumptions.

---

# 7. Completion checklist

- [x] Pass 44 factual runtime classified **RUNTIME REJECTED**.
- [x] Rejected screenshots archived with manifest.
- [x] RHI-thread A/B source correction implemented.
- [x] Frontend/gameplay performance markers implemented.
- [x] 40-pass landmark world scan retired.
- [x] Compact 960×940 m cull budget implemented.
- [x] Primitive tree visuals retired; real pine candidates retained.
- [x] Tactical map reference topology implemented.
- [x] Museum/Silpo/Culture single-shell ownership implemented.
- [x] 11-weapon mesh/material/texture dependency audit implemented.
- [x] B2 visible BasicShape inventory completed and merged by PR #81.
- [x] Imported residential house/fence visual owners coded; old boxes retained only as hidden backstops where conversion succeeds.
- [x] Imported ground/asphalt/sidewalk materials coded after source actor BeginPlay.
- [x] New B2 real visual layer has compact culls and no collision/navigation duplication.
- [x] College / park / oak remaining visual work classified as explicit content gaps rather than falsely READY.
- [x] First local UE build after PR #81 captured and classified **LOCAL UE BUILD REJECTED**.
- [x] C2131 tactical-map table root cause diagnosed and source fix merged by PR #82.
- [x] HMMWV/M2 deprecated Interchange property root cause diagnosed and source fix merged by PR #82.
- [x] BTR-4 local production import factually confirmed.
- [x] PR #82 final source head passed **20/20 workflows** and merged into `main` as runtime-code commit `1d6f57227bb15e84d7df911c192f53685b08544f`.
- [ ] Later local UE build exits 0 and promotes the build fix to VERIFIED BUILD.
- [ ] HMMWV and M2 re-import successfully with the current UE 5.8 property contract.
- [ ] Fresh local UE 5.8 weapon dependency report produced and its actual binary-content gaps closed.
- [ ] Local UE 5.8 frontend acceptance passed.
- [ ] Local UE 5.8 gameplay/performance acceptance passed.
- [ ] Local UE screenshots accept B2 visuals, trees, tactical map, landmarks and weapon rack.

**Current overall status: PASS 45 ACTIVE / LOCAL UE BUILD REJECTED / PR #82 FIX MERGED / SOURCE CI 20/20 GREEN / LOCAL REBUILD REQUIRED.**
