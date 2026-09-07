# LOC_SILPO_002 — CURRENT SOURCE GAP AUDIT — 2026-08-28

Parent contract: `_DOCS/REFERENCE_PACKS/LOC_SILPO_002_OSTER_SILPO/REFERENCE_SPEC.md`  
Pass45 binding: `PASS45_REFERENCE_PACK_BINDINGS.md`  
Runtime truth: **RUNTIME REJECTED 2026-08-27**  
Scope: source-to-reference reconciliation only; no runtime acceptance claimed.

## 1. Audit conclusion

Current Silpo source is substantially more than an empty box: it already encodes the canonical anchor, elongated one-storey mass, stepped parapet, far-left entrance, mostly blank advertising facade, parking apron and a sparse enterable interior. R14.3 also adds an orange/blue/white sign treatment.

However, the reviewed user reference pack is materially stricter. The current R14.0/R14.3 presentation is still **PROXY / TEMPORALLY WRONG / INCOMPLETE CONTEXT**, and several hero elements are built from visible Engine BasicShapes and `BasicShapeMaterial`, which Gate K explicitly forbids as accepted production art.

The correct path is to preserve useful anchor/layout semantics while replacing the player-facing proxy art and completing the missing site context.

## 2. What current source gets directionally right

Current `UOCR140SilpoPhotoModelSubsystem` already attempts:

- canonical Silpo geo anchor at Bohdana Khmelnytskoho 54;
- low elongated one-storey retail mass;
- mostly hidden low roof behind a parapet;
- stepped front silhouette;
- public entrance at the far-left portion of the long facade;
- largely windowless promo-board facade rather than a glass mall frontage;
- grey lower plinth;
- narrow planting strip;
- direct parking/asphalt apron;
- enterable public door;
- sparse shelf/cooler/checkout interior;
- fluorescent-style interior fixture rhythm.

Current `UOCR143SilpoFacadeIdentitySubsystem` additionally attempts:

- prominent orange/blue/white `Сільпо` identity;
- layered sign depth rather than one flat text label;
- dark parapet cap rails;
- parking sign.

These are useful semantic anchors. They are not final visual acceptance.

## 3. Silpo source gaps against the normative pack

### SILPO-GAP-01 — production baseline facade state is wrong

The normative pack selects the **2020 GRAPHITE STATE** as the Pass45 production baseline.

Current R14.0/R14.3 source still paints the shell/patch with warm beige/peach values:

- `R140Silpo_StuccoMat` ~ `(0.72, 0.57, 0.42)`;
- `R140Silpo_StuccoLightMat` ~ `(0.83, 0.69, 0.54)`;
- `R143Silpo_StuccoPatchMat` repeats the warm state.

This effectively retains the older 2017–2019 facade era while the new contract explicitly selects the later graphite state.

Required closure:

- one explicit selected temporal state in the production owner;
- 2020 graphite shell/facade baseline;
- older light facade may exist only as a separately named historical/alternate material variant;
- no simultaneous blending of old and new facade eras.

### SILPO-GAP-02 — hero shell is Engine Cube + BasicShapeMaterial

R14.0 directly loads:

- `/Engine/BasicShapes/Cube.Cube`;
- `/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial`.

The shell, parapet, entrance shell, foundation, roof, windows, sign back, ads, asphalt, interior shelves/checkouts/coolers and utilities are then assembled from this proxy family.

This is useful for layout but cannot satisfy Pass45 Gate K.

Required closure:

- authored Silpo shell/facade/entrance/parapet mesh hierarchy or accepted modular authored geometry;
- authored material chain with real texture dependencies;
- proxy cubes may remain only as invisible/debug/collision geometry where explicitly justified;
- no player-facing BasicShape shell in accepted screenshots.

### SILPO-GAP-03 — R14.3 logo still uses visible Engine Cylinder + TextRender

R14.3 improves readability but still builds its orange/blue oval from `/Engine/BasicShapes/Cylinder.Cylinder` and renders the brand lettering with `UTextRenderComponent`.

