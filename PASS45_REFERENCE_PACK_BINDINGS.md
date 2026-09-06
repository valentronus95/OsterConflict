# PASS45 — REFERENCE PACK BINDINGS

Status: canonical binding index for `PASS45_RUNTIME_RECOVERY_TZ.md`  
Active branch: `fix/pass45-runtime-rejection-material-closure-20260826`

## 0. Purpose

`PASS45_RUNTIME_RECOVERY_TZ.md` remains the single canonical active recovery TZ and owns execution order, status and merge/runtime gates.

Detailed visual evidence must not be copied wholesale into that file. Location-specific reference packs are bound here as normative subordinate contracts for the relevant Pass45 acceptance gates.

Authority remains:

1. latest explicit user correction + latest factual local UE 5.8 screenshot/log;
2. canonical Pass45 TZ;
3. bound normative reference pack for the exact location;
4. current implementation;
5. historical source verifiers / older passes.

A newer factual runtime screenshot can reject a location even when its source verifier and reference-pack structural checks are green.

## 1. Active bindings

### `LOC_MUSEUM_001_OSTER_MUSEUM`

Normative repository appendix:

`_DOCS/REFERENCE_PACKS/LOC_MUSEUM_001_OSTER_MUSEUM/REFERENCE_SPEC.md`

Applies to:

- Pass45 Gate E landmark/environment acceptance;
- Pass45 Gate K visual-fidelity acceptance;
- Museum/Culture House/Silpo separation;
- Museum hero massing/material/detail work;
- museum front approach / spruce allée / lawn and ground treatment;
- museum-adjacent football-field relation;
- direct Museum screenshot-match acceptance.

Required screenshot set defined by the appendix:

`MUS-CAM-01..07`

Current status:

**REFERENCE BOUND / SOURCE AUDIT REQUIRED / UE 5.8 RUNTIME ACCEPTANCE REQUIRED**

### `LOC_SILPO_002_OSTER_SILPO`

Normative repository appendix:

`_DOCS/REFERENCE_PACKS/LOC_SILPO_002_OSTER_SILPO/REFERENCE_SPEC.md`

Evidence basis: reviewed user pack containing 3 detailed description files, source-photo manifest, scene anchors and 20 facade/interior/street/water-tower photographs.

Applies to:

- Pass45 Gate E Silpo ownership/separation and branded-site identity;
- Pass45 Gate K Silpo photo-fidelity acceptance;
- selected-period facade state and explicit prohibition on mixing 2017–2019 light facade with the 2020 graphite state;
- stepped parapet, volumetric `Сільпо` sign, lower entrance wing and long advertisement wall;
- checkout-zone identity and interior acceptance where interior is in scope;
- parking/street/opposite-block context;
- `LOC_TOWER_002A_OSTER_WATER_TOWER` silhouette/material/sightline contract;
- conservative fail-visible treatment of the still-provisional exact water-tower world transform.

Required screenshot set defined by the appendix:

`CAM-SILPO-01_FRONT_WIDE`  
`CAM-SILPO-02_FRONT_CLOSE`  
`CAM-SILPO-03_ENTRANCE_SIDE`  
`CAM-SILPO-04_LONG_WALL`  
`CAM-SILPO-05_OPPOSITE_SIDE`  
`CAM-SILPO-06_STREET_AXIS`  
`CAM-SILPO-07_WATER_TOWER_SIGHTLINE`

Current status:

**REFERENCE BOUND / NORMATIVE USER EVIDENCE / UE 5.8 RUNTIME ACCEPTANCE REQUIRED**

### `LOC_CULTURE_003_OSTER_CULTURE_HOUSE`

Repository appendix:

`_DOCS/REFERENCE_PACKS/LOC_CULTURE_003_OSTER_CULTURE_HOUSE/REFERENCE_SPEC.md`

Evidence basis: verified address/identity and public historical/site grounding plus current project source inspection. No dedicated user photo pack has yet been promoted for this location.

Applies to:

- Pass45 Gate E strict Culture House separation from Museum/Silpo;
- Pass45 Gate K direct Culture House screenshot acceptance;
- verified `Hranovskoho 3` identity/site context;
- former-synagogue / Soviet Culture House historical context without speculative pre-Soviet reconstruction;
- old-park relationship;
- current six-column source facade as a **PROVISIONAL WORKING HYPOTHESIS**, not photo-verified exact geometry;
- fail-visible handling of unverified bearing, dimensions, exact facade materials and hidden/rear geometry.

Required provisional screenshot set:

`CUL-CAM-01_FRONT_WIDE`  
`CUL-CAM-02_FRONT_CLOSE`  
`CUL-CAM-03_OBLIQUE_LEFT`  
`CUL-CAM-04_OBLIQUE_RIGHT`  
`CUL-CAM-05_SITE_CONTEXT`

Current status:

**REFERENCE BOUND / VERIFIED IDENTITY+SITE / VISUAL GEOMETRY PARTLY PROVISIONAL / UE 5.8 RUNTIME ACCEPTANCE REQUIRED**

A later dedicated user Culture House photo pack outranks and replaces conflicting provisional geometry assumptions in this appendix.

## 2. Evidence classification rule

Each location pack must distinguish:

- `VERIFIED` — directly supported by repeated visual/factual evidence;
- `PROBABLE` — strongly implied but incomplete;
- `UNKNOWN` / `PROVISIONAL` — insufficient evidence; conservative placeholder permitted but may not be called exact.

Generic asset-pack assumptions cannot upgrade `UNKNOWN` to `VERIFIED`.

A current C++ constant or green structural verifier cannot by itself upgrade a visual hypothesis to `VERIFIED`.

## 3. Repository rule

Chat/session `/mnt/data/...` paths are evidence-working paths only and are non-portable.

Build scripts, C++ runtime code and persistent verifiers must reference repository-controlled manifests/specifications or actual project content, never ephemeral `/mnt/data` locations.

User/public reference photos are evidence and are not automatically distributable game content. Do not copy or package them into the game merely to make a verifier green.

## 4. Separation rule

Every distinct real landmark must have its own evidence namespace.

Museum evidence cannot define Culture House geometry. Culture House evidence cannot define Silpo. Silpo evidence cannot define Museum. Citywide house/road/fence archetype packs may guide background art, but may not overwrite a location-specific hero pack.

If two packs disagree about what appears to be the same runtime owner, resolve ownership explicitly before implementation rather than averaging the references into a fictional hybrid.

For Pass45 the Museum, Silpo and Culture House bindings above are all active simultaneously. Their evidence may share citywide context, but their hero-building geometry, material identity and screenshot acceptance remain independent.

## 5. Future bindings

Add future location packs here only after their evidence has been reviewed and their scope is explicit. Examples may include Central Park, street/road archetypes, and Oster private-sector house/fence archetypes.

A pack is not runtime accepted merely because it is listed here.
