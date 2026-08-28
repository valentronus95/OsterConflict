# OSTER CONFLICT — PASS45 BLOCK EXECUTION PLAN

Date adopted: 2026-08-28
Branch: `fix/pass45-runtime-rejection-material-closure-20260826`
Parent specification: `PASS45_RUNTIME_RECOVERY_TZ.md`
Latest factual runtime verdict: **RUNTIME REJECTED 2026-08-27**

## 0. Purpose

This file is the mandatory execution order for `PASS45_RUNTIME_RECOVERY_TZ.md`.

The parent TZ remains the technical requirements authority. This execution plan prevents the work from being spread across unrelated systems at the same time.

## 1. Non-negotiable execution rule

Only **one content block may be ACTIVE at a time**.

A later block is LOCKED until the active block reaches **RUNTIME ACCEPTED / FROZEN**.

Allowed exception: a compile/import/CI defect may be fixed out of order only when it directly prevents verification of the current active block. That fix must be narrow and must not start work on another content domain.

Forbidden while a block is ACTIVE:

- opportunistic edits to another landmark;
- simultaneous weapon/vehicle/world art changes;
- broad refactors unrelated to the active block;
- marking source/CI green as runtime acceptance;
- replacing missing reference facts with guessed production geometry;
- carrying unfinished visual work forward merely to make another verifier green.

## 2. Status vocabulary

Each block may have only one of these states:

- `LOCKED` — not allowed to modify yet;
- `REFERENCE AUDIT` — collect/recover user photos, map/geo facts and existing project evidence;
- `ACTIVE` — production work is allowed only for this block;
- `SOURCE CODED` — implementation exists but has not passed local UE 5.8 visual acceptance;
- `RUNTIME REJECTED` — local UE evidence shows defects; remain in this block;
- `RUNTIME ACCEPTED` — required direct visual/gameplay evidence is accepted;
- `FROZEN` — accepted block is protected from unrelated later edits except explicit regression repair.

`SOURCE PASS`, `CI PASS`, `IMPORT PASS` and `RUNTIME ACCEPTED` are not interchangeable.

## 3. Per-block workflow

Every content block follows the same sequence. No skipping.

1. **Reference recovery**
   - recover all relevant user-supplied photos/screenshots/maps from ChatGPT Library and repository reference manifests;
   - identify which images are factual source references and which are generated concept art;
   - do not use generated concept art to overrule factual photographs.
2. **Geo/site lock**
   - confirm canonical map location, orientation, road/site relationships and scale;
   - record confidence for anything not directly visible in references.
3. **Single-owner audit**
   - identify exactly one player-facing runtime owner;
   - retire/hide conflicting old/proxy owners instead of letting them overlap.
4. **Primary production pass**
   - geometry / mesh hierarchy;
   - scale and orientation;
   - authored materials/textures;
   - collision/navigation where needed.
5. **Detail pass**
   - facade/trim/windows/doors/signs;
   - vegetation/ground/context props;
   - interiors only where required and actually referenced.
6. **Regression/source gate**
   - narrow verifier for the current block;
   - no stale historical magic values that contradict the current accepted owner.
7. **Local UE 5.8 acceptance**
   - import/fresh-load/build;
   - direct screenshots from required angles;
   - gameplay interaction where applicable;
   - compare against factual references.
8. **Freeze**
   - record accepted evidence/head;
   - freeze the block;
   - only then unlock the next block.

## 4. Mandatory block order

| Order | Block | State | Unlock condition |
|---|---|---|---|
| 0 | Ground + grass foundation | **ACTIVE** | Full playable territory visually covered and UE accepted |
| 1 | Oster Local History Museum / Solonyna House | LOCKED | Block 0 FROZEN |
| 2 | Oster Culture House | LOCKED | Block 1 FROZEN |
| 3 | Oster `Сільпо` + immediate commercial context | LOCKED | Block 2 FROZEN |
| 4 | Weapons, one weapon at a time | LOCKED | Block 3 FROZEN |
| 5 | Grenades / throwable presentation | LOCKED | Weapon block FROZEN |
| 6 | HMMWV + M2 | LOCKED | Grenade block FROZEN |
| 7 | BTR-4 | LOCKED | HMMWV block FROZEN |
| 8 | Remaining city sectors / landmarks | LOCKED | BTR block FROZEN |
| 9 | Tactical map + performance + thermal + final runtime acceptance | LOCKED | All visual/gameplay blocks FROZEN |

