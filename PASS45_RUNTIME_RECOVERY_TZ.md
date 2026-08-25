# OSTER CONFLICT — PASS 45 RUNTIME RECOVERY TZ

Date: 2026-08-24
Completion audit: 2026-08-25
Status: **MERGED BASE + COMPLETION CORRECTION CODED_UNTESTED / RUNTIME UNTESTED**
Original source branch: `fix/runtime-recovery-pass-45-20260824` — merged by PR #79
Completion branch: `fix/pass45-completion-audit-20260825`
Current audited main baseline: `8d66e54c59d965b838931e1fa1d473e5ebb39fc9`
Target: UE 5.8.x Windows
User launcher: `START_HERE.cmd`

## 0. Purpose and authority

Pass 44 was source-green but factually rejected by the first local UE run. The user screenshots prove menu ~8 FPS, gameplay ~8–12 FPS, white weapon materials, wrong tactical topology, primitive/fantasy trees, blockout-grade world visuals and unresolved landmark identity.

**Pass 44 = RUNTIME REJECTED.**

Pass 45 is the corrective contract. Its merged source work is not runtime-verified. The 2026-08-25 completion audit found one genuine source omission that remained in this TZ: **B2 World proxy truth** still allowed visible BasicShape residential/environment families even though suitable imported assets existed.

Latest rejected runtime evidence:
`RUNTIME_EVIDENCE/2026-08-24_PASS44_REJECTED/`

Pass 45 completion audit:
`OsterConflict/Docs/WorkReports/PASS45_COMPLETION_AUDIT_2026-08-25.md`

Latest user-observed runtime always overrides source/CI claims.

---

# 1. Authoritative unresolved runtime facts

Until a newer local UE 5.8 run exists, these remain the runtime baseline:

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

# 2. Root-cause and correction state

## 2.1 Performance

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

Acceptance target remains **>=30 FPS minimum** in frontend and gameplay, with no progressive collapse.

## 2.2 Primitive world / B2 World proxy truth

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

## 2.3 Vegetation

Source state: **CODED_UNTESTED**.

- all eight Cylinder/Sphere tree proxy families are hidden from normal gameplay;
- real `SM_Pine_Tree_01` / `SM_Pine_Tree_03` candidates exist and are used by the Museum tree owner;
- required character: tall pine/conifer woodland plus appropriate oak where supported;
- oak remains explicit content gap until a suitable real asset is verified;
- no old birch/poplar/spherical proxy family may return merely for verifier compatibility.

## 2.4 Tactical map

Source state: **CODED_UNTESTED**.

The `M` map no longer treats procedural world ISMs as topology truth.

- compact 960×940 m north-up projection retained;
- dedicated 640×630 user-reference-traced street layer used;
- Museum / Culture House / Silpo / central park / Stadium use common `FOCGeoReference` authority;
- tactical-polish icons use the same authority;
- old Z=2 residential dimmer retired;
- giant procedural X/diagonal road pattern must not return.

Required runtime screenshot still pending.

## 2.5 Weapon material/texture truth

Audit implementation: **CODED_UNTESTED**.
Dependency closure: **PENDING FRESH LOCAL UE PREFLIGHT**.

All 11 required visuals now produce:

`weapon -> exact mesh -> material slots -> material assets -> used texture dependencies -> fresh dependency load result`

The NullRHI preflight writes `required_weapon_material_texture_dependencies.json` and separate mesh/material/texture statuses.

A white/default/null/BasicShape material, placeholder texture, missing texture dependency, or material with no discoverable texture dependency remains FAIL / CONTENT GAP.

M16/M4 are not claimed because no verified production payload exists.

## 2.6 Landmark ownership

Source state: **CODED_UNTESTED**.

Single visible shell owners:

- Museum: `R138_MuseumHighFidelityArchitecture`;
- Museum R13.7: reference/detail/interactivity parent only;
- Silpo: `R140_SilpoPhotoModel`;
- Culture House: `R146_CultureHouseAuthoritative`;
- Stadium authority unchanged.

Historical duplicate/recovery layers may not become second visible shell owners.

---

# 3. Implementation phases and current status

## Phase A — measurable performance baseline

- A1 RHI-thread A/B launcher: **MERGED / CODED_UNTESTED**.
- A2 frontend/gameplay performance instrumentation: **MERGED / CODED_UNTESTED**.
- A3 repeated full-world repair loop retirement / shell ownership: **MERGED / CODED_UNTESTED**.
- A4 compact sector render budget: **MERGED / CODED_UNTESTED**.

## Phase B — visible production proxy cleanup

- B1 primitive tree visuals retired: **MERGED / CODED_UNTESTED**.
- B2 World proxy truth inventory/ownership: **COMPLETION CORRECTION CODED_UNTESTED**.
- B2 remaining College/park/oak gaps: **EXPLICIT CONTENT GAP**, not production-ready.

## Phase C — tactical map topology

- C1 reference-traced compact topology: **MERGED / CODED_UNTESTED**.
- C2 runtime `M` screenshot: **PENDING RUNTIME**.

## Phase D — landmark ownership

- D1 Museum single-shell contract: **MERGED / CODED_UNTESTED**.
- D2 Culture House single-shell contract: **MERGED / CODED_UNTESTED**.
- D3 Silpo single-shell contract: **MERGED / CODED_UNTESTED**.

## Phase E — weapon material dependency closure

- audit implementation: **MERGED / CODED_UNTESTED**;
- actual 11-weapon binary-content gap closure: **PENDING FRESH LOCAL PREFLIGHT**.

---

# 4. Stale behavior forbidden from returning

1. implicit normal-game bot autofill;
2. 2.4 km procedural battlefield;
3. map auto-fit from arbitrary component extents;
4. grey BasicShape weapon-material repair;
5. primitive Cylinder/Sphere trees as accepted vegetation;
6. repeated competing landmark rebuild loops;
7. stale verifier requirements that restore a known regression;
8. READY claims contradicted by latest runtime evidence;
9. hidden resolution downgrade used to disguise FPS collapse;
10. visible generic residential/fence BasicShape art when verified imported replacements exist;
11. duplicate collision/navigation on the new B2 production visual layer;
12. permanent polling after B2 visual conversion succeeds.

---

# 5. Runtime acceptance gates

Pass 45 cannot be called `VERIFIED RUNTIME` until all applicable factual gates pass.

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

- current-head source CI green;
- CI alone never promotes runtime status;
- latest user screenshot/log overrides older source assumptions.

---

# 6. Completion checklist

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
- [x] **B2 visible BasicShape inventory completed after user-requested completion audit.**
- [x] Imported residential house/fence visual owners coded; old boxes retained only as hidden backstops where conversion succeeds.
- [x] Imported ground/asphalt/sidewalk materials coded after source actor BeginPlay.
- [x] New B2 real visual layer has compact culls and no collision/navigation duplication.
- [x] College / park / oak remaining visual work classified as explicit content gaps rather than falsely READY.
- [ ] Fresh local UE 5.8 weapon dependency report produced and its actual binary-content gaps closed.
- [ ] Local UE 5.8 frontend acceptance passed.
- [ ] Local UE 5.8 gameplay/performance acceptance passed.
- [ ] Local UE screenshots accept B2 visuals, trees, tactical map, landmarks and weapon rack.

**Current overall status: PASS 45 SOURCE COMPLETION CORRECTION IN PROGRESS / CODED_UNTESTED / LOCAL UE RUNTIME REQUIRED.**
