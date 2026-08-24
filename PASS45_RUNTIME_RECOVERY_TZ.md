# OSTER CONFLICT — PASS 45 RUNTIME RECOVERY TZ

Date: 2026-08-24
Status: **ACTIVE / IMPLEMENTATION STARTED**
Branch: `fix/runtime-recovery-pass-45-20260824`
Target: UE 5.8.x Windows
User launcher: `START_HERE.cmd`

## 0. Why this TZ exists

Pass 44 was implemented from the active requirements in `OSTER_CONFLICT_WORK_LEDGER.md` and root `AGENTS.md`; there was no separate complete Pass 44 TZ that combined runtime acceptance, visual fidelity, performance baseline and content truth in one contract. The first factual local run after merge disproved several Pass 44 assumptions.

Therefore this file is the canonical corrective TZ. It does not erase Pass 44 history. It records Pass 44 as **RUNTIME REJECTED** and converts the newest screenshots + source audit into explicit Pass 45 work and acceptance gates.

Latest runtime evidence pack:

![Pass 44 rejected runtime evidence](RUNTIME_EVIDENCE/2026-08-24_PASS44_REJECTED/pass44_runtime_evidence_20260824.jpg)

Manifest: `RUNTIME_EVIDENCE/2026-08-24_PASS44_REJECTED/MANIFEST.md`

---

# 1. Authoritative runtime verdict

The latest user run reached menu and gameplay, but the result is rejected.

Observed runtime facts:

- Main menu already shows about **8 FPS**.
- Gameplay is roughly **8–12 FPS**, with visible stutter/lag.
- Severe slowdown persists even in visually empty fields.
- Several rack weapons are **white / missing intended authored materials**.
- Held AK has a readable material while nearby pickup weapons do not, proving that successful mesh loading is not equivalent to material readiness.
- Tactical map is visually malformed and does not match the real compact central-Oster topology.
- Primitive/generic trees remain visible and do not match the required Oster vegetation character.
- Visible world quality is substantially worse than the intended target: flat green ground, primitive blocks, crude landmark shells and low-fidelity vegetation.
- Museum and Culture House still do not present as two clearly separated, correctly placed and visually identifiable landmarks.
- Silpo/legacy location presentation still contains stale visual identity and layered runtime construction.

**Pass 44 status: RUNTIME REJECTED.**

Green source CI from Pass 44 remains evidence that the source contract compiled/verified. It is not runtime acceptance.

---

# 2. Root-cause audit

## 2.1 Performance diagnosis: bot count was real but not the root cause

Pass 44 correctly removed implicit 16-player bot autofill from normal local gameplay. The latest screenshots prove this was insufficient because the frontend itself already runs around 8 FPS.

Therefore the root performance investigation must start before gameplay-only systems.

High-priority suspects requiring factual A/B measurement:

1. **Renderer/RHI compatibility launch path.** Current normal launch still forces `-d3d11 -sm5 -nohdr -norhithread`. `-norhithread` was retained as a crash-recovery compatibility measure and must not silently become the permanent performance baseline. Pass 45 must compare RHI-thread enabled vs compatibility mode without re-enabling D3D12/SM6.
2. **Editor runtime overhead.** Current playtest is `UnrealEditor.exe -game`, not a packaged Shipping/Development game executable. Editor-mode overhead must be distinguished from project GameThread/Draw/RHI/GPU cost.
3. **Accumulated delayed runtime mutation subsystems.** The project contains multiple post-BeginPlay scanners/rebuilders/recovery guards. Repeated world scans, instance removal, `MarkRenderStateDirty`, delayed landmark reconstruction and compatibility ownership checks can create frame spikes and long startup degradation.
4. **Overlarge cull distances relative to the new compact battlefield.** Several world families can remain visible across most or all of the 960×940 m sector.
5. **Primitive world authoring still creates large quantities of generic ISM geometry.** Compact bounds reduced extent, not necessarily frame cost per visible sector.

Pass 45 must collect frame-domain evidence, not only FPS:

