# OSTER S01 — ROAD / PATH TOPOLOGY AUDIT

Status: STRUCTURAL OWNERSHIP COMPLETE / REFERENCE CORRECTION ACTIVE
Sector: `S01_Krushelnytska_College_Park`

## Scope

This audit defines runtime ownership of road/path geometry that intersects the S01 development rectangle. The rectangle is a workflow boundary, not a cadastral or administrative border.

Its current bounds are derived from the canonical College and Central Park anchors plus `FOCLocationSectorPlan` margins. Approximate local bounds remain:

- X: `-68135 .. -18230 cm`
- Y: `1497 .. 53916 cm`

`VERIFY_R13_LOCATION_FIRST_S01_ROAD_TOPOLOGY.py` recomputes the bounds from the current georeference source instead of trusting these rounded documentation values.

## Ownership rule

- `Inside`: complete generated road/path envelope is inside S01.
- `Crossing`: geometry intersects S01 and an adjacent ownership area.
- `Outside`: no S01 ownership action.

Crossing geometry is never moved wholesale into S01. It is split into contiguous pieces that preserve the previous visible contour. Ownership cleanup therefore must not be confused with real-world accuracy.

## Road ownership state

The College approach is an explicit `Inside` record in `OwnedInsideCorridors()`.

All seven previously audited `BuildRoadNetwork()` crossings now have explicit continuity-preserving split manifests:

1. `KrushelnytskaSpineSegments()`
   - `S01_KR_SPINE_SOUTH_SHARED`
   - `S01_KR_SPINE_INSIDE`
   - `S01_KR_SPINE_NORTH_SHARED`
2. `EastWest02Segments()`
   - `S01_EW02_INSIDE`
   - `S01_EW02_EAST_SHARED`
3. `WorldDiag01Segments()`
4. `WorldNW01Segments()`
5. `WorldDiag02Segments()`
6. `ParkSouthSegments()`
7. `ParkNorthLinkSegments()`

The verifier checks original endpoints, total lengths, yaw, width, sidewalk profile, segment continuity and declared ownership relation. The one-sided sidewalk corridors use their real asymmetric generated envelope instead of being approximated as two-sided roads.

`SharedCrossingCorridors()` is intentionally empty. The old unsplit `S01_CROSS_*` audit records and direct runtime road calls are not allowed to return.

## Internal S01 paths

The following retained paths are explicit C-confidence `FOCS01PathSeed` records:

Central Park:

- `S01_PATH_PARK_EW`
- `S01_PATH_PARK_NS`
- `S01_PATH_PARK_DIAG_E`
- `S01_PATH_PARK_DIAG_W`

College:

- `S01_PATH_COLLEGE_CAMPUS`

Runtime consumes them through `OwnedCentralParkPaths()` and `OwnedCollegePaths()`.

## Central Park → north civic path

The former derived `CentralPark -> CultureParkNorth` sidewalk was the last path crossing still calculated directly inside `BuildCentralPark()`.

It is now represented by `ParkNorthCivicPathSegments()` as two contiguous absolute records:

- `S01_PATH_PARK_NORTH_CIVIC_INSIDE` — S01-owned portion;
- `S01_PATH_PARK_NORTH_CIVIC_SHARED` — shared north remainder.

The split preserves the former anchor-to-anchor contour, width `260 cm`, total length and yaw with no intended visual-layout change. `BuildCentralPark()` now renders the explicit records instead of calculating `Mid`, `Delta` and `LinkYaw` at runtime.

`VERIFY_R13_LOCATION_FIRST_S01_PARK_NORTH_PATH.py` independently reconstructs the original anchor-to-anchor vector, checks both endpoints, total length, yaw, continuity and `Inside/Crossing` classification, and rejects reintroduction of the old derived runtime call.

## Existing frontage/service geometry

`S01_KR_SERVICE_W` and `S01_KR_SERVICE_E` remain explicit S01 service-road records consumed by `BuildSolomiiKrushelnytskoiStreet()`. Frontage strips are explicit through `FOCS01FrontageSeed`.

These records remain confidence C retained blockout geometry.

## What is complete

The **ownership migration** part of S01.1 is structurally complete:

- no audited S01-intersecting `BuildRoadNetwork()` corridor remains as one unsplit crossing;
- the final Park → north-civic crossing path is explicitly split;
- internal park/campus paths are explicit records;
- old direct runtime construction is guarded by permanent verifiers;
- adjacent/shared pieces remain distinguishable from S01-owned pieces.

## What is not complete

This is **not** a declaration that the real Oster road topology is locked.

Most retained road/path geometry is still confidence C. The next work is evidence-driven correction, beginning with:

1. Solomii Krushelnytskoi street alignment and practical road width;
2. verified junction/approach relationships around the College;
3. Central Park secondary path geometry;
4. College campus secondary geometry;
5. residential frontage, driveway and fence alignment.

A C-confidence segment may move only when a map/photo/reference supports the correction. Registry cleanliness by itself is not evidence.