The reference requires a recognizable volumetric facade sign, not a production-visible primitive approximation.

Required closure:

- authored sign backing/outline/letter geometry or accepted sign mesh/material asset;
- correct orange/blue/white layering and physical depth;
- no visible Cylinder/BasicShape logo owner in Gate K acceptance;
- current R14.3 may remain a temporary source aid only until authored identity replaces it.

### SILPO-GAP-04 — stepped parapet shape is source-assumed rather than photo-locked

R14.0 labels its current front silhouette a **symmetrical stepped parapet** and hard-codes five tier blocks.

The user pack verifies a stepped parapet as a primary fingerprint, but the exact tier widths/heights must be matched to the factual photographs rather than preserved merely because current constants exist.

Required closure:

- compare current tier positions to `CAM-SILPO-01` / `CAM-SILPO-02` intent;
- correct asymmetry/width/height where photo evidence requires it;
- treat current hard-coded tier values as provisional until screenshot match.

### SILPO-GAP-05 — entrance is a porch approximation, not yet the full lower entrance wing

Current source cuts a small far-left opening and adds a compact canopy/threshold. This captures location but not the full evidence-driven lower entrance-volume identity.

Required closure:

- authored lower wing volume distinctly subordinate to the main parapet mass;
- white-framed glazed outward-opening door treatment;
- canopy/fascia and corrugated/profile-sheet character where visible;
- tiled raised platform/low steps;
- practical notices/stickers/utility lighting as detail layer;
- direct `CAM-SILPO-03_ENTRANCE_SIDE` acceptance.

### SILPO-GAP-06 — advertisement wall is generic colored geometry

Current five promo boards are generic blue/green/neutral boxes.

Required closure:

- preserve modular advertisement ownership;
- factual period-compatible visual treatment from the selected reference state;
- do not hard-bake unverified ad copy into structural geometry;
- retain the largely windowless long-wall rhythm;
- direct `CAM-SILPO-04_LONG_WALL` acceptance.

### SILPO-GAP-07 — checkout zone is materially under-modeled

Current source explicitly calls four cashier positions a practical gameplay blockout.

The user pack directly photographs a much richer checkout identity:

- several parallel lanes, with approximately 1–7 lane numbering visible across the evidence;
- orange square lane-number signs with blue numerals;
- monitors/scanners/keyboards/printers/chairs;
- anti-theft gates;
- carts with orange details;
- low suspended ceiling and fluorescent rhythm;
- light tiled floor.

Required closure:

- rebuild checkout zone from the photographed identity rather than four generic box counters;
- maintain interior confidence boundary: checkout high fidelity, unphotographed back rooms remain provisional;
- no invented exact full-store floor plan.

### SILPO-GAP-08 — opposite-side commercial/market context is missing

Current R14.0 source creates only the immediate Silpo apron. It does not model the opposite-side low-rise commercial context required by the pack.

Required closure:

- low-rise opposite block;
- red-brown two-storey orientation building with large arched upper opening/window;
- market/kiosk character where supported;
- utility poles/wires and pedestrian crossing relationship;
- no generic high-rise city wall;
- direct `CAM-SILPO-05_OPPOSITE_SIDE` and `CAM-SILPO-06_STREET_AXIS` acceptance.

### SILPO-GAP-09 — historic water tower is absent from the Silpo context contract

Current Silpo runtime owner does not provide the evidence-owned `LOC_TOWER_002A_OSTER_WATER_TOWER` landmark.

Required closure:

- separate authored tower asset/owner;
- aged dark red/brown brick;
- ring/cornice bands;
- arched upper openings and lower openings;
- telecom mast/antenna treatment;
- nearby low-house/fence/vegetation context;
- exact tower transform must stay `PROVISIONAL` until multi-view geographic closure;
- `CAM-SILPO-07_WATER_TOWER_SIGHTLINE` must prove plausible bearing/scale.

A generic water tank/tower family is not an acceptable substitute.

