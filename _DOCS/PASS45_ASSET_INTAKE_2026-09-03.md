# PASS45 Asset Intake — 2026-09-03

Status: **QUARANTINED / NOT PRODUCTION / NOT RUNTIME-ACCEPTED**

This file is a governed sub-process of `PASS45_RUNTIME_RECOVERY_TZ.md`. It does **not** add checklist items 37+ and does not change the canonical 36-item execution order.

## 1. Transport / quarantine truth

- Staging branch: `asset-intake-20260903`.
- Quarantine head after exact Kar98k duplicate removal: `3d8b88aa47c41923603174b474a7f8d583990130`.
- Intake payload: **18 ZIP archives**, transported with Git LFS; the upload reported approximately **3.4 GB**.
- Exact duplicate removed from quarantine: `models_game_OC/kar98k-free-model (1).zip`.
- The quarantine branch is a transport/source holding area only.
- **Never merge `asset-intake-20260903` wholesale into PR #94, PASS45, or `main`.**
- Raw ZIP presence is not evidence of license, UE compatibility, production quality, or runtime acceptance.

## 2. Current quarantine inventory

### Weapon candidates

1. `models_game_OC/ak-74m.zip`
2. `models_game_OC/ar15-rifle.zip`
3. `models_game_OC/assault-rifle-m4a1.zip`
4. `models_game_OC/colt-m1911.zip`
5. `models_game_OC/fn-ballista-sniper-rifle.zip`
6. `models_game_OC/kar98k-free-model.zip`
7. `models_game_OC/law-light-anti-tank-weapon-m72.zip`
8. `models_game_OC/makarov-pistol.zip`
9. `models_game_OC/rpg-26-grenade-launcher-low-poly.zip`
10. `models_game_OC/shotgun.zip`
11. `models_game_OC/tommy-gun.zip`

### Oster/world candidates

12. `models_game_OC/town/fivestory-building-appartament-of-post-soviet.zip`
13. `models_game_OC/town/konserva-sardines.zip`
14. `models_game_OC/town/po-2-fence.zip`
15. `models_game_OC/town/pripyat-chainlink-fence.zip`
16. `models_game_OC/town/pripyat-light-poles.zip`
17. `models_game_OC/town/telephone-pole-scene.zip`
18. `models_game_OC/town/ukrainian-cherry-juice-nash-sik.zip`

## 3. Known public-source anchors from the acquisition session

These anchors are convenience evidence only. License/public-repository permission still has to be pinned per payload before promotion.

| Intake archive / candidate | Known source anchor | Current intake license state |
|---|---|---|
| `ak-74m.zip` | `https://sketchfab.com/3d-models/ak-74m-84bb0a58a18047069706047639b82899` | **PENDING exact license verification** |
| `makarov-pistol.zip` | `https://sketchfab.com/3d-models/makarov-pistol-c201d870e7bd473da5d77775a6e5c4f8` | page was observed as **CC Attribution**; exact downloaded payload still must be matched |
| `law-light-anti-tank-weapon-m72.zip` | `https://sketchfab.com/3d-models/law-light-anti-tank-weapon-m72-70814e3b3a0e40bdad0235c6f5637047` | **PENDING exact license verification** |
| `rpg-26-grenade-launcher-low-poly.zip` | `https://sketchfab.com/3d-models/rpg-26-grenade-launcher-low-poly-92c9de9f829f4bdea5e215e8f8530d20` | **PENDING exact license verification** |
| `fivestory-building-appartament-of-post-soviet.zip` | `https://sketchfab.com/3d-models/fivestory-building-appartament-of-post-soviet-218130a3ab3c4c67baec224c214cda9c` | **PENDING exact license verification** |
| `konserva-sardines.zip` | `https://sketchfab.com/3d-models/konserva-sardines-121ed6fc2c504328b99a8aecd1d8c5de` | page was observed as **CC Attribution**; exact downloaded payload still must be matched |
| `pripyat-chainlink-fence.zip` | `https://sketchfab.com/3d-models/pripyat-chainlink-fence-35adb2f2461c4d30bc94e803bb50de50` | **PENDING exact license verification** |
| `pripyat-light-poles.zip` | `https://sketchfab.com/3d-models/pripyat-light-poles-7166d53851a54d2fb2bdbf2695694e10` | **PENDING exact license verification** |
| `ukrainian-cherry-juice-nash-sik.zip` | `https://sketchfab.com/3d-models/ukrainian-cherry-juice-nash-sik-3858e2210e104201b6894ecfb3aee271` | page was observed as **CC Attribution**; exact downloaded payload still must be matched |

The exact source URL/license for `ar15-rifle`, `assault-rifle-m4a1`, `colt-m1911`, `fn-ballista-sniper-rifle`, `kar98k-free-model`, `shotgun`, `tommy-gun`, `po-2-fence`, and `telephone-pole-scene` is currently **UNKNOWN/PENDING**. Unknown provenance is fail-closed: these payloads cannot become production dependencies until resolved.

Other models discussed during acquisition but not present in this 18-archive quarantine batch are not assumed to exist.

## 4. Mandatory intake pipeline