## BLOCK 0 — GROUND + GRASS FOUNDATION — ACTIVE

### Goal

The entire approved playable territory must stop reading as a prototype plane. Grass/ground coverage must be coherent before landmark polishing begins.

### Requirements

- cover the full playable land territory with a stable authored ground/grass presentation except roads, sidewalks, buildings, water and intentionally bare surfaces;
- no large naked BasicShape/default-material areas;
- avoid one uniform "golf course" material over the whole map;
- preserve maintained civic lawns, rough roadside/private-lot grass and future wetland/natural families as distinct visual zones;
- transition edges around roads, sidewalks, buildings and park paths must not visibly float or expose gaps;
- grass density/LOD/cull behavior must not collapse visibly at ordinary gameplay distances;
- vegetation must not create an unacceptable frame-time/tick cost;
- keep current playable bounds and navigation/collision semantics stable.

### Acceptance evidence

Direct UE 5.8 screenshots from at least:

1. museum/central sector ground context;
2. central park;
3. college/urban lawn context;
4. ordinary roadside/private-sector context;
5. a long sightline proving grass/ground LOD transition.

Block 0 cannot close from CI alone.

## BLOCK 1 — OSTER LOCAL HISTORY MUSEUM / SOLONYNA HOUSE

### Reference authority

Use factual user-supplied museum photos/maps first. Existing project notes identify the museum as the Solonyna House / Oster Local History Museum, `вул. Татарівська, 30`.

Recovered user constraints already known from prior reference work:

- central entrance must be recognisable;
- museum must be correctly tied to its real site rather than placed as a generic civic building;
- no invented `Остер`/museum facade sign where the factual photo does not show one;
- no birch substitution where the reference area does not contain birches;
- museum reads as being in a wooded setting;
- concrete/slab approach path and its real surrounding ground treatment must be respected;
- branch litter, pine needles/cones and sparse grass near the approach must follow the supplied photos rather than being scattered arbitrarily;
- preserve the Ukrainian flag only where the accepted target/reference requires it;
- Culture House six-column identity is forbidden on the Museum.

### Work order inside Museum block

1. recover and catalogue all factual museum photos from all sides plus site/context images;
2. lock canonical anchor, orientation, road/path relationship and footprint;
3. remove/hide overlapping legacy/source Museum geometry;
4. finish the full exterior silhouette;
5. finish brick/wood/roof material identity;
6. finish entrance, porch/steps, windows, frames, trims, annexes, roof/chimney details visible in references;
7. finish immediate fencing/path/trees/ground context;
8. add only referenced interior/access features required by the parent TZ;
9. direct screenshot comparison from front, both oblique sides and site-context angle;
10. freeze Museum only after runtime acceptance.

No Culture House, `Сільпо`, weapons or vehicles are to be developed while Museum is ACTIVE.

## BLOCK 2 — OSTER CULTURE HOUSE

### Goal

Build and accept the Culture House as a separate landmark at its real map site. It must never overlap or visually merge with the Museum.

### Required sequence

1. recover all factual Culture House photos/maps;
2. lock canonical geo anchor and facade orientation against roads/park/site context;
3. enforce one visible owner;
4. build full massing and roof from photos;
5. reproduce the distinctive six-column facade **only here**;
6. finish windows/doors/steps/materials/signage and immediate exterior context from references;
7. compare direct UE screenshots to the supplied photos;
8. freeze before touching `Сільпо`.

## BLOCK 3 — OSTER `СІЛЬПО` + IMMEDIATE COMMERCIAL CONTEXT

### Goal

Finish the real Oster `Сільпо` site, not merely a box with a sign.

### Required sequence

1. recover all factual user photos/maps of the store, surrounding buildings and interior references;
2. lock exact site location/orientation and relationship to the nearby road, park entrance/commercial node and adjacent structures;
3. build the store footprint/facade/roof/openings from factual photos;
4. reproduce the readable `Сільпо` identity/signage where factual references show it;
5. reproduce factual advertising/sign boards only from supplied/verified references;
6. finish parking/entrances/service passages/curbs/asphalt/nearby objects and adjoining commercial context;
7. if interior photos exist and the parent TZ requires enterability, complete the interior as a separate substage before closing this block;
8. direct front/oblique/site-context/interior UE screenshots;
9. freeze before weapon work starts.

