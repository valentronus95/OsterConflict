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

## 2. Evidence classification rule

Each location pack must distinguish:

- `VERIFIED` — directly supported by repeated visual evidence;
- `PROBABLE` — strongly implied but incomplete;
- `UNKNOWN` / `PROVISIONAL` — insufficient evidence; conservative placeholder permitted but may not be called exact.

Generic asset-pack assumptions cannot upgrade `UNKNOWN` to `VERIFIED`.

## 3. Repository rule

Chat/session `/mnt/data/...` paths are evidence-working paths only and are non-portable.

Build scripts, C++ runtime code and persistent verifiers must reference repository-controlled manifests/specifications or actual project content, never ephemeral `/mnt/data` locations.

User/public reference photos are evidence and are not automatically distributable game content. Do not copy or package them into the game merely to make a verifier green.

## 4. Separation rule

Every distinct real landmark must have its own evidence namespace.

Museum evidence cannot define Culture House geometry. Culture House evidence cannot define Silpo. Silpo evidence cannot define Museum. Citywide house/road/fence archetype packs may guide background art, but may not overwrite a location-specific hero pack.

If two packs disagree about what appears to be the same runtime owner, resolve ownership explicitly before implementation rather than averaging the references into a fictional hybrid.

## 5. Future bindings

Add future location packs here only after their evidence has been reviewed and their scope is explicit. Examples may include Culture House, Silpo, Central Park, street/road archetypes, and Oster private-sector archetypes.

A pack is not runtime accepted merely because it is listed here.
