# LOC_SILPO_002 — OSTER SILPO REFERENCE SPEC

Status: **NORMATIVE SUBORDINATE REFERENCE CONTRACT / SOURCE PREPARATION ONLY / UE 5.8 RUNTIME ACCEPTANCE REQUIRED**  
Parent TZ: `PASS45_RUNTIME_RECOVERY_TZ.md`  
Binding index: `PASS45_REFERENCE_PACK_BINDINGS.md`  
Location id: `LOC_SILPO_002_OSTER_SILPO`  
Water-tower sub-id: `LOC_TOWER_002A_OSTER_WATER_TOWER`  
Reviewed: 2026-08-28

## 0. Purpose

This file normalizes the user-supplied Silpo photo/description pack into one repository-controlled contract for Pass45 implementation and screenshot acceptance.

The original chat/library evidence is not a runtime dependency and must not be copied into packaged game content merely to satisfy a verifier. This repository spec carries the reviewed geometry/material/site rules; the source photographs remain evidence.

Authority for this location:

1. latest explicit user correction + latest factual UE 5.8 screenshot/log;
2. `PASS45_RUNTIME_RECOVERY_TZ.md`;
3. this spec;
4. current implementation;
5. older source verifiers/passes.

A green source verifier never upgrades the location to runtime accepted.

## 1. Reviewed evidence set

The user reference pack reviewed on 2026-08-28 contained:

- `LOC_SILPO_002__01_SITE_ORIENTATION_AND_URBAN_CONTEXT.md`;
- `LOC_SILPO_002__02_BUILDING_ARCHITECTURE_AND_INTERIOR.md`;
- `LOC_SILPO_002__03_WATER_TOWER_SURROUNDINGS_AND_UE5.md`;
- `SOURCE_PHOTO_MANIFEST.md`;
- `SCENE_ANCHORS.json`;
- 20 photographs covering the store facade, entrance, long wall, checkout zone, opposite-side street context and water-tower sightlines.

These filenames identify the reviewed source package only. No `/mnt/data` path is part of the persistent contract.

## 2. Canonical identity and anchor

`VERIFIED`:

- object: Silpo supermarket, Oster, Chernihiv region;
- address: **Oster, Bohdana Khmelnytskoho Street 54**;
- address-level anchor used by current project georeference: approximately `50.9488338 N, 30.8757224 E`;
- Silpo is a separate landmark owner and may not borrow Museum or Culture House geometry;
- the visible `Сільпо` identity belongs only to the canonical Silpo parcel.

Recommended local authoring frame:

- local origin: center of the main front facade below the logo;
- +X: along the facade;
- +Y: across the street;
- +Z: up.

Exact cadastral boundaries and a survey-grade footprint are not proven by the pack.

## 3. Temporal facade state

The reference set contains more than one factual facade era. **They must not be blended into one impossible building.**

`VERIFIED` temporal states:

- roughly 2017–2019: light beige / peach / pale-pink main facade;
- 2020-era/latest pack state: graphite/dark-grey main facade.

Pass45 baseline for current production modeling:

**2020 GRAPHITE STATE**.

The older light facade may exist only as an explicit alternate material/state variant. Geometry, signage and surface treatment from different eras may not be silently mixed.

Hard fail:

- graphite upper facade combined with arbitrary old-state paint patches as though simultaneous;
- material state selected without declaring its reference period.

## 4. Primary building silhouette

`VERIFIED`:

- low, elongated, utilitarian one-storey retail mass;
- roof is visually subordinate and mostly hidden by a **high stepped parapet**;
- stepped front/parapet profile is a primary recognition fingerprint;
- central/main facade carries a large volumetric Silpo logo;
- one side has a distinctly **lower entrance wing**;
- long side/front continuation is largely blank/windowless and carries advertisement panels;
- building must not read as a modern glass mall or a generic rectangular warehouse.

`PROBABLE`:

- plan is broadly elongated rectangular;
- primary roof is low-slope behind the parapet.

`UNKNOWN / PROVISIONAL`:

- exact surveyed dimensions;
- hidden rear architecture;
- exact roof drainage/HVAC layout where not visible.

Greybox-only dimensional envelope, not survey data:

- visible facade order of magnitude: ~20–30 m;
- main parapet height order of magnitude: ~5–7 m;
- entrance wing height order of magnitude: ~3–4 m.

These values are for first-pass proportion only and must yield to photo matching.

## 5. Silpo logo and facade identity

The logo is a hero feature.

Required:

- orange volumetric oval/cloud-like backing;
- blue/white `Сільпо` lettering treatment matching the selected period;
- readable thickness/depth and shadowing;
- logo placed as real facade geometry/signage, not a flat generic text decal;
- scale and placement must match the stepped facade composition.

Hard fail:

