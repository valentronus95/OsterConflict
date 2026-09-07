# PASS45 single-branch consolidation audit

Status date: 2026-08-28
Canonical execution branch: `fix/pass45-runtime-rejection-material-closure-20260826`
Canonical execution specification: `PASS45_RUNTIME_RECOVERY_TZ.md`
Latest accepted factual runtime verdict: `RUNTIME REJECTED 2026-08-27`

## 1. Authority rules

1. User screenshots and direct local UE 5.8 runtime evidence outrank green source/CI checks.
2. `PASS45_RUNTIME_RECOVERY_TZ.md` is the only execution specification. This file is a subordinate audit ledger, not a second TZ.
3. Old Pass45 branches may contain useful historical evidence, but they must not be cherry-picked wholesale when their contracts are stale or superseded.
4. `SOURCE-CODED`, `SOURCE-VERIFIED`, `CONTENT GAP`, `RUNTIME REJECTED`, and `RUNTIME ACCEPTED` are distinct states. No source verifier may promote runtime acceptance.
5. Weapon visual readiness remains `exact production visual OR explicit real-mesh fallback`; fallback keeps the exact-production item as `CONTENT GAP` and never impersonates exact-production READY.
6. Museum/Silpo/Culture House reference packs guide source reconstruction, but they do not create READY without current-head UE 5.8 visual acceptance.

## 2. Canonical reference bindings retained

The canonical branch already carries the compact repository-side reference contracts:

- `PASS45_REFERENCE_PACK_BINDINGS.md`
- `_DOCS/REFERENCE_PACKS/LOC_MUSEUM_001_OSTER_MUSEUM/REFERENCE_SPEC.md`
- `_DOCS/REFERENCE_PACKS/LOC_SILPO_002_OSTER_SILPO/REFERENCE_SPEC.md`
- `_DOCS/REFERENCE_PACKS/LOC_CULTURE_HOUSE_003_OSTER_CULTURE_HOUSE/REFERENCE_SPEC.md`
- `Tools/Windows/VERIFY_PASS45_REFERENCE_PACKS.ps1`

External detailed source material remains outside the repository and is bound through the compact specs rather than duplicated into Git:

Museum:
- `/mnt/data/LOC_MUSEUM_001__01_SITE_OVERVIEW_AND_LAYOUT.md`
- `/mnt/data/LOC_MUSEUM_001__02_BUILDING_ARCHITECTURE.md`
- `/mnt/data/LOC_MUSEUM_001__03_GROUNDS_STADIUM_AND_UE5_NOTES.md`
- `/mnt/data/OSTER_CONFLICT_REFERENCE_PACKS/LOC_MUSEUM_001_OSTER_MUSEUM/`
- `/mnt/data/OSTER_CONFLICT_REFERENCE_PACKS/LOC_MUSEUM_001_OSTER_MUSEUM/SOURCE_PHOTO_MANIFEST.md`

Silpo:
- `/mnt/data/LOC_SILPO_002__01_SITE_ORIENTATION_AND_URBAN_CONTEXT.md`
- `/mnt/data/LOC_SILPO_002__02_BUILDING_ARCHITECTURE_AND_INTERIOR.md`
- `/mnt/data/LOC_SILPO_002__03_WATER_TOWER_SURROUNDINGS_AND_UE5.md`

Museum source truth that must not be lost during model cleanup:
- one-storey, low horizontal historic main building;
- pale yellow/cream plaster, white window trim, low grey pitched/hipped roof;
- simple window rhythm and entrance, no invented columns/towers/ornament;
- distinct western outbuilding, north/north-east service context and central yard/open grass relationships where supported by the references;
- the large south/south-east grassy rectangle is not automatically a verified stadium merely because a convenient old source label says so;
- neighboring modern/taller light building mass must remain separate unless identity is verified.

Silpo source truth that must not be lost:
- single-storey strongly horizontal grocery/supermarket frontage;
- readable entrance/shopfront and street relationship;
- nearby water tower is a separate context landmark, never joined to the store mesh;
- no invented interior/public route without direct evidence;
- Silpo must not absorb or replace Culture House identity.

## 3. Old Pass45 branch audit

### Fully contained by the canonical branch

These branches currently have no unique commits relative to the canonical branch and therefore require no cherry-pick:

- `audit/pass45-tz-completion-20260825`
- `fix/runtime-acceptance-pass45-environment`
- `fix/runtime-acceptance-pass45-generic-residential-retirement-20260826`
- `fix/runtime-acceptance-pass45-r14-verifier-forward-port-20260826`
- `chore/pass45-postmerge-state-sync-20260825`
- `chore/pass45-pr91-postmerge-state-20260826`
- `fix/pass45-btr-authored-material-20260825`
- `fix/pass45-completion-audit-20260825`
- `fix/pass45-content-visual-cleanup-20260825`
- `fix/pass45-local-build-import-regression-20260825`
- `fix/pass45-strict-runtime-acceptance-harness-20260825`
- `fix/pass45-weapon-material-closure-20260826`
- `fix/pass45-weapon-material-dependency-audit-20260825`

