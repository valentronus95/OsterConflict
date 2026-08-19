# OSTER S01 — КРУШЕЛЬНИЦЬКА / КОЛЕДЖ / ЦЕНТРАЛЬНИЙ ПАРК

Status: ACTIVE RECONSTRUCTION SECTOR
Strategy: `OSTER_LOCATION_STRATEGY_R13.md`

## Purpose

S01 is the first location-first reconstruction sector. It is the proving ground for the workflow that will later be repeated across Oster.

The sector is a development ownership boundary, not an administrative or cadastral claim. Its runtime bounds are derived from the canonical college and central-park geo anchors through `FOCLocationSectorPlan`.

## Canonical anchors

The sector currently depends on these `FOCGeoReference` anchors:

- Oster College / Solomii Krushelnytskoi 7A — confidence A in the project reference model.
- Central City Park — confidence A in the project reference model.

All additional residential and migrated vegetation placements currently remain provisional unless individually backed by a reference. Provisional placement is confidence C and must never be silently promoted to A/B.

## Current technical state

Completed location-first protections and migrations:

- generic `OCR13ResidentialInfillSubsystem` no longer invents new houses beside roads;
- generic `BuildResidentialBlocks()` is excluded from S01;
- generic `OCR13EnvironmentDressingSubsystem` does not generate adaptive grass, plants, house extras, yard props, companion trees or stumps inside S01;
- obsolete near-spawn R12/R13 Krushelnytska visual-slice content is migration-only cleanup;
- stadium geography no longer depends on a late relocation pass and is owned by `FOCGeoReference` / `AOCWorldSectorOster`;
- the old arithmetic Krushelnytska house row has been migrated to 16 individually addressable `FOCS01ResidentialPlotSeed` records;
- the eight old repeated frontage slots are now 8 individually addressable `FOCS01FrontageSeed` records;
- the two old hard-coded side/service road strips are now explicit `FOCS01RoadSeed` records;
- all 26 migrated residential/frontage/service-road records remain confidence C because migration into explicit data does not make their old blockout coordinates factual;
- `BuildSolomiiKrushelnytskoiStreet()` consumes the explicit S01 registry rather than deriving house, frontage or service-road coordinates from slot arithmetic;
- city-wide rough-grass and generic-tree loops now reject placements whose generated point is inside S01;
- the retained central-park canopy has been frozen into 54 individually addressable `FOCS01TreeSeed` records;
- the four retained college trees have been frozen into 4 individually addressable `FOCS01TreeSeed` records;
- the retained central-park and college mown-grass areas are now 2 explicit `FOCS01GrassPatchSeed` records;
- all 60 vegetation records remain confidence C; explicit ownership is not evidence of real-world accuracy;
- `VERIFY_R13_LOCATION_FIRST_S01_VEGETATION_DATA.py` checks the vegetation registry independently in CI.

Still provisional or unresolved inside S01:

- exact residential house footprints and real plot boundaries;
- exact frontage fence/gate positions and fence families;
- several road widths, side-road alignments and sidewalk/path details;
- runtime `BuildVegetation()` still renders the intentional central-park canopy and college trees from the legacy loop/direct calls; the explicit registry is prepared but runtime consumption is the next migration step;
- park secondary geometry beyond directly supported reference cues;
- college campus secondary blocks beyond the strongly referenced main facade/site cues;
- the existing enterable-house anchor is still a gameplay/blockout placement rather than a verified real plot assignment.

## Reconstruction order

### S01.1 — topology lock

1. Audit the Krushelnytska road spine and approach roads.
2. Keep College and Central Park fixed to canonical geo anchors.
3. Express every S01 road/path as an explicit authored segment.
4. Label each segment A/B/C confidence.
5. Remove duplicate or overlapping source road strips.

Gate: no S01 road may be created by a city-wide procedural generator.

Current progress: residential frontage/service roads are explicit C-confidence records; the larger road network still needs reference-by-reference audit before topology lock.

### S01.2 — plot registry

The repeated arithmetic house loop has been replaced by an explicit plot registry.

Each plot records:

- stable plot ID;
- placement/orientation;
- approximate footprint;
- reference confidence;
- source/reference note;
- whether the primary house and outbuilding exist in the retained blockout;
- presentation variant needed to preserve current visuals during migration.

Gate: moving one house must not shift every later house in a loop.

Current progress: **architectural migration complete**. All 16 residential plot slots are individually addressable. Their coordinates remain provisional C until better references replace them.

### S01.3 — exterior architecture

For verified/referenced plots, replace generic house presentation with local archetypes while preserving explicit footprint ownership.

Archetype library:

- brick one-storey;
- plaster one-storey;
- older timber/rural;
- larger post-Soviet private house;
- garage/outbuilding/shed;
- controlled roof, porch and window variants.

Gate: architecture may change appearance but not secretly move a plot.

### S01.4 — yards and boundaries

Author plot boundaries explicitly:

- tall wood fences;
- metal fences;
- light sheet/slate-like fences;
- gates and pedestrian entrances;
- driveways and worn yard surfaces;
- outbuildings.

Gate: no random yard prop generator inside S01.

Current progress: frontage positions are independently addressable but still represent old C-confidence blockout strips, not final real plot boundaries.

### S01.5 — vegetation

Separate source-wide vegetation from S01-owned vegetation.

- park canopy and campus vegetation become explicit S01 data;
- residential trees are attached to plot/site data;
- USSR-era planted tree rows are explicit where supported;
- generic city vegetation may not leak into locked S01 bounds.

Gate: whole-Oster art may render S01 vegetation, but it may not decide its placement.

Current progress:

- generic rough-grass and generic tree points are rejected inside S01;
- 54 current central-park trees, 4 college trees and 2 mown-grass areas are frozen as 60 explicit anchor-relative C-confidence records;
- registry integrity is now checked in CI;
- runtime rendering still needs to switch from the legacy park loop/direct college calls to these records one-for-one.

### S01.6 — enterable buildings

Only after exterior topology is locked:

- keep the existing enterable-house gameplay path operational;
- align the selected enterable shell with a stable S01 plot;
- then migrate doors, windows and interior layout to that plot.

Gate: interior work never changes the street or neighboring plot topology.

### S01.7 — validation and lock

Required before `S01 LOCKED`:

- source verification green for location-related checks;
- no generic residential infill inside S01;
- no generic environment dressing inside S01;
- no generic source vegetation leaking into S01;
- no duplicated landmark ownership;
- stable collision/access routes;
- packaged-build visual check;
- performance check after final vegetation pass.

## Immediate next implementation

1. Make `BuildVegetation()` consume `ProvisionalVegetationTrees()` and `ProvisionalGrassPatches()` for S01-owned park/college vegetation.
2. Remove the legacy central-park row/column placement loop and the four direct college tree calls only after one-for-one registry consumption exists.
3. Add a runtime structural verifier that rejects reintroduction of those legacy S01 vegetation formulas.
4. Only then replace C-confidence vegetation positions with reference-backed positions where evidence exists.
