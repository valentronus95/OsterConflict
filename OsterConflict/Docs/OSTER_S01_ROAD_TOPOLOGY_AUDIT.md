# OSTER S01 — ROAD / PATH TOPOLOGY AUDIT

Status: ACTIVE
Sector: `S01_Krushelnytska_College_Park`

## Scope

This audit classifies current road/path geometry against the S01 workflow/ownership rectangle. The rectangle is not a cadastral border. It exists only to prevent location-first work from silently taking ownership of geometry that belongs to adjacent Oster sectors.

Current bounds are derived from the canonical College and Central Park anchors plus the margins in `FOCLocationSectorPlan`.

Approximate current local bounds:

- X: `-68135 .. -18230 cm`
- Y: `1497 .. 53916 cm`

`VERIFY_R13_LOCATION_FIRST_S01_ROAD_TOPOLOGY.py` recomputes these values from current geo-reference data; the approximations above are explanatory, not verifier inputs.

## Classification rule

- `Inside`: the complete oriented rectangle is inside S01. It may be moved into S01-owned data one-for-one without changing neighboring sectors.
- `Crossing`: the geometry intersects S01 but extends outside it. It remains city/shared-owned until a later split creates explicit inside/outside pieces without changing the visible layout.
- `Outside`: no S01 ownership action.

The verifier uses oriented-rectangle vs sector-AABB SAT tests rather than classifying by the center point.

## Fully inside / migrated road

### S01_ROAD_COLLEGE_APPROACH

- Anchor: College
- Legacy expression: `College + FVector(-13500, 0, RoadZ)`
- Size: `FVector(30000, 660, 14)`
- Yaw: `0`
- Walks: both sides
- Confidence: C
- Relation: `Inside`
- Runtime owner: `FOCLocationSectorS01RoadData::OwnedInsideCorridors()`

The direct `BuildRoadNetwork()` call has been removed. Runtime resolves the College anchor and consumes the explicit record.

## Fully inside / migrated paths

The following five path rectangles are now explicit `FOCS01PathSeed` records and are rendered from `FOCLocationSectorS01RoadData`.

### Central Park

- `S01_PATH_PARK_EW` — offset `(0, 0)`, size `(17800, 360)`, yaw `0`
- `S01_PATH_PARK_NS` — offset `(0, -300)`, size `(360, 13200)`, yaw `0`
- `S01_PATH_PARK_DIAG_E` — offset `(1800, 900)`, size `(11800, 260)`, yaw `31`
- `S01_PATH_PARK_DIAG_W` — offset `(-2300, 1300)`, size `(9300, 240)`, yaw `-28`

Runtime owner: `FOCLocationSectorS01RoadData::OwnedCentralParkPaths()`.

### College

- `S01_PATH_COLLEGE_CAMPUS` — offset `(900, 5200)`, size `(8000, 5900)`, yaw `1`

Runtime owner: `FOCLocationSectorS01RoadData::OwnedCollegePaths()`.

All five remain confidence C. Moving them into explicit data changes ownership architecture, not real-world confidence.

## Shared crossing road corridors / audit-only

These records exist in `SharedCrossingCorridors()` but are deliberately not rendered from the S01 registry yet.

1. `S01_CROSS_WORLD_EW_02`
   - absolute center `(-18000, 17000)`
   - size `(61000, 820)`
   - yaw `0`
   - crosses S01 east side

2. `S01_CROSS_KRUSHELNYTSKA_SPINE`
   - absolute center `(-33500, 25000)`
   - size `(112000, 920)`
   - yaw `91.5`
   - long north/south crossing; cannot become S01-owned as one unsplit segment

3. `S01_CROSS_WORLD_DIAG_01`
   - absolute center `(-23500, 40500)`
   - size `(51000, 760)`
   - yaw `18`
   - shared diagonal corridor

4. `S01_CROSS_WORLD_NW_01`
   - absolute center `(-48000, 51000)`
   - size `(52000, 720)`
   - yaw `63`
   - one-sided walk configuration retained

5. `S01_CROSS_WORLD_DIAG_02`
   - absolute center `(-5000, 33500)`
   - size `(49000, 760)`
   - yaw `-34`
   - mostly outside S01; only intersecting portion belongs to future split work

6. `S01_CROSS_PARK_SOUTH`
   - anchor: Central Park
   - offset `(0, -8500)`
   - size `(43000, 720)`
   - yaw `2`
   - crosses the west workflow bound

7. `S01_CROSS_PARK_NORTH_LINK`
   - anchor: Central Park
   - offset `(-9000, 13500)`
   - size `(37000, 700)`
   - yaw `79`
   - one-sided walk configuration retained
   - crosses north/west workflow bounds

## Shared crossing derived path

The Central Park → CultureParkNorth path is built from both canonical anchors:

- center: `(CentralPark + CultureParkNorth) / 2`
- length: distance between both anchors
- yaw: direction from Central Park to CultureParkNorth
- width: `260 cm`

The geometric verifier classifies it as `Crossing`. It remains in `BuildCentralPark()` and is not duplicated in the S01-owned path registry.

## Existing S01 service/frontage paths

`S01_KR_SERVICE_W` and `S01_KR_SERVICE_E` already live in the original S01 data registry and are rendered by `BuildSolomiiKrushelnytskoiStreet()`. Frontage walk strips are likewise explicit through `FOCS01FrontageSeed`. They remain C-confidence retained blockout geometry.

## Current ownership count

- S01-owned road corridors: **1**
- S01-owned internal park/campus paths: **5**
- shared crossing road corridors: **7**
- shared crossing derived park path: **1**

## Gate for the next road step

No `Crossing` corridor/path may be moved wholesale into S01 runtime ownership.

The next legal operation on shared geometry is an exact no-visual-change split at sector ownership boundaries, followed by separate IDs for the S01-owned and neighboring/shared pieces. Reference-backed coordinate correction happens only after that ownership split is stable.