- flat unlit decal pretending to be the hero sign;
- wrong brand colors;
- logo on Museum/Culture House parcels;
- logo floating free of the facade;
- generic Latin `SILPO` substitution in accepted screenshots unless a historical reference explicitly requires it.

## 6. Entrance wing

`VERIFIED`:

- entrance volume is lower than the main facade;
- simple rectangular utility architecture rather than a monumental centered lobby;
- small canopy/fascia zone;
- corrugated/profile-sheet character appears in the entrance treatment;
- white-framed glazed outward-opening doors;
- stickers/notices create lived commercial clutter;
- overhead box/roller/awning element and practical capsule/utility lighting;
- slightly raised tiled platform with roughly one or two low steps;
- blue bin is a useful period/context prop when compatible with the selected reference state.

Hard fail:

- entrance centered symmetrically as a mall portal;
- full-height curtain wall;
- grand stairs or decorative canopy not present in evidence.

## 7. Long facade / advertisement rhythm

`VERIFIED`:

- long wall is mostly windowless;
- modular advertisement/billboard rhythm is visually important;
- grey plinth/base strip;
- narrow vegetation strip near the wall;
- slight stains, wear and imperfect ageing are part of the reference character.

Implementation rule:

Advertisement content should be modular material/decal/signage content, not baked into structural wall geometry. This allows period-correct replacement without rebuilding the shell.

Do not invent panoramic retail windows on the long facade.

## 8. Roof

`PROBABLE`:

- low-slope practical roof behind the parapet.

Do not invent:

- dormers;
- decorative roof towers;
- dense visible HVAC forests;
- a pitched residential roof.

Any roof equipment not visible in reference remains `PROVISIONAL` and should be visually subordinate.

## 9. Interior checkout zone

The source pack contains direct checkout photographs. This zone has higher evidence confidence than the unphotographed store middle/back rooms.

`VERIFIED` checkout character:

- low suspended ceiling grid/tiles;
- bright linear fluorescent lighting;
- several parallel checkout lanes;
- practical grey checkout desks/counters;
- monitors, scanners, keyboards, printers and chairs;
- orange square lane-number signs with blue numerals; approximately lanes 1–7 are visible across the evidence set;
- anti-theft gates near circulation/exit;
- metal carts with orange handles/inserts;
- nearby shelf/merchandise zones;
- light tiled floor.

Fidelity priority:

- checkout zone: **HIGH**;
- central sales-floor population/shelf arrangement: **MODULAR / PROVISIONAL** where photos do not prove exact layout;
- back rooms/service rooms: **UNKNOWN** unless later evidence is supplied.

Hard fail:

- replacing the photographed period checkout arrangement with a modern self-checkout-only hall;
- omitting the fluorescent ceiling rhythm and orange lane-number identity;
- claiming an invented full floor plan as exact.

People/cashiers/customers are dynamic population and are not baked into the building mesh.

## 10. Immediate site / street edge

`VERIFIED`:

- narrow pedestrian sidewalk along the facade;
- vehicle parking directly in front of the store;
- aged/imperfect asphalt rather than a pristine new plaza;
- vegetation strips near the plinth;
- simple utility poles and overhead wires;
- nearby pedestrian crossing;
- low-rise urban/commercial context;
- street/market character is irregular and utilitarian rather than master-planned.

Parking must feel like an occupied small Ukrainian town commercial edge:

- irregular spacing is acceptable;
- period-appropriate ordinary/older vehicles are preferable to a uniform fleet of new cars;
- do not sterilize the forecourt into an empty architectural visualization.

## 11. Opposite-side urban context

`VERIFIED / HIGH-CONFIDENCE CONTEXT`:

- opposite side remains low-rise;
- mixed trade/market/parking character;
- notable red-brown two-storey commercial building with a large arched upper opening/window is a useful orientation landmark;
- low market pavilions/kiosks are present in the context, including some green-roofed elements in the reference era;
- no high-rise wall should dominate the opposite side.

This block is context, not license to invent exact hidden interiors.

## 12. Water tower — `LOC_TOWER_002A_OSTER_WATER_TOWER`

The water tower is a separate landmark asset associated with the Silpo urban-context pack.

`VERIFIED` silhouette/material character:

- tall, narrow cylindrical historic utility tower;
- dark red/brown aged brick;
- strong horizontal ring/cornice bands;
- arched upper openings;
- lower rectangular openings, with some appearing blocked/light-filled in the references;
- telecom mast/antenna equipment occupies the top;
- brick is irregular/weathered with staining/patina, not clean uniform new brick.

Greybox-only magnitude:

- height roughly 20–30 m;
- diameter roughly 5–8 m.

These are not survey dimensions.

Telecom equipment should be separately authored/modular where practical:

- mast;
- antenna/panel elements;
- dish where supported by the selected view;
- brackets/cables/guy-wire treatment as supported by evidence.

