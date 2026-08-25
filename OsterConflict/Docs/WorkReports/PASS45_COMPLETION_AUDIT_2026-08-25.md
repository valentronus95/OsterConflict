# Oster Conflict — Pass 45 completion audit — 2026-08-25

Status: **SOURCE COMPLETION AUDIT / CODED_UNTESTED**

Canonical TZ: `PASS45_RUNTIME_RECOVERY_TZ.md`

This audit was opened because the user explicitly required a line-by-line check of whether Pass 45 was actually completed. The answer was **no**: Phase B2 still allowed several visible `OCWorldSectorOster` BasicShape families even though suitable imported content already existed.

## 1. What was still incomplete

`OCWorldSectorOster` still assigns `/Engine/BasicShapes/Cube` to ground, roads, sidewalks, residential buildings/roofs/details, landmark proxies, fences, park/stadium proxy families and reference/backstop geometry, then applies `BasicShapeMaterial` tinting in `BeginPlay()`.

Pass 45 had already retired primitive tree visuals, but that did not complete **B2 World proxy truth**.

## 2. Current B2 ownership matrix

| Source family | Pass 45 completion state | Current visual truth |
|---|---|---|
| `Ground` | **CODED_UNTESTED** | Cube remains only as the compact physical carrier; visual material is replaced after actor `BeginPlay` with imported `AdvancedVillagePack/M_Inst_Landscape`. |
| `Roads` | **CODED_UNTESTED** | Existing semantic road slabs remain geometry/collision; `BasicShapeMaterial` visual is replaced with imported `MI_Urb_Roa_Asphalt_01`. |
| `Sidewalks` | **CODED_UNTESTED** | Existing semantic slabs remain geometry/collision; visual is replaced with imported `MI_Urb_Roa_Sidewalk_01`. |
| `Buildings` | **CODED_UNTESTED** | Old cubes remain hidden collision/backstop. Visible residential owner uses imported `SM_House_Var01` / `SM_House_Var02`, preserving authored mesh proportions rather than independently stretching X/Y/Z. |
| `ResidentialRoofs` / `ResidentialDetails` | **CODED_UNTESTED** | Hidden with the residential cube visuals after full house conversion; old collision/backstop may remain. The imported house mesh owns the visible roof/detail silhouette. |
| `Fences` | **CODED_UNTESTED** | Public/source fence boxes remain hidden collision. Visible segments use imported `SM_Fence_Var04`. |
| `WoodFences` | **CODED_UNTESTED** | Hidden collision/backstop; visible segments use `SM_Fence_Var01`. |
| `MetalFences` | **CODED_UNTESTED** | Hidden collision/backstop; visible segments use `SM_Fence_Var02`. |
| `LightSheetFences` | **CODED_UNTESTED** | Hidden collision/backstop; visible segments use `SM_Fence_Var03`. |
| primitive tree trunk/crown families | **CODED_UNTESTED** | Already retired by Pass 45 foliage guard; real pine owner exists. Oak remains an explicit content gap. |
| `StadiumGeometry` / `StadiumDetails` | **DEDICATED OWNER / CODED_UNTESTED** | Source proxy art is already hidden; dedicated Stadion Oster runtime presentation remains the player-facing owner. |
| Museum source `Landmark*` instances | **DEDICATED OWNER / CODED_UNTESTED** | Museum shell authority is R13.8; legacy/generic geometry around the site is reconciled/hidden by current ownership rules. |
| Silpo | **DEDICATED OWNER / CODED_UNTESTED** | R14.0 is the single shell owner. |
| Culture House | **DEDICATED OWNER / CODED_UNTESTED** | R14.6 is the single shell owner. |
| College / other generic `LandmarkBlocks`, roofs, windows, details | **CONTENT GAP / IN_PROGRESS ART** | No photo-faithful production College/civic mesh was verified in the current imported inventory. Do not relabel a random house/building asset as the College. Existing proxy remains explicitly non-production. |
| `ParkGeometry` / `ParkDetails` | **CONTENT GAP / IN_PROGRESS ART** | No verified complete park/plaza/bench/skate production set matching the current source semantics was identified. Real foliage is separate. Do not declare cube benches/plaza details production-ready. |
| `Waterways` / `Bridges` | **NO CURRENT INSTANCES** | `BuildHydrography()` is retired for the compact battlefield. Imported `SM_Bridge_Var01..04` exist but there is currently no user-approved central-area bridge/water instance to replace. |
| `ReferenceMarkers` / text labels | **DEVELOPER-ONLY** | Explicitly hidden from gameplay. |

## 3. New current visual owner

`UOCWorldProductionVisualsSubsystem` is the single generic environment visual-conversion owner for B2.

It is deliberately bounded:

- waits 0.05 s so `OCWorldSectorOster::BeginPlay()` cannot reapply the old R11 BasicShape tint after production materials;
- only retries while the sector actor itself is unavailable, maximum 20 attempts;
- no full-world repeated scan;
- after success: `polling_after_ready=0`;
- real visual ISMs have collision/navigation disabled;
- house cull budget: 300–650 m;
- fence cull budget: 60–280 m;
- hidden source boxes retain collision/backstop ownership instead of duplicating physical collision on production art;
- each fence family is converted/fail-reported independently;
- missing imported assets cause explicit `PASS45_B2_PRODUCTION_VISUALS_FAIL`, never silent production READY.

## 4. Verified imported dependencies used by B2

- `/Game/AdvancedVillagePack/Meshes/SM_House_Var01`
- `/Game/AdvancedVillagePack/Meshes/SM_House_Var02`
- `/Game/AdvancedVillagePack/Meshes/SM_Fence_Var01`
- `/Game/AdvancedVillagePack/Meshes/SM_Fence_Var02`
- `/Game/AdvancedVillagePack/Meshes/SM_Fence_Var03`
- `/Game/AdvancedVillagePack/Meshes/SM_Fence_Var04`
- `/Game/AdvancedVillagePack/Materials/M_Inst_Landscape`
- `/Game/Scene_RoadsideConstruction/Materials/MaterialInstances/MI_Urb_Roa_Asphalt_01`
- `/Game/Scene_RoadsideConstruction/Materials/MaterialInstances/MI_Urb_Roa_Sidewalk_01`

These paths exist in the repository/LFS tree. Actual UE fresh-load/render correctness remains runtime evidence, not a source claim.

## 5. What still cannot be declared completed remotely

The following are **not source omissions** and cannot be honestly promoted without the next local UE 5.8 run:

1. frontend >=30 FPS and no RenderTargetPool crash on the normal RHI-thread route;
2. gameplay >=30 FPS without progressive collapse;
3. visual confirmation that primitive trees are gone and real pines read correctly;
4. tactical `M` screenshot acceptance;
5. visible Museum / Culture House / Silpo separation;
6. the fresh 11-weapon material/texture dependency report and actual closure of any binary-content gaps it reports;
7. visual confirmation that the new residential/fence/ground/road B2 owner improves the world without placement/stretching defects.

Those remain `CODED_UNTESTED` / `CONTENT GAP` until factual runtime evidence exists.