Do not treat earlier generated battle-background images as geometry truth when they conflict with factual store photos.

## BLOCK 4 — WEAPONS — STRICT ONE-WEAPON SUBBLOCKS

Weapons are no longer edited as one giant family. Each weapon must be accepted and frozen before the next weapon is touched.

### Weapon order

1. AK-47 family;
2. MP5;
3. M1911;
4. M700;
5. Remington 870;
6. M249;
7. M14;
8. MAC-10;
9. TEC-9;
10. Lever Action .45-70;
11. anti-armor launcher.

Order may change only if a weapon is factually absent from production content. A content gap does not justify modifying several other weapons simultaneously.

### Mandatory checklist for every weapon

1. exact production mesh/model;
2. correct material/skin/texture dependencies;
3. correct first-person scale/orientation and attachment to hands;
4. correct third-person/pickup/drop visual;
5. muzzle location and projectile/trace/FX origin;
6. exact supported fire modes;
7. cadence/mechanical action;
8. recoil and recovery;
9. per-weapon ADS/sight calibration;
10. reload and manual-action animation;
11. shot/reload/mechanical/distant audio;
12. ammunition accounting and empty-state behavior;
13. drop physics;
14. direct hip/ADS/fire/reload/drop runtime evidence.

`bADSCalibrated=true` is forbidden until that exact weapon has factual local UE 5.8 alignment evidence.

## BLOCK 5 — GRENADES / THROWABLES

Only after firearm block completion:

- frag/smoke/flash visual identity;
- first-person hand/throw/recover animation;
- safe spawn and inventory commit semantics;
- real smoke VFX;
- audio/FX;
- near-wall and movement-inheritance tests;
- runtime freeze.

## BLOCK 6 — HMMWV + M2

Only after weapon/throwable completion:

- HMMWV production model/materials;
- forward orientation/scale;
- >=80 km/h gameplay target with stable handling;
- M2 authored mount/pivot;
- ring/shield/gunner hierarchy;
- continuous 360-degree yaw;
- elevation/muzzle/camera;
- enter/exit possession behavior;
- direct runtime acceptance.

## BLOCK 7 — BTR-4

- production material state before/after possession;
- canonical +X forward orientation and correct imported up-axis;
- scale;
- turret hierarchy;
- remote optic/monitor gameplay;
- camera/possession;
- direct runtime acceptance.

## BLOCK 8 — REMAINING CITY CONTENT

Only after the named high-priority blocks are frozen:

- park details;
- college;
- stadium;
- bus station;
- private-sector reference slices;
- fences/gates;
- remaining road/sidewalk/path art;
- additional landmarks and props.

Each landmark becomes its own sequential subblock using the same reference -> geo -> owner -> model -> detail -> runtime -> freeze workflow.

## BLOCK 9 — FINAL ACCEPTANCE

- tactical-map current topology screenshot;
- intended fullscreen mode;
- native render scale;
- stable ~60 FPS target under the accepted quality contract;
- 10-minute mixed thermal soak;
- current-head `START_HERE.cmd -> 2. ПОВНИЙ RUNTIME-ТЕСТ`;
- all strict evidence gates;
- direct screenshots;
- merge PR #94 only after factual current-head runtime acceptance.

## 5. Current execution state

Current active work is now **BLOCK 0 — GROUND + GRASS FOUNDATION**.

All Museum verifier cleanup, Culture House work, `Сільпо`, weapons, HMMWV and BTR work that does not directly block Block 0 is paused.

The previously observed stale `World geometry stability pass 12` Museum magic-vector expectation is recorded as backlog for the Museum block unless it directly blocks Block 0 validation. It must not drag execution back into Museum while ground/grass is ACTIVE.

## 6. Completion discipline

At the end of every block, record:

- accepted commit SHA;
- exact files changed;
- source verifier results;
- local UE 5.8 import/build result;
- direct runtime evidence paths;
- remaining known gaps = zero for the block, or the block stays open.

No percentage-based "mostly done" state unlocks the next block.