## 13. Water-tower surroundings

`VERIFIED / HIGH-CONFIDENCE CONTEXT`:

- low one-storey houses nearby;
- light plastered facades;
- simple gable/slate-like roofs;
- older ordinary window proportions;
- profiled-sheet/metal/wood fences;
- narrow sidewalks;
- mature trees/bushes;
- ordinary older cars, bicycles/mopeds may appear as dynamic/context props.

The tower is visible from several Silpo-area viewpoints but is **not proven to stand directly on the Silpo parcel**.

### Tower transform rule

Exact tower transform remains `PROVISIONAL` until resolved by a factual placement chain:

1. lock the verified Silpo anchor;
2. resolve tower map/satellite/GIS point;
3. match multiple photographed sightlines;
4. only then lock the final tower transform.

A guessed single-view transform may not be promoted to `VERIFIED`.

## 14. UE5 ownership / recommended decomposition

Recommended content/data-layer split:

- `DL_SILPO_BUILDING`;
- `DL_SILPO_INTERIOR`;
- `DL_SILPO_STREET_PROPS`;
- `DL_SILPO_CIVILIANS`;
- `DL_SILPO_WATER_TOWER`;
- `DL_SILPO_OPPOSITE_BLOCK`;
- `DL_SILPO_COMBAT_DAMAGE`.

The exact naming may adapt to project conventions, but ownership boundaries must remain equivalent.

Combat-damaged Silpo/tower presentation, if introduced, is a separate state. The clean/reference baseline may not be destructively baked into a combat-only mesh.

The water tower is **not declared destructible by this spec**. Destruction requires a separate gameplay/content contract.

## 15. Required screenshot set

Runtime photo-fidelity acceptance requires all of these views or direct equivalents with preserved intent:

- `CAM-SILPO-01_FRONT_WIDE` — full stepped facade, logo, entrance relation and parking context;
- `CAM-SILPO-02_FRONT_CLOSE` — facade/logo/material detail;
- `CAM-SILPO-03_ENTRANCE_SIDE` — lower entrance wing, door/canopy/platform treatment;
- `CAM-SILPO-04_LONG_WALL` — blank-wall/ad rhythm/plinth/vegetation;
- `CAM-SILPO-05_OPPOSITE_SIDE` — cross-street low-rise commercial/market context;
- `CAM-SILPO-06_STREET_AXIS` — Silpo embedded in the actual small-town street axis, wires/parking included;
- `CAM-SILPO-07_WATER_TOWER_SIGHTLINE` — Silpo-area composition proving plausible tower bearing/scale without pretending a guessed transform is exact.

At least one interior acceptance capture must additionally show the photographed checkout identity when the interior is in the tested build.

## 16. Hard acceptance failures

Silpo Gate K is hard FAIL if any of the following is visible in an acceptance capture:

- building reads as a generic modern mall or warehouse;
- stepped parapet identity is lost;
- logo is flat, missing, floating or materially wrong;
- entrance is wrongly centered/monumental;
- long facade is replaced by generic panoramic glazing;
- 2017–2019 and 2020 facade states are mixed without an explicit variant;
- parking/street edge is sterilized into an empty pristine plaza;
- opposite side becomes high-rise/generic city blocks;
- water tower is omitted from a required sightline or replaced by a generic tank;
- tower brick is clean/uniform and loses its ageing/ring/opening identity;
- exact tower transform is claimed despite remaining unverified;
- interior checkout loses its fluorescent/parallel-lane/orange-number identity;
- Museum or Culture House geometry/signage contaminates the Silpo owner;
- production BasicShape/proxy/default materials remain visible as accepted hero content.

## 17. Evidence confidence summary

- Silpo address/identity: `VERIFIED`;
- stepped facade + logo + entrance relation: `VERY HIGH`;
- selected facade material state: `EXACT TO CHOSEN PERIOD`;
- long wall/ad rhythm: `HIGH`;
- checkout identity: `HIGH`;
- immediate street/parking character: `HIGH`;
- opposite-side block character: `HIGH`;
- water-tower silhouette/material: `VERY HIGH`;
- water-tower exact world transform: `PROVISIONAL`;
- hidden rear/roof service detail: `PROVISIONAL`;
- exact store floor plan/back rooms: `UNKNOWN / PROVISIONAL`.

## 18. Pass45 implementation consequence

Binding this spec does **not** mark Silpo complete.

It changes the standard of proof:

- source work must target this reviewed identity rather than a generic branded shell;
- Gate E must preserve one canonical Silpo owner and separation from Museum/Culture House;
- Gate K must compare the live UE result against the required camera set and selected temporal facade state;
- tower placement stays fail-visible/provisional until multi-view geographic evidence closes it;
- final acceptance still requires current-head local UE 5.8 screenshots.
