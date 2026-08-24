# OSTER CONFLICT — WORK LEDGER

> Current authoritative work state. Latest user runtime evidence overrides source-only claims and historical pass notes.
> Historical Pass 1–44 details remain preserved in Git history and reports; they are chronology, not current rules.

## 1. Current context

- Repository: `valentronus95/OsterConflict`
- Current `main` baseline: `bf483f8dc473862e0d3ce6468db44f025abbeef1` (merged Pass 44 / PR #78)
- Active correction branch: `fix/runtime-recovery-pass-45-20260824` → `main`
- Canonical corrective TZ: `PASS45_RUNTIME_RECOVERY_TZ.md`
- Latest runtime evidence: `RUNTIME_EVIDENCE/2026-08-24_PASS44_REJECTED/`
- UE target: 5.8.x Windows
- Project: `OsterConflict/OsterConflict.uproject`
- User-facing launcher: **only `START_HERE.cmd`**.
- Current root authority/conflict policy: `AGENTS.md`.
- Hard playable-map reference: `REFERENCE_PHOTOS/map_extent/oster_central_playable_area_20260824.jpg`.
- Current prohibition: **no new decorative layers until frontend/gameplay performance, topology, landmark ownership and material truth are recovered.**

## 2. Status rules

- `IN_PROGRESS` — latest runtime proves the problem still exists, or implementation is incomplete.
- `CODED_UNTESTED` — a Pass 45 source correction exists but has not yet been accepted by a local UE 5.8 runtime.
- `RUNTIME REJECTED` — a factual local run disproved the claimed result.
- `VERIFIED BUILD` — build blocker removed by a later factual build.
- `VERIFIED RUNTIME` — only after factual user/local UE runtime demonstrates the requested behavior.
- Green source CI is never runtime acceptance.
- Latest screenshot/log outranks an older verifier.
- Authored material/texture truth outranks mesh-load success or coloured fallback.
- Missing production source remains a content gap, never READY.
- A stale verifier must be updated/retired instead of restoring obsolete behavior.

## 3. Latest authoritative user runtime — 2026-08-24 after Pass 44 merge

This is the first factual local runtime after Pass 44 merged to `main`.

Runtime evidence proves:

- frontend/main menu renders but is already about **8 FPS**;
- gameplay is roughly **8–12 FPS** with heavy lag/stutter;
- severe slowdown also occurs in visually empty fields, so implicit bots or one dense foliage patch cannot be the sole root cause;
- tactical map is smaller than the historical 2.4 km version but still shows malformed procedural topology, including oversized straight/diagonal road geometry;
- multiple weapon-rack models render white / without intended authored materials;
- the held AK material is readable while nearby pickup weapons are not, proving `mesh loads` != `authored materials ready`;
- primitive tree silhouettes remain visible and do not match the required Oster vegetation character (tall pine/conifer forest and appropriate oak assets);
- world presentation is visibly blockout-grade: flat green ground, primitive shapes, crude civic/landmark shells;
- Museum and Culture House still do not present as two clearly separated and correctly placed visible landmarks;
- stale/over-layered location construction remains around Silpo/Museum/Culture House.

**Pass 44 verdict: RUNTIME REJECTED.**

Pass 44 source CI remains historical source evidence only.

Evidence sheet + manifest:
`RUNTIME_EVIDENCE/2026-08-24_PASS44_REJECTED/`

## 4. Current root-cause findings

### PERF-ROOT-001 — frontend collapse precedes gameplay population

The screenshot with the main menu at ~8 FPS makes bot population an incomplete diagnosis. Current normal launch still uses DX11/SM5/no-HDR plus `-norhithread`, retained from the earlier renderer-crash recovery path. Pass 45 must perform an explicit DX11/SM5 RHI-thread A/B test rather than silently keeping compatibility mode as the permanent baseline.

### PERF-ROOT-002 — delayed runtime mutation debt

The project contains multiple post-BeginPlay landmark replacement/recovery/guard subsystems. Repeated actor/component scans, ISM removal, `MarkRenderStateDirty`, delayed rebuilds and compatibility ownership loops are now treated as a first-class performance/stability risk. One landmark must have one current placement/visibility owner.

### VIS-ROOT-001 — primitive world remains visible production output

`OCWorldSectorOster` still uses Engine BasicShapes for major world families. Cylinder/Sphere tree families and cube-based environment authoring explain the visible low-fidelity blockout result. Compact bounds reduced spatial extent but did not replace production visuals.

### MAP-ROOT-001 — Pass 44 bounded the wrong topology

Pass 44 constrained tactical-map projection but left `BuildRoadNetwork()` procedural straight/diagonal blockout as the source geometry. Pass 45 must rebuild tactical topology from the authoritative compact central-Oster reference and geo anchors.

### WEAPON-ROOT-001 — mesh load is not material readiness

Pass 44 correctly retired grey BasicShape material repair. The latest white rack weapons are explicit authored-material gaps. Pass 45 requires per-weapon mesh → material → texture dependency truth for all 11 required classes.

### LANDMARK-ROOT-001 — distinct coordinates do not guarantee distinct runtime identity

Museum and Culture House geo references are not equal, but runtime still fails visual separation. The defect is treated as ownership/presentation layering, not merely one bad coordinate constant.

## 5. Active requirements

| ID | Requirement | Repeat | Status | Current Pass 45 action |
|---|---|---:|---|---|
| PERF-COLLAPSE-001 | Stop frontend/gameplay collapse to ~8–12 FPS | ≥6 | IN_PROGRESS | Pass 45: renderer/RHI-thread A/B, frame-domain markers, delayed mutation audit, compact cull rebudget. Minimum acceptance >=30 FPS; no progressive collapse. |
| UI-MENU-001 | Frontend/menu stable and usable | ≥9 | IN_PROGRESS | Menu no longer crashes but ~8 FPS is runtime failure. Keep Pass 43 lifecycle protections; test normal RHI threading vs explicit compatibility route. |
| VIS-GRAPHICS-QUALITY-001 | Restore readable non-blockout visual quality without hidden resolution downgrade | ≥4 | IN_PROGRESS | Keep native 100% clarity target. Do not hide perf failure by lowering render scale. Remove visible production proxies instead. |
| VIS-TREES-001 | Oster vegetation: tall pine/conifer forest + appropriate oak, no fantasy primitive trees | ≥2 | IN_PROGRESS | Retire visible Cylinder/Sphere tree families from normal gameplay; inventory real foliage assets first. Missing real oak/pine remains content gap. |
| UI-TACTICAL-MAP-001 | `M` map matches real compact central-Oster topology | ≥4 | IN_PROGRESS | Pass 44 bounds kept; procedural road/blockout topology rejected. Rebuild from authoritative map reference + geo anchors; new runtime screenshot mandatory. |
| MAP-EXTENT-001 | Keep compact central Oster battlefield | ≥2 | CODED_UNTESTED / RETAIN | Pass 44 960×940 m compact bound remains current unless new user evidence changes it. Do not restore 2.4 km map. |
| LOC-MUSEUM-001 | Museum visibly present and uniquely owned near live spawn | ≥10 | IN_PROGRESS | Consolidate Museum placement/visibility owner; historical recovery/detail layers may not fight current shell. |
| LOC-CULTURE-001 | Culture House visually and spatially separate from Museum | ≥2 | IN_PROGRESS | Distinct geo anchor exists; runtime identity still fails. One authoritative owner at Culture House anchor only. |
| LOC-SILPO-001 | Silpo one authoritative site owner, no stale duplicate signage/geometry | ≥2 | IN_PROGRESS | Consolidate replacement/detail stack and remove repeated full-site mutation after stabilization. |
| WEAPON-MATERIAL-001 | All required rack weapons use authored materials/textures | ≥10 | IN_PROGRESS / CONTENT CHECK | White/default material slots are runtime FAIL. Build per-weapon dependency audit; no BasicShape fallback. |
| GAME-WEAPONS-001 | 11 grounded pickup classes near actual Museum spawn | ≥9 | CODED_UNTESTED | Grounding logic retained; visual/material acceptance now separated from mere pickup existence. |
| GAME-SPAWN-001 | Actual live pawn spawns near Museum BASE | ≥9 | CODED_UNTESTED | Pass 44 pawn-distance correction remains but latest screenshots do not establish final visual Museum acceptance. |
| VIS-GRASS-001 | Natural grass without FPS collapse | ≥6 | IN_PROGRESS | Keep density constrained until performance baseline recovered. Grass cannot be blamed as sole root because menu already runs ~8 FPS. |
| VEH-PICKUP-001 | Real HMMWV + M2 Browning | ≥5 | IN_PROGRESS / ASSET CHECK | Preserve fail-visible asset truth. Do not let vehicle content work block performance recovery. |
| ASSET-BTR-001 | Real BTR-4/Bucephalus | ≥5 | IN_PROGRESS / CONTENT GAP | No fake READY. Missing real source remains explicit gap. |
| ASSET-M16-M4-001 | M16/M4 production visuals | ≥2 | IN_PROGRESS / CONTENT GAP | No verified M16/M4 payload in checked repository/tree/history. Do not claim connected. |
| GAME-VEHICLE-INPUT-001 | WASD/mouse after vehicle exit | 1 | CODED_UNTESTED | Pass 41 recovery remains; not current primary blocker. |

## 6. Pass 44 behavior retained unless disproved

The following Pass 44 corrections remain current because the latest run does not justify restoring the old behavior:

1. no implicit normal-local 16-bot autofill;
2. no 2.4 km playable/tactical map;
3. no old edge BASE/test-lane/vehicle spawn coordinates;
4. no coordinate-based ±920 m primary/secondary BASE discriminator;
5. no grey BasicShape weapon-material “repair”;
6. no all-or-nothing vehicle import where missing BTR blocks HMMWV/M2;
7. no optimistic launcher READY text for missing production sources;
8. no old verifier may force those regressions back.

## 7. Pass 45 execution order

1. Lock `PASS45_RUNTIME_RECOVERY_TZ.md` and runtime evidence pack. **DONE**.
2. Mark Pass 44 runtime rejected and make Pass 45 the active ledger. **DONE**.
3. Implement DX11/SM5 RHI-thread A/B launcher route; keep explicit `-norhithread` compatibility route.
4. Add lightweight frontend/gameplay performance-domain markers; no per-frame spam.
5. Audit and retire repeated landmark world-mutation loops; enforce one current owner per Museum/Culture/Silpo site.
6. Recalculate cull/render budget for 960×940 m sector.
7. Retire primitive tree visuals from normal gameplay and bind verified real foliage assets where available.
8. Rebuild tactical-map topology from authoritative central-Oster reference rather than procedural road blockout.
9. Run all 11 weapon material/texture dependency checks and close what existing content can support.
10. Forward-port/retire stale verifiers.
11. Full source CI.
12. Merge only after source checks are green.
13. Local test order after pull: **frontend performance first**, then gameplay; stop immediately on unsafe thermals or renewed catastrophic FPS collapse.

## 8. Acceptance gates

- Frontend: no historical RenderTargetPool crash and **>=30 FPS minimum**.
- Gameplay: bots off, no progressive collapse, **>=30 FPS minimum**.
- Graphics: no hidden resolution-scale downgrade below intended 100% clarity target.
- Trees: no visible primitive Cylinder/Sphere fantasy forest.
- Tactical map: recognizable compact central-Oster topology, no giant synthetic X/diagonal roads, player marker visible.
- Landmarks: Museum and Culture House visibly separate; Silpo stale/duplicate presentation removed; one owner per site.
- Weapons: no white/default rack slot accepted; material/texture truth for all 11 classes or explicit named content gap.
- CI: green source checks required but never sufficient for runtime verification.

**Current overall status: PASS 45 ACTIVE / IMPLEMENTATION IN PROGRESS.**
