# LOC_CULTURE_003 — CURRENT SOURCE GAP AUDIT — 2026-08-28

Parent contract: `_DOCS/REFERENCE_PACKS/LOC_CULTURE_003_OSTER_CULTURE_HOUSE/REFERENCE_SPEC.md`  
Pass45 binding: `PASS45_REFERENCE_PACK_BINDINGS.md`  
Runtime truth: **RUNTIME REJECTED 2026-08-27**  
Scope: source-to-reference reconciliation only; no runtime acceptance claimed.

## 1. Audit conclusion

Current Culture House source is structurally cleaner than the current Silpo proxy: `UOCR146CultureHousePhotoModelSubsystem` already uses committed authored modular meshes, preserves their authored materials and explicitly refuses an Engine BasicShape fallback.

That is good source architecture. It is **not proof that the visible building matches the real Oster Culture House**.

The new reference contract confirms the identity/site/history but deliberately leaves exact facade geometry provisional. Therefore the present 32 m x 18.5 m hall, six-column layout, three entrance bays, roof tiling, window rhythm and `0°` facade bearing remain a **PROVISIONAL SOURCE HYPOTHESIS** until direct factual photos/map bearing and local UE screenshots accept them.

## 2. What current source gets directionally right

Current R14.6 source correctly establishes:

- separate canonical `FOCGeoReference::CultureHouse()` owner at Hranovskoho 3;
- authoritative Culture House actor tags separate from Museum and Silpo;
- explicit comment that the exact facade bearing is provisional rather than invented;
- no Engine BasicShape structural fallback;
- committed authored modular wall/window/door/roof/foundation/porch/frame assets;
- authored material preservation instead of repainting with `BasicShapeMaterial`;
- fail-closed behavior if required authored modular assets are unavailable;
- a civic front organization with columns and multiple entrance bays;
- separate roof/foundation/forecourt ownership.

These are strong implementation foundations and should be retained while visual evidence is tightened.

## 3. Culture House source gaps against the conservative reference contract

### CULTURE-GAP-01 — current six-column facade is not externally photo-verified

Source currently hard-codes six columns:

`{-1130, -680, -230, 230, 680, 1130}`

The existing landmark verifier uses this count as an ownership signature so the Culture House cannot contaminate the Museum. That is useful as a regression guard.

It is **not** enough to call six columns an exact factual architectural feature.

Required closure:

- retain six-column count only as a provisional current implementation until direct photos confirm/correct it;
- if later factual photos disagree, update geometry and verifier together;
- never protect an incorrect facade simply because an old verifier expects six columns.

### CULTURE-GAP-02 — 32 m x 18.5 m hall dimensions are source-greybox values

Current source comment declares a `32 m x 18.5 m hall` and fits modular authored meshes to that envelope.

The current evidence set does not prove survey-grade dimensions.

Required closure:

- classify current dimensions as provisional;
- derive scale from direct photographs/map footprint when available;
- keep canonical geo owner stable while allowing shell proportions to change;
- runtime comparison must judge recognizable proportions, not C++ constant preservation.

### CULTURE-GAP-03 — facade bearing remains unresolved

Source correctly states:

`Exact facade bearing is still provisional rather than invented.`

and currently uses `CultureHouseYawDegrees = 0.0f`.

Required closure:

- resolve facade bearing from road/park/map evidence;
- do not promote `0°` to factual truth merely because it is the current implementation;
- `CUL-CAM-05_SITE_CONTEXT` must prove parcel/site orientation.

### CULTURE-GAP-04 — generic rural-cabin modular kit may not match real civic proportions

The source uses `/Game/Modular_Rural_Cabin/Meshes/Modular/` wall/window/door/roof components. These are authored assets, which is preferable to Engine primitives, but they were not created specifically for the Oster Culture House.

Required closure:

- inspect whether current window/door/roof proportions actually match factual Culture House photos;
- replace or re-author hero facade pieces if the generic modular vocabulary becomes visually obvious;
- authored does not automatically mean location-faithful.

### CULTURE-GAP-05 — historical context is not yet reflected or explicitly ruled out at detail level

Public grounding identifies the current building as a former synagogue substantially adapted/rebuilt for Culture House use during the Soviet period.

This context must be handled conservatively.

Required closure:

- current-state civic model remains primary;
- do not invent pre-Soviet synagogue ornament, towers, domes or religious symbols without direct selected-period evidence;
- if factual current photos reveal surviving historic proportions/details, reproduce only those visible facts;
- keep historical reconstruction separate from current-game state.

