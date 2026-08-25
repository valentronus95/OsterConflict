# OSTER CONFLICT — WORK LEDGER

> Current authoritative work state. Latest user runtime evidence overrides source-only claims and historical pass notes.
> Historical Pass 1–44 details remain preserved in Git history/reports; they are chronology, not current rules.

## 1. Current context

- Repository: `valentronus95/OsterConflict`
- Current audited `main` baseline: `8d66e54c59d965b838931e1fa1d473e5ebb39fc9`.
- Pass 45 original source branch: `fix/runtime-recovery-pass-45-20260824` — merged by PR #79.
- Active completion branch: `fix/pass45-completion-audit-20260825` → `main`.
- Pass state token: **PASS 45 ACTIVE / COMPLETION CORRECTION CODED_UNTESTED**.
- Canonical TZ: `PASS45_RUNTIME_RECOVERY_TZ.md`.
- Completion audit: `OsterConflict/Docs/WorkReports/PASS45_COMPLETION_AUDIT_2026-08-25.md`.
- Latest factual runtime evidence: `RUNTIME_EVIDENCE/2026-08-24_PASS44_REJECTED/`.
- UE target: 5.8.x Windows.
- Project: `OsterConflict/OsterConflict.uproject`.
- User launcher: **only `START_HERE.cmd`**.
- Hard map reference: `REFERENCE_PHOTOS/map_extent/oster_central_playable_area_20260824.jpg`.

## 2. Status rules

- `IN_PROGRESS` — source implementation or required content is incomplete.
- `CODED_UNTESTED` — source correction exists but local UE 5.8 has not accepted it.
- `CONTENT GAP` — required matching production content is absent/unverified; never fake READY.
- `RUNTIME REJECTED` — factual local run disproved the claimed result.
- `VERIFIED BUILD` — a later factual UE build removes a build blocker.
- `VERIFIED RUNTIME` — only factual local UE/user playtest can assign this.
- Green source CI is never runtime acceptance.
- Latest screenshot/log outranks historical source/verifier assumptions.
- Mesh load success is weaker than authored material/texture truth.
- Stale verifier expectations must be retired instead of restoring known regressions.

## 3. Latest authoritative runtime

The latest factual UE runtime remains the 2026-08-24 run after Pass 44.

It proves:

- menu about **8 FPS**;
- gameplay about **8–12 FPS** with heavy lag/stutter;
- slowdown also in visually empty areas;
- malformed procedural tactical topology;
- multiple white/missing-material rack weapons;
- primitive/fantasy tree silhouettes;
- blockout-grade flat environment visuals;
- Museum / Culture House visual identity not acceptably separated;
- stale/over-layered Silpo/Museum/Culture presentation.

**Pass 44 verdict: RUNTIME REJECTED.**

No newer factual UE runtime exists yet. Every Pass 45 runtime-facing correction remains `CODED_UNTESTED`.

## 4. Completion audit finding — 2026-08-25

The user explicitly required a full check of whether the Pass 45 TZ had actually been completed.

The audit found a genuine source omission:

### B2 World proxy truth was still incomplete

`OCWorldSectorOster` still visibly used Engine BasicShape cubes/tints for generic residential/environment families even though suitable imported assets existed.

Current completion correction:

- new single generic visual owner: `UOCWorldProductionVisualsSubsystem`;
- conversion occurs after source actor `BeginPlay` so old R11 BasicShape tint cannot overwrite production materials;
- hidden source boxes remain collision/backstop only;
- visible generic houses use imported `SM_House_Var01` / `SM_House_Var02`;
- public/wood/metal/sheet fence visuals use imported `SM_Fence_Var04/01/02/03` respectively;
- ground material uses imported `M_Inst_Landscape`;
- road material uses imported `MI_Urb_Roa_Asphalt_01`;
- sidewalk material uses imported `MI_Urb_Roa_Sidewalk_01`;
- real visual layer has no collision/navigation duplication;
- house cull: 300–650 m;
- fence cull: 60–280 m;
- no permanent polling after conversion succeeds;
- each fence family has independent fail-visible readiness.

### B2 remaining classified content gaps

- College / unrelated generic civic `Landmark*` art: **CONTENT GAP**. No verified photo-faithful College mesh; random replacement forbidden.
- Park geometry/details set: **CONTENT GAP**. No verified complete park/plaza/bench/skate production set.
- oak asset: **CONTENT GAP** until a suitable real oak is verified.
- hydrography/bridges: no current compact-area instances, so imported bridge assets are not spawned without user-approved map evidence.

The B2 inventory decision is now source-complete, but runtime placement/scale/material quality remains `CODED_UNTESTED`.

## 5. Active requirements