- Game thread frame time;
- Draw thread frame time;
- GPU frame time;
- RHI thread / render submission behavior where available;
- actor/component/ISM counts at stabilized gameplay;
- delayed subsystem activity during the first 10 seconds.

## 2.2 Primitive world is still the visible production world

`OCWorldSectorOster` still uses engine BasicShapes for major semantic families:

- Cube: ground, roads, buildings, fences, grass and many landmark/park/stadium elements;
- Cylinder: tree trunks;
- Sphere: tree crowns;
- BasicShapeMaterial tinting for many world families.

This explains the visibly flat/blockout presentation. Pass 45 must stop treating these proxy families as acceptable production visuals.

## 2.3 Tree vegetation is wrong by construction

Current source retains generic `Tree*`, `SovietPoplar*`, `Birch*`, and `Pine*` families using primitive Cylinder/Sphere geometry. The latest runtime screenshot visibly exposes this proxy forest.

Required current vegetation identity:

- tall pine / conifer forest as the dominant museum/park woodland character where reference supports it;
- appropriate oak assets where broadleaf trees are required;
- no fantasy-like swollen trunks / spherical crowns;
- no primitive Cylinder/Sphere tree family visible in normal gameplay.

If a suitable real oak/pine asset is absent, it is a named content gap. Do not synthesize a “production” tree from BasicShapes.

## 2.4 Tactical map was bounded but not rebuilt from correct topology

Pass 44 correctly reduced projection bounds, but `BuildRoadNetwork()` still authors a hand-built procedural network of large straight/diagonal segments. The `M` map visualizes those source components as vector rectangles.

Result: the map is smaller, but the topology remains wrong.

Pass 45 requirement:

- tactical map road topology must originate from the authoritative compact central-Oster map/reference, not from the old procedural blockout;
- real landmark positions must project consistently through the same geo reference;
- no road/block visible on `M` may exist solely because an obsolete blockout component happens to be inside the new bounds;
- raw component extents may not define map truth.

## 2.5 Weapon mesh load is not material readiness

Pass 44 correctly stopped painting missing materials with a grey BasicShape fallback, but the latest rack proves several models still lack intended authored materials/textures.

Pass 45 requires a per-weapon dependency audit:

`weapon class -> exact mesh -> material slot(s) -> material asset(s) -> texture dependency/dependencies -> runtime visible result`

All 11 required pickup classes must have explicit status. A white/default slot is failure.

Do not claim M16/M4 connected without a verified real payload.

## 2.6 Museum / Culture House / Silpo ownership is over-layered

Source geo references for Museum and Culture House are distinct, yet runtime still fails to present two clearly separated landmarks. This indicates a runtime ownership/presentation problem, not merely one equal coordinate constant.

The project currently has many late landmark replacement/recovery/detail subsystems. Pass 45 must enforce one current placement/visibility owner per site.

Required site ownership:

- Museum: one authoritative shell/location owner;
- Culture House: one authoritative shell/location owner;
- Silpo: one authoritative shell/location owner;
- Stadium: dedicated Stadion Oster authority remains unchanged.

Historical compatibility subsystems may observe or become inert, but may not repeatedly mutate the same site after the authoritative owner has built it.

---

# 3. Pass 45 implementation work

## Phase A — recover a measurable performance baseline

### A1. RHI-thread A/B launch path

- Keep DirectX 11 + SM5 + no HDR for the first recovery pass.
- Normal game must no longer silently hard-code `-norhithread` as the only route.
- Provide an explicit compatibility/recovery launch route that retains `-norhithread` for crash comparison.
- Normal route should test DX11/SM5 with normal RHI threading after Pass 43 renderer-lifecycle fixes.
- D3D12/SM6 remains disabled until DX11 baseline is stable.
- Launcher/log must print exactly which renderer mode was used.

Acceptance:

- main menu remains open without the historical RenderTargetPool crash;
- compare frontend FPS/frame times between normal RHI-thread and compatibility `-norhithread` route;
- if RHI-thread route crashes, preserve crash evidence and revert only that route, not unrelated fixes.

### A2. Runtime performance instrumentation

