# OSTER CONFLICT — RUNTIME PLAYTEST AUDIT — 2026-08-22

## Authority

This document records the user runtime playtest supplied on 2026-08-22. The screenshots and observed behavior override any prior code-only claim that an item was fixed. A source change is not `VERIFIED RUNTIME` until a new UE 5.8 playtest confirms it.

Reference baseline supplied by the user: `88fbebdbb9bd2178cdfe8195539d9b2db1082c20`.
Branch used for this correction pass: `fix/runtime-playtest-2026-08-22`.

## P0 — deployment and world entry

### DEPLOY-START-001 — deployment START transition
Observed: pressing `СТАРТ` in the team/group/role/spawn deployment panel causes a short freeze and visible panel shift before gameplay.
Required:
- pressing `СТАРТ` must not expose an intermediate shifted deployment layout;
- show a blocking loading presentation immediately;
- show visible progress `0%` through `100%`;
- only remove the loading presentation after the player pawn is possessed and deployment UI is closed;
- enter gameplay with weapon already available.
Current source action: dedicated deployment loading overlay/subsystem added; `UICommitDeployment()` now routes through it.
Status: `CODED_UNTESTED`.

### SPAWN-MUSEUM-001 — test spawn near museum
Observed: player still spawns far away in a mostly empty field.
Required:
- base test spawn is outside, but immediately near, the canonical museum site;
- spawn must ground-snap to a walkable surface;
- existing test weapon rack must be reachable from the primary base spawn without crossing the map.
Current source action: Team 1/2 base spawns moved to offsets around `AOCWorldSectorOster::MuseumAnchor()`; the existing 11-class test rack remains attached to the primary base spawn.
Status: `CODED_UNTESTED`.

### HUD-MINIMAP-001 — always-on minimap
Observed: HUD has no minimap; full tactical map exists on `M`.
Required:
- compact HUD minimap during normal gameplay;
- same world projection/source as Tactical Map 2.0, not a separate invented geography;
- player position and heading marker;
- hidden when frontend, deployment, settings/admin/chat or full `M` map is open.
Current source action: `OCMinimapSubsystem` added and reuses `OCTacticalMapSubsystem` render target/projection.
Status: `CODED_UNTESTED`.

## P0 — landmark placement and map truth

### LANDMARK-SEPARATION-001 — Museum / Silpo / Culture House overlap
Observed: Museum, Silpo and Culture House runtime models are still effectively in one location.
Required:
- Museum uses canonical Museum anchor;
- Silpo uses canonical Silpo anchor;
- Culture House uses canonical Culture House anchor;
- no legacy/photo-model duplicate or generic building geometry may occupy those exclusion areas;
- one landmark/site = one placement owner.
Existing canonical source anchors are retained. They must not be moved to cosmetically hide the runtime defect.
Status: `IN_PROGRESS`, requires runtime owner/transform acceptance after current separation/startup code.

### STADIUM-GEO-001 — stadium real-world registration
Observed: stadium and adjacent geometry are still in the wrong location and read as blockout.
Required:
- use canonical Stadion Oster geo reference;
- orientation/scale must be checked against the real site, not invented from available space;
- remove blockout apron/duplicate owner geometry;
- adjacent paths/roads must line up with the georeferenced site.
Status: `IN_PROGRESS`; detailed stadium reconstruction is intentionally isolated for a dedicated branch/chat, but canonical placement remains mandatory.

### SILPO-DETAIL-001 / MUSEUM-DETAIL-001 / CULTURE-DETAIL-001
Observed: current landmark meshes are not final-detail quality. Silpo especially remains blockout-like.
Required: photo-reference reconstruction and real-world placement in dedicated landmark branches, then merge without changing canonical anchors.
Status: `IN_PROGRESS`.

## P0 — vehicle assets and interaction

### PICKUP-M2-001 — mounted Browning and proxy cleanup
Observed: pickup has oversized primitive/disc/bar geometry and no convincing Browning M2.
Required:
- production M2 Browning mesh when the canonical asset exists;
- never display primitive cylinder/cube turret geometry as if it were the Browning;
- fallback may be used only if it is an actual mounted-gun mesh and must be logged as fallback;
- missing production asset must be logged as a content error, not silently disguised.
Current source action: primitive turret presentation is always hidden; production M2 and real R13 machinegun fallback are attempted; absence of both produces a hard runtime content log.
Status: `CODED_UNTESTED`; exact production M2 asset availability still requires UE/content verification.

### PICKUP-GUNNER-001 — understandable gunner entry
Observed: user cannot determine how to enter the mounted gun; previous logic filled driver first and required a driver before gunner operation.
Required:
- approach armed vehicle from rear/turret side + `E` enters gunner seat;
- approach front/driver side + `E` enters driver when empty;
- solo test gunner can aim/fire without a driver occupying the vehicle;
- prompt must reflect the seat that will be entered.
Current source action: armed vehicle rear-side entry now selects gunner directly; `CanGunnerOperateServer()` no longer requires a driver.
Status: `CODED_UNTESTED`.

