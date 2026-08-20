# R14.6 — LANDMARK SEPARATION / MAP CLEANUP REPORT

Date: 2026-08-21
Branch: `main`
Status: `CODED_UNTESTED`

## User regression report

Playtest showed Museum / Silpo / Culture House visual layers appearing as one mixed location. User explicitly required:
- Museum only on Museum parcel;
- Silpo only on Bohdana Khmelnytskoho 54;
- Culture House only on Hranovskoho 3;
- no building-in-building;
- map cleanup based on real geography rather than shared procedural anchors.

## Implemented

### Geo ownership
- Museum remains `FOCGeoReference::Museum()` = `50.948239, 30.883865`.
- Silpo remains `FOCGeoReference::Silpo()` = `50.948833799986254, 30.87572244094098`.
- Culture House now has dedicated `FOCGeoReference::CultureHouse()` = `50.948694, 30.881435`, address Hranovskoho 3, confidence B because exact parcel bearing is not survey-grade.
- Stadium remains separate at `50.949360, 30.884660`.

### Culture House current owner
Added:
- `OCR146CultureHousePhotoModelSubsystem.h/.cpp`
- authoritative tags `R146_CultureHouseAuthoritative` and `CultureHouseOster_Hranovskoho3`
- one site root with all shell/detail geometry local to it
- six-column facade, three entrance bays, pediment, stairs, conservative side/rear windows, eaves/downspouts
- old `R13_CultureHousePhotoModel` actor is destroyed if it somehow returns
- current owner uses a short startup handoff instead of a multi-second late replacement

### Anti-overlap / map cleanup
Added `UOCR146LandmarkSeparationSubsystem`:
- clears generic source building instances locally around Museum / Silpo / Culture House parcels before current owners reveal;
- removes legacy R13 Culture/Silpo actors if present;
- does not relocate current authoritative landmark actors;
- removes the old synthetic north-civic grove and its long straight park-link sidewalk, which were approximate procedural geometry rather than a verified mapped Culture House parcel/street.

### Regression guard
`VERIFY_R14_MAIN_LOCATION_OWNERSHIP.py` now rejects `main` if:
- Culture House owner is missing;
- Culture House geo point changes/disappears;
- Culture House references Museum/Silpo geo owners;
- separation guard disappears;
- legacy R13 Culture/Silpo owner files return.

GitHub Actions `Source verification` now runs this R14 ownership verifier on `main` pushes.

### Playtest launcher
`RUN_R14_MAIN_SANDBOX_TEST.cmd` now explicitly requires visual verification that:
- Museum, Silpo, Culture House are separate;
- no generic shell is nested inside them;
- old mixed civic/Silpo scene is absent;
- synthetic straight park-to-north-civic link is absent;
- Stadium remains separate.

## Safety / recovery
Pre-change backup branch:
`backup/main-before-landmark-separation-20260821-0033`

## Not yet VERIFIED

No UE 5.8 compiler/runtime is available in the GitHub connector session. Therefore these changes remain `CODED_UNTESTED` until the user's local UE 5.8 option-5 build/playtest succeeds and the map is visually checked.
