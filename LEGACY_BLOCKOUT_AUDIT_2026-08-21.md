# OsterConflict legacy blockout source audit — 2026-08-21

## Scope

This is a **source-level** audit of legacy/procedural visual paths that can still affect the current `OsterConflict_Runtime` world after the 14-screen runtime regression report.

It is not a UE playtest and it does not mark any runtime defect fixed. `RUNTIME_AUDIT_2026-08-21.md` remains the runtime authority until a fresh UE 5.8 current-`main` playtest.

## Classification rule

Not every primitive is a bug.

Allowed:
- invisible collision;
- explicit diagnostic-only geometry;
- temporary source-only presentation that is never presented as production verified.

Open defect / migration target:
- visible cube/cylinder/sphere presentation standing in for a production weapon, vehicle, character or named landmark;
- fixed raw-world placement with no canonical geo/ownership contract;
- delayed legacy cleanup/replacement pass that can mutate a landmark after reveal;
- a second subsystem that behaves as another placement owner for an already-owned landmark.

## Confirmed active risks

### LB-01 — R13.7 museum is still a delayed procedural owner

`OCR137MuseumPhotoModelSubsystem` is active on `OsterConflict_Runtime` and waits **5.10 seconds** before `ReplaceMuseum()`.

Its `BuildMuseum()` explicitly loads `/Engine/BasicShapes/Cube.Cube` and builds substantial visible museum architecture from cube instances, while mixing in real roof/tree assets.

Risk:
- the museum can visibly change several seconds after gameplay starts;
- the subsystem is still effectively an architecture placement owner despite later R14 detail layers;
- a source-level photo approximation is being confused with a production landmark asset.

Required treatment:
- do not add another decorative owner;
- keep one authoritative museum owner;
- move final architecture toward authored/production meshes while preserving collision separately;
- until then, runtime status remains `IN_PROGRESS`.

### LB-02 — separate R13.7 museum site cleanup wakes at 4.95 seconds

`OCR137MuseumSiteReplacementSubsystem` runs a one-shot `PrepareMuseumSite()` after **4.95 seconds**, immediately before the R13.7 museum photo model at 5.10 seconds.

It hides/cleans multiple museum, landmark, fence and tree component families around `MuseumAnchor`.

Risk:
- post-reveal cleanup can cause visible rebuild/flicker;
- this is an additional delayed world mutation path independent of the current R14 exclusion guard.

Current mitigation:
- `OCR146LandmarkSeparationSubsystem` now guards Museum/Silpo/Culture through the startup window and keeps a late actor-spawn guard active.

Still open:
- the old 4.95s/5.10s architecture pipeline itself must be consolidated rather than permanently relying on cleanup races.

### LB-03 — recovered unfinished building uses raw fixed world coordinates

`OCRecoveredEnvironmentSubsystem` creates a visible unfinished-building shell at raw world position:

`(-69000, 64500, 0)`

`OCRecoveredBuildingDetailsSubsystem` independently wakes at 0.35s and adds floors, stairs, walls and windows around that same raw site.

The placement is not derived from `FOCGeoReference`/a canonical Oster landmark anchor in these source files.

Risk:
- an invented/legacy structure can remain in the world without a documented real-site owner;
- two recovered subsystems cooperate as a de facto placement owner pair outside the landmark ownership model.

Required treatment:
- keep this site `UNVERIFIED/PROVISIONAL`;
- either map it to an evidence-backed real location and one owner, or remove the visible placement;
- do not expand it decoratively before georeference evidence exists.

### LB-04 — weapon primitive fallbacks were visible production substitutes

Runtime evidence showed proxy presentation for M249, M1911 and MAC-10.

Current source mitigation:
- `OCRealWeaponFallbackSubsystem` hides old static primitive presentation when an exact production visual is absent and uses already-imported real R13 mesh fallbacks where available;
- the real fallback does **not** receive a production-verification tag and explicitly leaves verification open.