Add a lightweight Pass 45 marker/report that samples stabilized frame domains instead of only instantaneous FPS.

Required markers should make it possible to distinguish:

- `PASS45_FRONTEND_PERF_BASELINE`
- `PASS45_GAMEPLAY_PERF_BASELINE`
- `PASS45_RHI_MODE`
- delayed mutation activity during first 10 s

No per-frame spam.

### A3. Stop repeated full-world repair loops

Audit all runtime subsystems that:

- iterate all actors/components repeatedly;
- remove ISM instances after BeginPlay;
- rebuild landmark shells after another subsystem already built them;
- call `MarkRenderStateDirty` repeatedly;
- poll hundreds of times for compatibility state.

For Museum/Culture/Silpo placement, convert repeated mutation loops to one-shot authority or inert compatibility observers.

Acceptance:

- no current landmark owner performs a 0.2 s × dozens-of-attempts world mutation loop after the site has stabilized;
- no duplicate shell is rebuilt later by a historical subsystem.

### A4. Compact-sector render budget

Recalculate cull distances for the 960×940 m battlefield.

Rules:

- small residential details/fences/grass: aggressively local;
- trees: visible at credible distance but not essentially across the full sector by default;
- landmark silhouette may use longer distance where needed;
- no shadow budget for decorative grass/proxy detail;
- culling must not create obvious pop-in at ordinary infantry sight lines.

---

## Phase B — remove visible primitive production proxies

### B1. Retire primitive tree generator from normal gameplay

- Primitive Cylinder/Sphere tree families must not render in normal game.
- Prefer existing real imported foliage assets after content inventory.
- Place tall pines/conifers according to reference character.
- Use oak only from a real suitable asset; otherwise keep it as a content gap.
- Do not restore birch/poplar proxy families merely to satisfy an old verifier.

Acceptance:

- screenshot from the former “Warcraft tree” area contains no primitive bulbous/spherical proxy trees.

### B2. World proxy truth

Inventory existing imported environment assets before adding any new primitive replacement.

Classify visible world families:

- production asset ready;
- acceptable temporary gameplay collision only and hidden visually;
- content gap;
- obsolete and removable.

Primitive collision helpers may remain only if invisible and required for gameplay.

---

## Phase C — tactical map topology correction

### C1. Real compact-Oster topology

Use `REFERENCE_PHOTOS/map_extent/oster_central_playable_area_20260824.jpg` and current geo anchors as the authoritative topology basis.

- replace hand-authored oversized straight/diagonal road blockout used by the tactical map;
- map must show the central street network in recognizably correct relative geometry;
- Museum / Culture House / Silpo / central park / Stadium anchors must appear in correct distinct positions;
- player marker must remain foreground-visible;
- projection remains north-up and bounded by compact playable area.

### C2. Tactical map acceptance screenshot

A new runtime screenshot of `M` is mandatory before acceptance.

Reject if:

- giant X-shaped/diagonal synthetic roads remain;
- POIs overlap because of wrong source ownership;
- map must zoom out to display obsolete procedural components;
- player marker is missing/hidden.

---

## Phase D — landmark ownership consolidation

### D1. Museum

- choose one current authoritative visible shell/model owner;
- retire or make inert historical runtime rebuilders that mutate the same shell;
- spawn approach must visually identify the Museum itself;
- no Culture House shell may appear at/inside Museum site ownership radius.

### D2. Culture House

- maintain its distinct geo anchor;
- visible model must build at that anchor only;
- no Museum-coordinate inheritance;
- no duplicate columned shell at Museum.

### D3. Silpo

- one authoritative site/model owner;
- late detail subsystems may decorate only the current model and must not rescan/rebuild the whole source site repeatedly;
- stale signage/legacy source geometry must not survive beside the authoritative Silpo result.

---

## Phase E — weapon material dependency closure

Create a table/report for all 11 required pickup weapons.

For each weapon verify:

- exact visual asset path;
- material slot count;
- every material path;
- texture dependencies load;
- no Engine BasicShape material;
- no null/default white slot;
- fresh-load UE preflight passes material truth, not only mesh load;
- runtime rack screenshot shows intended authored appearance.