### Divergent historical branches: do not cherry-pick wholesale

`chore/pass45-postmerge-state-sync-2-20260825`
- contains only stale ledger/TZ state synchronization from 2026-08-25;
- canonical branch is hundreds of commits newer;
- useful historical truth is the need to keep TZ/ledger synchronized, not the old state text itself.

`fix/pass45-content-gap-truth-20260825`
- retains historical fail-fast content-gap work and explicit not-ready checks;
- its old exact-content policy predates the canonical `exact production OR explicit real-mesh fallback` rule;
- preserve the fail-closed principle and explicit CONTENT GAP reporting, not the obsolete exact-only implementation.

`fix/pass45-postmerge-content-closure-20260825`
- contains older content-dependency and fresh-load verifiers plus broad old world/model edits;
- some dependency assumptions are incompatible with current authored BTR R3/glTF/vertex-colour handling;
- preserve independent fresh-load validation as a principle, not old hard-coded texture-name assumptions.

`fix/pass45-runtime-rejection-20260825`
- contains useful historical material-remediation/revision/fresh-load rejection machinery;
- current branch has newer Stein R3, M2 authored-pivot, BTR R3 Y-up/+X-forward, weapon fallback, runtime evidence and strict acceptance architecture;
- do not revive old material/world mutations wholesale. Preserve only the facts that stale assets, default materials, zero meaningful dependencies and stale runtime revisions are rejection conditions.

## 4. Previously at-risk truths now explicitly consolidated

- Latest runtime truth is 2026-08-27, not 2026-08-25/26.
- M2 source uses authored pivot/hierarchy logic; old bounds-bottom/longest-axis alignment must not return.
- BTR source uses the current R3 Y-up import/+X-forward contract with one explicit compensation path; old duplicate axis fixes must not return.
- Uncalibrated ADS is presentation fail-closed; requested aiming must not apply guessed arms/weapon offsets until exact per-weapon calibration is proven in UE 5.8.
- Authored vegetation source and primitive-tree guards are source evidence only; rejected screenshots still outrank them.
- Museum, Culture House and Silpo must have separate identities/components and must not overlap into one another.
- Water tower is Silpo-area context, not part of Silpo geometry.
- Museum-adjacent south grassy rectangle remains semantically uncertain until independently verified; do not hard-code a stadium identity from inference alone.
- Green CI never overrides direct runtime rejection.

## 5. Current completion accounting

Canonical numbered execution checklist: 36 items.

At this consolidation checkpoint:
- checked/source-reference completed: 22/36 = 61.1%;
- still open: 14/36 = 38.9%.

Open numbered items are synchronized to the current canonical `PASS45_RUNTIME_RECOVERY_TZ.md`:
- 16: replace procedural manual-action cues with accepted authored moving-part/skeletal presentation where supported, and populate real bolt/pump/lever sound content;
- 18: exact per-weapon rear/front/optic references and ADS transform calibration in local UE 5.8;
- 20: replace the temporary generic audio fallback with accepted exact per-weapon shot/reload/distant/mechanical profiles;
- 24: accepted first-person grenade hand/throw/recover animation, distinct frag/smoke/flash presentation and real smoke VFX;
- 27: replace rejected vegetation family and complete broader environment acceptance;
- 28: runtime-accept HMMWV/M2 ring/shield/gunner hierarchy with authored pivot, 360-degree yaw and correct camera;
- 29: runtime-calibrate HMMWV top speed to at least 80 km/h without breaking handling;
- 30: close BTR white-material state across pre/post possession in runtime;
- 31: runtime-accept BTR R3 Y-up/+X-forward orientation and remote operator monitor/optic gameplay;
- 32: raise core world/material/LOD fidelity above prototype state, including dedicated `ParkPaths`, ground and landmark surroundings;
- 33: fullscreen + 60 FPS + thermal-soak validation after visual fixes;
- 34: tactical-map screenshot validation;
- 35: current-head `START_HERE.cmd -> 2. ПОВНИЙ RUNTIME-ТЕСТ` import/build/gameplay/automated-gates/direct-screenshot run;
- 36: merge PR #94 only after factual current-head runtime acceptance.

This percentage is checklist/source-reference completion, not release readiness. Final runtime acceptance remains rejected until a current-head full UE 5.8 run passes every mandatory gate and direct visual review.

## 6. Single-branch continuation rule

All new Pass45 production/source work, verifiers, reference bindings and execution-state updates go to:

`fix/pass45-runtime-rejection-material-closure-20260826`

Do not create another implementation branch to continue this TZ. Historical branches are audit sources only. Any useful fact mined from them must be rewritten against the current contracts on the canonical branch rather than merged mechanically.