### VEH-REVERSE-STEER-001 — steering while reversing
Observed: pickup and other vehicles reverse almost straight; left/right authority is extremely weak.
Root cause in source: steering torque was multiplied by `abs(ForwardSpeed)/800`, which approaches zero exactly when low-speed reverse steering is needed.
Required:
- meaningful steering authority from near-zero speed under throttle;
- stronger minimum steering authority in reverse;
- retain speed-based interpolation at higher speeds;
- preserve reverse steering direction.
Current source action: minimum low-speed authority added (higher in reverse) plus reverse torque boost.
Status: `CODED_UNTESTED`.

### BTR-PRODUCTION-001
Observed: production BTR model is still not visible/loaded in the playtest.
Required: canonical BTR-4 production mesh, grounded scale/pivot/wheels/camera, no mannequin or box proxy presented as final.
Status: `ASSET/IMPORT ACCEPTANCE BLOCKED` until UE import/runtime confirms the canonical asset is actually present and loading.

## P1 — weapons and combat visuals

### WEAPON-MUZZLE-001 — shot visual must originate at muzzle
Observed: shot effect appears offset from the firearm instead of from the muzzle.
Required:
- ballistic damage trace may remain camera/aim driven for aiming correctness;
- visible muzzle flash/tracer must originate from the weapon muzzle/forward weapon geometry;
- visual effect must never begin from an unrelated camera-side point.
Status: `IN_PROGRESS`; round-projectile presentation is corrected in this pass, exact visual-start rebase still requires the weapon multicast path change and UE acceptance.

### WEAPON-TRACER-001 — no yellow round bullet balls
Observed: bullet visualization reads as a round yellow object.
Required: short, thin directional tracer/muzzle flash; no visible sphere projectile for hitscan small arms.
Current source action: tracer radius constrained to a thin streak; muzzle uses directional cone/cylinder presentation rather than sphere.
Status: `CODED_UNTESTED`.

### PISTOL-ASSET-001
Observed: expected pistol production model is not loaded/visible as intended.
Required: verify canonical pistol asset import/path and remove any primitive fallback from final presentation.
Status: `IN_PROGRESS / CONTENT CHECK`.

## P1 — environment rendering

### GRASS-001 — visible continuous ground cover
Observed: grass coverage is effectively absent/sparse in the runtime screenshots.
Required:
- real grass mesh only;
- continuous-looking coverage on valid soil/yard/park surfaces;
- no grass on roads, sidewalks, buildings or hard surfaces;
- HISM/culling retained for performance.
Current source action: sampling grid tightened from 13.5 m to 10 m, clumps increased to 3–5, cull coverage increased; missing real grass assets now log as a hard content error.
Status: `CODED_UNTESTED`.

### DISTANT-FLICKER-001 — distant buildings/objects shimmer
Observed: distant houses/objects visibly flicker in the marked horizon area.
Likely classes to verify: overlapping/duplicate geometry, z-fighting, late rebuild owners, LOD/material instability.
Acceptance:
- no duplicate coplanar owners in city/blockout layers;
- no landmark late rebuilds producing overlapping geometry;
- stable distant silhouettes when camera is stationary and moving.
Status: `IN_PROGRESS`; runtime evidence is still authoritative.

### CITY-HOUSES-001 — generic identical huts
Observed: many houses are repeated identical hut-like blockouts and do not resemble actual Oster housing.
Required: broader real-house variation, photo/geography-informed placement, remove repeated placeholder silhouette as final visual owner.
Status: `IN_PROGRESS`; detailed city architecture remains a separate content pass.

### ROADS-SIDEWALKS-001 — excessive convexity
Observed: road and sidewalk surfaces appear too raised/convex.
Required: lower profile, consistent curb/road relationship, no inflated strips, preserve collision drivability.
Status: `IN_PROGRESS`.

### LARGE-BUILDING-STAIRS-001
Observed: marked large building has visibly crooked stairs and insufficient detail.
Required: rebuild stairs to consistent rise/run, align with entrance, increase facade/entry detail.
Status: `IN_PROGRESS`.

## Runtime acceptance order

1. Main menu → deployment panel.
2. Press deployment `СТАРТ`; loading overlay must show 0–100 without panel jump.
3. Player appears outside near Museum, already armed, test weapon rack nearby.
4. Confirm HUD minimap and `M` full tactical map coexist without input conflict.
5. Walk/drive to Museum, Silpo, Culture House and Stadium; verify they are spatially separate and georeferenced.
6. Pickup: front-side driver entry; rear-side gunner entry; solo gunner operation; no primitive fake turret.
7. Reverse steering test at 0–20 km/h on pickup and another vehicle.
8. Fire rifle and pistol: tracer is thin, no yellow sphere; muzzle visual origin must be checked.
9. Check grass density, distant flicker, roads/sidewalks and generic-building duplication.
10. BTR/M2/pistol production asset paths must be verified in the actual UE content/runtime, not inferred from source strings.