### CULTURE-GAP-06 — old-park relationship is not yet a photo-locked site composition

Public grounding supports an old park immediately by the Culture House, but current source mainly builds the building/forecourt.

Required closure:

- mature park vegetation/context around the correct side(s) of the building;
- path/ground/edge relationship from direct evidence;
- maintain clear hero facade visibility;
- do not import Museum spruce-allée/stadium grounds into this parcel;
- do not guess monument transforms merely because a nearby park/monument exists in public descriptions.

### CULTURE-GAP-07 — exact facade materials remain unverified

Current implementation correctly preserves modular asset materials, but those material slots are not automatically factual Culture House materials.

Required closure:

- direct-photo material zones/colors/weathering;
- no Museum red-brick/timber palette shortcut;
- no Silpo graphite/orange retail palette bleed;
- authored texture dependencies required for final hero acceptance;
- accepted surface ageing should match factual selected-period photos.

### CULTURE-GAP-08 — roof/window/door rhythm is current-kit-driven

Current source derives facade openings from reusable modular assets and hard-coded placement arrays.

Required closure:

- direct reference count/spacing of windows and doors;
- exact entry steps/canopy/trim where visible;
- exact roof profile rather than generic tiled coverage if photographs disagree;
- both oblique cameras must prove side-depth and roof transition.

### CULTURE-GAP-09 — runtime separation remains unaccepted despite source ownership guards

Latest runtime history already proved that separate source owners do not guarantee visually separate landmarks if transforms/old actors/geometry overlap in gameplay.

Required closure:

- direct Culture House + Museum + Silpo site-context captures;
- zero shell intersection/fusion;
- no six-column Culture facade at Museum site;
- no Silpo signage/retail geometry at Culture House site;
- source validators remain non-mutating and cannot relocate buildings to create a pass.

## 4. Evidence still needed before exact hero modeling

Before promoting Culture House from provisional to photo-driven exact fidelity, collect/recover:

1. straight frontal facade photo;
2. front-left oblique;
3. front-right oblique;
4. one side/rear view if available;
5. wider site view showing road and old-park relation;
6. map/satellite/orthophoto bearing sufficient to resolve facade yaw;
7. close material/window/door/entrance details where possible.

Until then, reversible modular construction is preferable to hard-coding speculative decorative detail.

## 5. Required execution slices

### CULTURE-R1 — factual photo/map recovery

- recover direct Culture House photographs;
- resolve selected current/historical time state;
- lock facade bearing and visual envelope;
- update `REFERENCE_SPEC.md` confidence classifications.

### CULTURE-R2 — source hypothesis reconciliation

- compare six-column current source against factual facade;
- correct column/entrance/window count and spacing;
- correct shell proportions;
- correct roof form;
- retain canonical geo owner and authored-material fail-closed architecture.

### CULTURE-R3 — authored hero/detail pass

- location-faithful facade modules;
- doors/windows/frames;
- entrance/steps/trim;
- material chain;
- old-park/ground/site context;
- remove generic-kit visual fingerprints where they conflict with evidence.

### CULTURE-R4 — direct runtime acceptance

Capture and compare:

- `CUL-CAM-01_FRONT_WIDE`;
- `CUL-CAM-02_FRONT_CLOSE`;
- `CUL-CAM-03_OBLIQUE_LEFT`;
- `CUL-CAM-04_OBLIQUE_RIGHT`;
- `CUL-CAM-05_SITE_CONTEXT`.

No source verifier may mark CULTURE-R4 complete.

## 6. Current status

- identity/address Hranovskoho 3: **VERIFIED / BOUND**;
- former-synagogue/Soviet-adaptation historical context: **VERIFIED PUBLIC CONTEXT**;
- old park beside Culture House: **VERIFIED PUBLIC CONTEXT / EXACT LAYOUT OPEN**;
- current R14.6 authored no-BasicShape shell architecture: **GOOD SOURCE FOUNDATION**;
- current six-column facade: **PROVISIONAL WORKING HYPOTHESIS**;
- current 32 m x 18.5 m dimensions: **PROVISIONAL SOURCE GREYBOX**;
- current `0°` facade yaw: **PROVISIONAL**;
- exact facade material/roof/window/door fidelity: **OPEN**;
- final Gate K Culture House acceptance: **OPEN**.
