# OSTER S01 — КРУШЕЛЬНИЦЬКА / КОЛЕДЖ / ЦЕНТРАЛЬНИЙ ПАРК

Status: ACTIVE RECONSTRUCTION SECTOR
Strategy: `OSTER_LOCATION_STRATEGY_R13.md`

## Purpose

S01 is the first location-first reconstruction sector and the template for later Oster sectors. Its bounds are a development ownership boundary, not an administrative or cadastral claim.

The sector is derived from canonical College and Central Park geo anchors through `FOCLocationSectorPlan`.

## Canonical anchors

Current project reference anchors used directly by S01:

- Oster College / Solomii Krushelnytskoi 7A — confidence A;
- Central City Park — confidence A;
- CultureParkNorth — confidence B for the north civic/park connection.

Explicit placement does not automatically mean factual placement. Migrated blockout geometry remains confidence C until a map/photo/reference supports promotion or correction.

## Structural ownership state

### Residential / frontage

Completed:

- procedural `OCR13ResidentialInfillSubsystem` is a migration stub and no longer invents houses from road geometry;
- generic `BuildResidentialBlocks()` is excluded from S01;
- 16 residential plot slots are explicit `FOCS01ResidentialPlotSeed` records;
- 8 frontage records are independently addressable;
- 2 Krushelnytska service-road records are explicit;
- `BuildSolomiiKrushelnytskoiStreet()` consumes the S01 registry instead of row/slot arithmetic.

All 26 retained residential/frontage/service records are still confidence C.

### Generic environment exclusion

Completed:

- generic `OCR13EnvironmentDressingSubsystem` does not generate S01 grass, plants, house extras, yard props, companion trees or stumps;
- generic city rough-grass/tree loops reject S01 points;
- obsolete near-spawn visual-slice content is cleanup-only and may not become an alternate topology owner.

### Vegetation ownership

Completed:

- 54 Central Park trees are explicit S01 records;
- 4 College trees are explicit S01 records;
- 2 mown-grass areas are explicit S01 records;
- `BuildVegetation()` consumes the explicit registry;
- the old park Row/Col/Jitter algorithm and direct College tree calls are removed;
- data/runtime verifiers prevent migration drift and procedural reintroduction.

All 60 vegetation placements are still confidence C retained blockout positions. Fidelity work remains.

### Road/path ownership

**Structural ownership migration is complete.**

- College approach is explicit;
- every audited `BuildRoadNetwork()` corridor that intersects S01 is either an explicit S01 piece or explicit adjacent/shared piece;
- all seven former crossing corridors have continuity-preserving split manifests;
- `SharedCrossingCorridors()` is empty;
- four Central Park internal paths and the College campus path are explicit;
- the former derived Central Park → CultureParkNorth sidewalk is split into `S01_PATH_PARK_NORTH_CIVIC_INSIDE` and `S01_PATH_PARK_NORTH_CIVIC_SHARED`;
- `BuildCentralPark()` consumes that split registry instead of recomputing `Mid`, `Delta` and `LinkYaw`;
- permanent road and dedicated Park→north-civic verifiers guard continuity, endpoints, profiles and ownership.

See `OSTER_S01_ROAD_TOPOLOGY_AUDIT.md`.

This closes the **ownership-plumbing** part of S01.1. It does not mean the physical street network is factually locked.

## Reconstruction stages

### S01.1 — topology ownership and reference correction

Ownership phase: **complete**.

Reference-correction phase: **active**.

Next evidence-driven targets:

1. Solomii Krushelnytskoi street alignment and practical carriageway width;
2. College approach/junction relationships;
3. Central Park secondary path geometry;
4. College campus secondary blocks;
5. residential frontage/driveway/fence alignment.

Gate: no coordinate changes merely for visual neatness. C-confidence geometry moves only when supported by evidence.

### S01.2 — plot registry

Status: **architectural migration complete; factual correction pending**.

Each of the 16 residential plots is independently addressable, so correcting one plot no longer shifts an entire procedural row.

### S01.3 — exterior architecture

For reference-backed plots, replace generic presentation with controlled Oster-compatible archetypes:

- brick one-storey;
- plaster one-storey;
- older timber/rural;
- larger post-Soviet private house;
- garage/outbuilding/shed;
- controlled roof, porch and window variants.

Architecture may change appearance but not secretly move plot ownership.

### S01.4 — yards and boundaries

Reference-backed authoring targets:

- tall wood fences;
- metal fences;
- light sheet/slate-like fences;
- gates and pedestrian entrances;
- driveways and worn surfaces;
- outbuildings.

No random yard generator is allowed inside S01.

### S01.5 — vegetation fidelity

Ownership phase: **complete**.

Next work is evidence-driven replacement/correction of C records, especially planted rows, park canopy spacing and residential trees. Whole-Oster art may render S01 vegetation but may not choose its placement.

### S01.6 — enterable buildings

Only after exterior topology for the chosen plot is stable:

- keep the current enterable-house gameplay path operational;
- align the shell to a stable S01 plot;
- then migrate doors, windows and interior layout.

Interior work must not modify surrounding street/plot topology.

### S01.7 — validation and lock

Required before `S01 LOCKED`:

- location-specific CI green;
- no generic residential/environment/vegetation ownership leakage;
- no duplicated landmark owner;
- reference-backed road/plot corrections completed to the chosen fidelity threshold;
- stable collision/access routes;
- packaged-build visual check;
- performance check after vegetation/art passes.

## Immediate next implementation

The next implementation pass is no longer another registry migration.

1. Audit real Krushelnytska alignment/width against public map/reference evidence.
2. Keep College and Central Park canonical anchors fixed unless stronger evidence changes the source reference itself.
3. Correct only supported S01 road/path segments and record why each correction is justified.
4. Then move to plot-frontage/fence corrections one addressable record at a time.
5. Bots/AI remain outside this phase.