### SILPO-GAP-10 — asphalt/parking/ground reads as clean planar blockout

Current source uses a single flat Cube asphalt apron and thin parking-mark boxes.

Required closure:

- aged/imperfect asphalt material variation;
- real curb/sidewalk transitions;
- irregular practical parking rather than sterile architecture-visualization spacing;
- vegetation/plinth contact treatment;
- ordinary small-town utility clutter where reference-supported;
- no large exposed BasicShape slab.

### SILPO-GAP-11 — source cleanup radius can erase real surrounding context

`SuppressSourceSite()` removes source building/fence instances within a broad radius around the Silpo anchor. This is safer than overlap, but the new pack now contains factual opposite-side/nearby context.

Required closure:

- replace broad radial cleanup with explicit parcel/context ownership where practical;
- do not delete a real reference-supported adjacent object merely because it lies inside a generic cleanup circle;
- separate Silpo shell parcel from opposite-side commercial context and tower sightline owners.

### SILPO-GAP-12 — R14.0 and R14.3 are still a two-stage visible proxy composition

R14.0 owns the main shell and a rectangular placeholder sign. R14.3 then overlays a patch and a second sign identity actor to hide/replace that placeholder visually.

This can remain during transition but should not become final architecture.

Required closure:

- one authoritative production shell/facade owner;
- separate additive detail owner is acceptable only if it does not cover contradictory placeholder geometry;
- retire the R14.0 placeholder sign when the authored sign becomes authoritative;
- no layered proxy-over-proxy geometry in accepted screenshots.

## 4. Required execution slices

### SILPO-R1 — temporal state + ownership cleanup

- lock 2020 graphite production state;
- classify/delete historical warm facade from active baseline;
- remove contradictory placeholder sign ownership;
- split exact Silpo parcel, opposite-side context and water-tower context owners.

### SILPO-R2 — authored exterior hero model

- shell;
- stepped parapet;
- lower entrance wing;
- plinth;
- roof edge;
- doors/frames;
- volumetric authored sign;
- authored materials/textures;
- long advertisement wall.

### SILPO-R3 — photographed checkout interior

- ceiling/lighting rhythm;
- lane geometry;
- orange/blue lane-number signs;
- scanners/monitors/peripherals;
- anti-theft gates;
- carts;
- light tiled floor;
- keep unphotographed areas explicitly provisional.

### SILPO-R4 — street/opposite context + water tower

- aged street/parking/curb treatment;
- opposite low-rise commercial/market block;
- wires/poles/crossing;
- separate historic water tower asset;
- multi-view tower placement reconciliation.

### SILPO-R5 — direct runtime acceptance

Capture and compare all required Silpo cameras:

- `CAM-SILPO-01_FRONT_WIDE`;
- `CAM-SILPO-02_FRONT_CLOSE`;
- `CAM-SILPO-03_ENTRANCE_SIDE`;
- `CAM-SILPO-04_LONG_WALL`;
- `CAM-SILPO-05_OPPOSITE_SIDE`;
- `CAM-SILPO-06_STREET_AXIS`;
- `CAM-SILPO-07_WATER_TOWER_SIGHTLINE`;
- photographed checkout identity when interior is included in the tested build.

No source verifier may mark SILPO-R5 complete.

## 5. Current status

- normative Silpo user reference: **BOUND**;
- selected temporal state: **2020 GRAPHITE / CURRENT SOURCE MISMATCH**;
- current R14.0 shell/interior: **PARTIALLY REFERENCE-INFORMED PROXY / BASICSHAPE-DRIVEN**;
- current R14.3 logo identity: **READABLE PROXY / BASICSHAPE+TEXTRENDER / NOT GATE-K ACCEPTABLE**;
- photographed checkout: **SOURCE BLOCKOUT ONLY**;
- opposite-side urban context: **MISSING**;
- historic water tower: **MISSING / TRANSFORM PROVISIONAL**;
- final Gate K Silpo acceptance: **OPEN**.