Any gap remains explicit `CONTENT GAP` / `IN_PROGRESS`.

---

# 4. Stale behavior that must not return

Pass 45 explicitly forbids reintroducing:

1. implicit normal-game bot autofill;
2. 2.4 km procedural playable map;
3. map bounds derived from arbitrary component extents;
4. grey runtime BasicShape “repair” for weapon materials;
5. primitive Cylinder/Sphere trees as accepted normal-game vegetation;
6. repeated late landmark rebuilders competing for the same location;
7. source verifier expectations that force obsolete proxy behavior back into runtime;
8. declaring a mesh/material/location READY from source checks when latest runtime screenshot contradicts it;
9. hiding performance collapse by automatically lowering resolution scale below the intended 100% clarity baseline;
10. adding new decorative content before frontend/gameplay performance baseline is recovered.

---

# 5. Pass 45 acceptance gates

Pass 45 cannot be called verified until factual UE runtime satisfies all applicable gates.

## Gate 1 — frontend

- menu opens reliably;
- no RenderTargetPool startup crash;
- measured frontend performance is no longer ~8 FPS;
- target for acceptance: **>=30 FPS minimum**, with frame-domain evidence recorded.

## Gate 2 — gameplay performance

- enter normal local game with bots off;
- remain only as long as thermals are safe;
- no progressive collapse to 8–12 FPS;
- **>=30 FPS minimum** is required for current acceptance;
- no automated graphics downgrade below intended native 100% clarity target.

## Gate 3 — vegetation

- no primitive fantasy-like tree forest;
- tall pine/conifer character visible where expected;
- no production claim for missing oak/pine assets.

## Gate 4 — tactical map

- `M` visually matches compact central-Oster topology;
- synthetic X/giant straight procedural roads are gone;
- player marker visible;
- landmarks distinct.

## Gate 5 — landmarks

- Museum and Culture House are visibly separate and correctly located;
- Silpo stale/duplicate identity removed;
- one placement owner per landmark.

## Gate 6 — weapons

- all required rack weapons have authored material truth or an explicit named content gap;
- no white/default slot accepted;
- screenshot evidence required.

## Gate 7 — evidence and CI

- source CI must pass after stale verifiers are retired/forward-ported;
- CI alone cannot promote the pass to VERIFIED;
- latest user runtime screenshot/log overrides historical source assumptions.

---

# 6. Execution order

1. Lock this TZ + runtime evidence into the branch.
2. Update `OSTER_CONFLICT_WORK_LEDGER.md` to mark Pass 44 `RUNTIME REJECTED` and Pass 45 active.
3. Implement RHI-thread A/B route and performance markers.
4. Retire repeated landmark world-mutation loops / enforce single ownership.
5. Recalculate render/cull budgets for compact map.
6. Remove primitive tree families from normal visual runtime and bind real foliage assets where verified available.
7. Rebuild tactical-map topology from authoritative central-Oster reference rather than procedural road blockout.
8. Run per-weapon material dependency audit and close real asset/material gaps that can be closed from existing content.
9. Update/retire conflicting historical verifiers.
10. Run full source CI.
11. Merge only after source checks are green.
12. After local pull, perform **frontend performance test first**, then gameplay. Stop immediately if thermals/FPS become unsafe; that failure is sufficient evidence.

---

# 7. Current implementation status

- [x] Pass 44 latest runtime classified as **RUNTIME REJECTED**.
- [x] User screenshots archived as Pass 45 evidence sheet + manifest.
- [x] Pass 45 corrective TZ created.
- [ ] Ledger updated for Pass 45.
- [ ] RHI-thread A/B launcher implemented.
- [ ] Performance baseline instrumentation implemented.
- [ ] Repeated landmark mutation loops consolidated.
- [ ] Compact render budgets recalculated.
- [ ] Primitive tree production visuals retired/replaced.
- [ ] Tactical map rebuilt from authoritative topology.
- [ ] 11-weapon authored material dependency audit closed.
- [ ] Full source CI green.
- [ ] Local UE 5.8 runtime accepted.