| ID | Requirement | Repeat | Status | Current action |
|---|---|---:|---|---|
| PERF-COLLAPSE-001 | Stop frontend/gameplay collapse to ~8–12 FPS | ≥6 | CODED_UNTESTED | RHI A/B, perf markers, mutation cleanup, compact culls coded. New B2 real visual layer also has bounded culls. Runtime >=30 FPS required. |
| UI-MENU-001 | Frontend/menu stable and usable | ≥9 | CODED_UNTESTED | Pass43 lifecycle protection + normal RHI threading/compat route coded. Runtime must prove no crash and >=30 FPS. |
| VIS-GRAPHICS-QUALITY-001 | Restore readable non-blockout visual quality without hidden resolution downgrade | ≥4 | CODED_UNTESTED / CONTENT GAP | B2 generic residential/fence/material source correction coded. College/park art gaps remain explicit. Native 100% clarity target retained. |
| VIS-TREES-001 | Tall pine/conifer character + appropriate oak; no fantasy primitive trees | ≥2 | CODED_UNTESTED / CONTENT GAP | Primitive trees retired; real pines verified; oak remains gap. |
| UI-TACTICAL-MAP-001 | `M` matches compact central-Oster topology | ≥4 | CODED_UNTESTED | Reference-traced topology + common `FOCGeoReference` coded. Runtime screenshot required. |
| MAP-EXTENT-001 | Keep compact central Oster battlefield | ≥2 | CODED_UNTESTED / RETAIN | 960×940 m hard extent retained. Never restore 2.4 km map. |
| LOC-MUSEUM-001 | Museum visible/unique near actual live spawn | ≥10 | CODED_UNTESTED | R13.8 single shell owner; runtime identity still required. |
| LOC-CULTURE-001 | Culture House separate from Museum | ≥2 | CODED_UNTESTED | R14.6 single shell owner at Culture geo anchor. |
| LOC-SILPO-001 | Silpo one site owner, no stale duplicate | ≥2 | CODED_UNTESTED | R14.0 single shell owner; runtime inspection required. |
| WEAPON-MATERIAL-001 | 11 rack weapons have authored material/texture truth | ≥10 | CODED_UNTESTED / CONTENT CHECK | Fresh NullRHI dependency audit coded. Actual local report still required to identify/close binary gaps. |
| GAME-WEAPONS-001 | 11 grounded pickups near Museum spawn | ≥9 | CODED_UNTESTED | Grounding retained; visual/material gate separate. |
| GAME-SPAWN-001 | Live pawn spawns near Museum BASE | ≥9 | CODED_UNTESTED | Actual-pawn distance correction retained; runtime proof pending. |
| VIS-GRASS-001 | Natural grass without FPS collapse | ≥6 | CODED_UNTESTED | Density constrained; runtime baseline required. |
| VEH-PICKUP-001 | Real HMMWV + M2 | ≥5 | IN_PROGRESS / ASSET CHECK | Fail-visible asset truth retained. Not allowed to block perf diagnosis. |
| ASSET-BTR-001 | Real BTR-4/Bucephalus | ≥5 | IN_PROGRESS / CONTENT GAP | Missing source remains explicit gap. |
| ASSET-M16-M4-001 | M16/M4 production visuals | ≥2 | IN_PROGRESS / CONTENT GAP | No verified payload; do not claim connected. |
| GAME-VEHICLE-INPUT-001 | WASD/mouse after vehicle exit | 1 | CODED_UNTESTED | Pass41 source recovery retained. |

## 6. Pass 44 behavior retained unless disproved

1. no implicit normal-local 16-bot autofill;
2. no 2.4 km playable/tactical map;
3. no old edge BASE/test-lane/vehicle spawn coordinates;
4. no ±920 m coordinate-based BASE role discriminator;
5. no grey BasicShape weapon-material repair;
6. no all-or-nothing vehicle import where missing BTR blocks HMMWV/M2;
7. no optimistic READY text for missing production sources;
8. no stale verifier may force these regressions back.

## 7. Pass 45 execution state

1. Runtime evidence/TZ locked. **DONE**.
2. Pass44 marked rejected. **DONE**.
3. DX11/SM5 RHI-thread A/B route. **MERGED / CODED_UNTESTED**.
4. Frontend/gameplay performance markers. **MERGED / CODED_UNTESTED**.
5. Repeated landmark mutation cleanup / single ownership. **MERGED / CODED_UNTESTED**.
6. Compact 960×940 m render budget. **MERGED / CODED_UNTESTED**.
7. Primitive tree visual retirement. **MERGED / CODED_UNTESTED; OAK GAP**.
8. Tactical-map reference topology. **MERGED / CODED_UNTESTED**.
9. 11-weapon dependency audit implementation. **MERGED / CODED_UNTESTED; LOCAL REPORT PENDING**.
10. Historical verifier retirement/forward-port. **DONE FOR MERGED SOURCE SUITE**.
11. Original Pass45 merge PR #79. **DONE**.
12. **B2 World proxy truth completion audit. SOURCE CORRECTION ACTIVE / CODED_UNTESTED.**
13. Full current-head source CI for completion correction. **PENDING**.
14. Merge completion correction into `main` only after green current-head CI. **PENDING**.
15. Local UE test: frontend first, then gameplay. **PENDING — AUTHORITATIVE ACCEPTANCE GATE**.

## 8. Runtime acceptance gates

- Frontend: no historical RenderTargetPool crash and **>=30 FPS minimum**.
- Gameplay: bots off, no progressive collapse, **>=30 FPS minimum**.
- Graphics/B2: real residential/fence visuals, real road/sidewalk/ground materials, no gross stretching/floating, no duplicate collision, no hidden quality downgrade.
- Trees: no primitive fantasy forest; real pine/conifer character visible.
- Tactical map: recognizable compact Oster topology, no giant synthetic X/diagonals, player marker visible.
- Landmarks: Museum/Culture visibly separate; Silpo stale duplicate absent.
- Weapons: fresh 11-class dependency report; no white/default slot accepted.
- CI: required but never sufficient for runtime verification.

**Current overall status: PASS 45 ACTIVE / COMPLETION CORRECTION CODED_UNTESTED / CURRENT-HEAD CI PENDING / LOCAL UE RUNTIME PENDING.**