Known exact-asset gaps on current `main`:
- exact `/Game/Production/Weapons/M249/...` asset path is absent;
- exact `/Game/Production/Weapons/Remington870/...` asset path is absent.

This is a visual safety improvement, not production completion.

### LB-05 — BTR still has a visible source proxy path and exact production asset is absent

`OCBTR.cpp` constructs a source BTR from primitive body/turret/barrel/wheel components and hides that source model only if `/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus` loads.

The expected `Content/Production/Vehicles/BTR4` asset path is absent from current `main`.

Therefore the green BTR box/proxy seen at runtime is consistent with current source state and must remain `IN_PROGRESS`.

No code-only wording may call BTR production-ready until the intended asset exists, loads, and is confirmed in UE runtime.

### LB-06 — character source proxy is intentionally shown when production body fails

`OCCharacterVisualComponent` shows source-only proxy geometry when the production body cannot be resolved.

The intended QuantumCharacter body/arms are stored through Git LFS. A clone that has only pointer files can therefore fall into the proxy path.

Current launcher mitigation:
- `RUN_R14_MAIN_SANDBOX_TEST.cmd` now runs `git lfs pull` and blocks the LocationTest if critical `.uasset` files are missing/tiny pointer files.

This improves test integrity but does not prove character production presentation until UE runtime confirms it.

## Named-location ownership snapshot

### Museum

Authoritative target owner:
- museum photo/architecture pipeline at canonical `MuseumAnchor`.

Current conflict risk:
- multiple R13.7–R14.5 museum architecture/detail/replacement subsystems still exist;
- the 4.95s site replacement and 5.10s procedural architecture pass are especially important because they mutate the world late.

Guard:
- R14.6 landmark separation/exclusion guard.

Status: `IN_PROGRESS`.

### Silpo

Authoritative target owner:
- `OCR140SilpoPhotoModelSubsystem` plus permitted detail-only layers.

Guard:
- R14.6 landmark separation/exclusion guard removes foreign generic/legacy geometry in the Silpo protected zone and watches late legacy actors.

Status: `IN_PROGRESS` until UE playtest proves one-site separation and no late rebuild.

### Culture House

Authoritative target owner:
- `OCR146CultureHousePhotoModelSubsystem`.

Guard:
- R14.6 landmark separation/exclusion guard removes foreign generic/legacy geometry in the Culture House protected zone and watches late legacy actors.

Status: `IN_PROGRESS` until UE playtest proves one-site separation and no late rebuild.

## Other current source families requiring runtime inspection, not blind deletion

The current source tree still contains recovered/procedural environment, foliage, roadside-prop, stadium, museum-detail and world-sector systems. Their existence alone is not proof of a defect.

The next playtest/log pass must distinguish:
- visible source blockout that should be migrated/removed;
- real imported asset layers that are valid;
- collision-only geometry that must stay invisible;
- diagnostic geometry allowed only in explicit test mode.

Do not mass-delete these systems solely because they are old or named `R13`. That would replace one regression with a more fashionable regression.

## Source conclusions

1. The remaining blockout problem is real and broader than a single Museum/Silpo overlap.
2. The highest-confidence late-mutation risk is the museum 4.95s cleanup + 5.10s rebuild sequence.
3. The recovered unfinished-building site at `(-69000,64500)` is not evidence-backed in its current source implementation.
4. Weapon/character/BTR proxies have different causes and must not be treated as one generic fallback problem.
5. Current source corrections improve ownership/test integrity but **do not close runtime evidence**.
6. No decorative R15/R16 work is authorized while these runtime gates remain open.

## Required next proof

Fresh UE 5.8 current-`main` `LocationTest=1` build/playtest with hydrated LFS assets, followed by normal frontend → TEAM gameplay validation.

Only after that evidence may relevant ledger items move from `IN_PROGRESS`/`CODED_UNTESTED` to `VERIFIED`.
