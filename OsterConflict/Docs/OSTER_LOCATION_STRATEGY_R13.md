# OSTER CONFLICT — LOCATION-FIRST STRATEGY (R13)

Status: ACTIVE
Branch: `r13-content-gameplay-pass`
Primary objective: reconstruct the Oster location coherently before returning to bots/AI expansion.

## 1. Scope lock

Until the location milestone is complete, work is limited to:

- verified geography and coordinate anchors;
- road/street topology;
- landmark placement and silhouettes;
- residential blocks and yards;
- fences, utility infrastructure and street furniture;
- vegetation and ground surfaces;
- enterable-building shells and interiors after exterior topology stabilizes;
- lighting, ambience and optimization after the physical location is stable.

Deferred:

- bot behavior and bot mobility improvements;
- new weapons and vehicle variants;
- combat balancing;
- non-critical menu/UI redesign.

Existing gameplay fixes remain preserved, but they are not the active development track.

## 2. Core rule: one authoritative location model

`FOCGeoReference` and `AOCWorldSectorOster` are the authoritative geographic layer.

Every permanent location object must belong to one of these confidence classes:

- **A** — verified coordinates plus strong visual/documentary reference;
- **B** — reliable public placement, but incomplete architectural/site detail;
- **C** — gameplay-authored approximation used only where reliable reference is unavailable.

No new landmark, street segment or residential cluster may receive invented coordinates while an exact reference is still missing.

## 3. Stop runtime patch stacking

R13 currently contains many independent world subsystems that execute delayed art/repair passes. This was useful for rapid prototyping but is no longer acceptable as the permanent map architecture.

From this point:

1. Do not create another location subsystem merely to repair the output of an older one.
2. New permanent placement data goes into the authoritative location layer.
3. Existing repair/art subsystems are treated as migration code and are retired progressively.
4. Timer ordering must not be used as a dependency system for permanent map construction.
5. Procedural residential infill is disabled during the verified reconstruction pass. Buildings are added from explicit block/street plans instead.

## 4. Location build order

### L0 — Stabilize source geography

- keep museum as WGS84 local origin;
- verify all existing A/B anchors;
- keep stadium relocation tied to a verified map anchor;
- remove obsolete R12 fake near-spawn/Krushelnytska presentation data;
- establish explicit sector boundaries and street corridors.

Acceptance: one coordinate system, no duplicate versions of the same landmark, no fake near-spawn slice.

### L1 — Street skeleton

Build the road network before houses:

- primary roads;
- secondary streets;
- narrow pedestrian/service paths;
- intersections;
- sidewalks only where reference supports them;
- bridge/water relationships;
- utility corridors.

Acceptance: the player can navigate Oster using street geometry and major landmarks without relying on decorative filler.

### L2 — Landmark blockout

Complete exterior massing and correct placement for known reference locations:

- Oster local history museum / Solonyna house;
- Oster college and adjacent park;
- central city park;
- stadium;
- other reference-backed civic locations already present in `FOCGeoReference`.

Additional locations such as supermarket/bus-station sectors are added only after their source placement is verified.

Acceptance: landmark position, orientation, footprint and approach routes are stable before detail work.

### L3 — Residential Oster

Replace generic procedural housing with explicit residential clusters per street/block.

Create a controlled kit of characteristic local house archetypes instead of one repeated village asset:

- brick single-storey house;
- plastered single-storey house;
- timber/older rural house;
- larger post-Soviet private house;
- outbuildings/garage/shed;
- several roof and porch variants.

Each residential cluster defines:

- house footprint and orientation;
- setback from street;
- yard depth;
- gate/driveway;
- fence family;
- outbuildings;
- vegetation density.

Acceptance: no automatic "put a house beside a long road" generation in the verified play area.

### L4 — Yards, fences and vegetation

Add environment only after structures are fixed:

- tall wooden fences;
- metal fences;
- light sheet/slate-like fences;
- gates and pedestrian entrances;
- fruit trees, pines and characteristic planted trees;
- grass, worn ground, needles/cones in reference-backed areas;
- poles, wires and restrained roadside furniture.

Acceptance: dressing cannot alter road topology or building positions.

### L5 — Enterable buildings

Select a limited number of tactically useful buildings first.

For each:

- exterior shell remains authoritative;
- doors/windows align with the exterior;
- interior floor plan is built after shell lock;
- furniture is modular and inexpensive/ordinary rather than duplicated showroom sets;
- collision, doors, lights and navigation are validated separately.

Acceptance: interiors never force exterior buildings to move.

### L6 — Atmosphere

Only after geometry is stable:

- daylight/sky calibration;
- local ambient sound zones;
- vegetation motion;
- smoke/fire/battle dressing as scenario layers, not baked geography;
- menu cinematic viewpoints reference the same actual map locations.

### L7 — Performance and packaging

- HISM/ISM consolidation by asset/material family;
- cull distances and LOD/Nanite review;
- collision simplification;
- foliage density budgets;
- navigation rebuild;
- packaged-build visual regression pass.

## 5. Sector workflow

Each sector is completed through the same sequence:

1. references/coordinates;
2. roads and terrain;
3. landmark/residential footprints;
4. exterior architecture;
5. fences/yards;
6. vegetation/props;
7. enterable interiors where selected;
8. collision/navigation validation;
9. performance validation;
10. lock sector and move to the next.

Do not detail five sectors simultaneously.

## 6. Immediate R13 work queue

1. Disable procedural residential infill that invents houses from road length.
2. Audit `AOCWorldSectorOster::BuildRoadNetwork()` and existing hard-coded street geometry against the geo reference layer.
3. Convert the current Krushelnytska/college/park area into the first locked sector.
4. Move its permanent road/building/fence placement into explicit authored data rather than timer-based repair passes.
5. Retire obsolete Krushelnytska visual-slice migration code after the replacement sector passes validation.
6. Repeat the same workflow for the museum/Tatarivska sector, then center/stadium.

## 7. Definition of progress

Location progress is measured by locked sectors, not by number of new subsystems or commits.

A sector counts as complete only when geography, exterior structures, access routes, collision and basic environmental dressing agree with the available references and survive a packaged build.