Every archive must pass this sequence before any production cutover:

1. **Archive identity**
   - local actual-payload SHA-256;
   - compressed size;
   - exact duplicate detection.
2. **Safe archive inventory**
   - reject ZIP path traversal / absolute-path entries;
   - reject encrypted archives for unattended intake;
   - enumerate suspicious executable/script payloads.
3. **Content inventory**
   - enumerate FBX / GLB / glTF / OBJ / other model files;
   - enumerate textures/material-related files;
   - enumerate rig/skeleton/animation clues;
   - enumerate audio and documentation files.
4. **Provenance / license**
   - exact creator/publisher;
   - exact source URL;
   - exact license;
   - attribution requirements;
   - public-repository redistribution permission;
   - version/payload identity.
5. **Candidate quality audit**
   - geometry/polycount and topology;
   - materials/textures/resolution;
   - scale/orientation/pivots;
   - moving-part separation;
   - skeleton/rig/animation suitability;
   - collision/LOD/Nanite suitability;
   - memory/streaming implications.
6. **Classification**
   - `REJECT`;
   - `DONOR_ONLY`;
   - `PRODUCTION_CANDIDATE`.
7. **Promotion**
   - only verified candidates move from quarantine into `SOURCE_ASSETS/PASS45/...`;
   - create an individual record in `_DOCS/THIRD_PARTY_CODE_AND_ASSET_REGISTER.md`;
   - do not preserve an unverified raw ZIP in the production branch merely because it was downloaded.
8. **Isolated UE 5.8 proof**
   - import/fresh-load;
   - Data Validation;
   - skeleton/material/animation/collision checks;
   - no gameplay authority switch yet.
9. **Integration**
   - bind only to an existing factual gap or an explicitly accepted weapon/world role;
   - preserve one gameplay/source-of-truth owner;
   - avoid constructor/startup-heavy hard references when soft/async ownership is appropriate.
10. **Cutover / cleanup**
    - switch production authority only after proof;
    - remove the obsolete duplicate/fallback owner;
    - remove stale tests/config/fallbacks.
11. **Acceptance**
    - source/regression gates first;
    - final gameplay/network/performance/visual/audio runtime evidence after the relevant setup group is complete.

## 5. Weapon execution rule — batch technical setup, consolidated runtime

For weapon work, PASS45 must **not** run the full UE 5.8 runtime acceptance after every tiny model/audio/animation edit.

Required cadence:

1. perform bounded source/import/fresh-load/animation/audio checks for each weapon candidate while integrating it;
2. finish the intended weapon setup set: model, materials, moving parts, ADS, recoil, reload/manual action, audio, pickup/presentation and network-safe binding;
3. only then run **one consolidated current-head weapon runtime acceptance** covering the configured weapon set;
4. retain direct gameplay visual/audio evidence;
5. only accepted weapons may count toward checklist closure.

This batching rule does not weaken fail-closed acceptance. It removes repeated expensive full-runtime runs while preserving cheap early technical gates.

## 6. Binding to the frozen 36-item PASS45 checklist

The intake does not create new checklist numbers. Accepted payloads may close or improve existing work only where they factually apply:

- **Item 16** — authored manual-action presentation/audio for M700 / Remington 870 / Lever Action; new weapon candidates do not bypass this first open gate.
- **Item 18** — accepted weapon ADS/presentation calibration.
- **Item 20** — accepted exact per-weapon audio.
- **Item 28** — M2 production assembly only if a legally/technically acceptable M2 source is later present and proven.
- **Item 32** — environment/world fidelity: fences, poles, residential/support props and other reusable generic world content after location/style/LOD/material acceptance.
- **Item 35** — final current-head runtime acceptance.

M72/RPG-26 and other newly downloaded weapons are expansion candidates. They do not silently become mandatory PASS45 blockers and they do not preempt item 16.

## 7. Oster map-fidelity rule

“Looks similar to Oster” is enough for **candidate intake**, not enough for final placement truth.

- Generic fences, light poles, utility props and household/shop props may become reusable modular kits.
- Residential buildings may be adapted where photo/geographic evidence supports the massing and placement.
- A generic post-Soviet building must not replace a photo-bound landmark merely because the silhouette feels plausible.
- Museum, Silpo, Culture House, stadium and other evidence-bound landmarks keep their existing reference/geo acceptance requirements.
- Pripyat/STALKER-style weathering must not turn living Oster into an abandoned-zone visual theme unless a specific local reference supports it.

## 8. Current acceptance state

```text
asset_intake_quarantine=1
production_cutover=0
runtime_visual_acceptance=0
runtime_acceptance=0
item16_checked=0
merge_permitted=0
official_progress=22/36=61.1%
```

PR #94 remains OPEN / UNMERGED.

## 9. First executable operation

Run:

`OsterConflict\RUN_PASS45_ASSET_INTAKE_20260903.cmd`

The launcher audits the local actual ZIP payloads and writes reports under ignored `PC_TEST/TEST_RESULTS/`. It performs **no extraction into production, no Git mutation, no UE import and no deletion**.

After that report exists, resolve provenance and classify candidates before any selective extraction/promotion.
