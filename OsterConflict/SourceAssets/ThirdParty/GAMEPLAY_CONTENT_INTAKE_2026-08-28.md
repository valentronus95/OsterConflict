# Oster Conflict — gameplay content intake

Date: 2026-08-28
Branch: `content/free-gameplay-assets-intake-20260828`
Base: `fix/pass45-runtime-rejection-material-closure-20260826@4aaf29f3c0ae74792ad72ff41184bb9b5445d26b`

This intake exists to acquire missing presentation content without replacing the existing Oster Conflict gameplay/runtime architecture.

## Rules

1. Raw downloads stay under `SourceAssets/ThirdParty/Gameplay` and are not treated as production Unreal assets merely because they exist on disk.
2. First-person arms already exist in Oster Conflict as `/Game/QuantumCharacter/Mesh/Modules/SKM_Arms`. External arms are animation/rig/reference donors until a retarget pass proves they are useful.
3. No third-party pack replaces Character Movement, Chaos physics, Niagara, IK, replication, recoil ownership, fire modes, weapon drop physics, or the existing first-person weapon presentation code.
4. Every network source is pinned by immutable commit or is a direct upstream asset URL with a recorded SHA-256 where a verified hash is available.
5. A source with ambiguous or repository-level missing licensing must remain a candidate until the exact shipped files are re-verified. Human beings have invented enough licensing surprises already.
6. Generated/imported `.uasset` content remains subject to the existing PASS45 runtime gates.

## Approved intake sources

### OC-INTAKE-ARMS-001 — FPS Arms 3D

- Repository: `Ayush-Mohanty/FPS-Arms-3D`
- Pinned commit: `c80a452e680c5a27c9e936176eb17e446869529a`
- Intake: `Models/arms_model.fbx`, glove texture, and the bundled AK-74M animation donor files.
- Repository license: MIT.
- Important nested-license exception: `Models/fps_ak-74m_animations/license.txt` identifies the AK-74M animation asset as Leonard Koch / Sketchfab under CC BY 4.0. Preserve that attribution if any part is shipped.
- Status: `CANDIDATE_FOR_RETARGET`, not a replacement for canonical `SKM_Arms`.

### OC-INTAKE-FPSKIT-001 — FPS Asset Kit

- Repository: `petroulacl/fps-asset-kit`
- Pinned commit: `a19b7458a593598211c95ec46ef4eb4b6d1f94d7`
- Intake: `sfx/` and `weapons/` only; unrelated web/game scripts and bulk texture/HDRI material are not copied into the Oster working source tree.
- Source README states that the curated kit is CC0 and lists Freesound provenance for its gunshot/footstep files.
- GitHub repository metadata does not expose a repository-level license identifier, so shipping files from this kit remain `SOURCE_ASSERTED_CC0_REVERIFY_BEFORE_SHIP` until the selected exact files have a final provenance row.
- Status: `CANDIDATE`.

### OC-INTAKE-SHOTGUN-001 — CC0 shotgun recordings

- Repository mirror/source record: `yegors/hard-lines`
- Pinned commit: `7c1f90b0295030e60cf9ea731371156466ddb181`
- Files: `public/audio/sg-report-0.wav`, `public/audio/sg-report-1.wav`.
- Recorded origin: Benelli Nova, 12 gauge, from The Free Firearm Sound Library.
- License recorded by source: CC0 1.0.
- Intended use: Remington-class shotgun candidate after mix/identity review.
- Status: `APPROVED_SOURCE_CANDIDATE`.

### OC-INTAKE-FOOTSTEP-001 — CC0 concrete footsteps

- Repository mirror/source record: `yegors/hard-lines`
- Pinned commit: `7c1f90b0295030e60cf9ea731371156466ddb181`
- Files: `public/audio/step-hard-0.wav` through `step-hard-3.wav`.
- Recorded origin: Joseph Sardin / BigSoundBank, shoe on concrete.
- License recorded by source: CC0 1.0.
- Status: `APPROVED_SOURCE_CANDIDATE`.

### OC-INTAKE-ENGINE-001 — CC0 engine RPM loops

- Repository mirror/source record: `yashimosh/border-run`
- Pinned commit: `38d6afa442a06bbd414f21ac05c0f1b4d08bd705`
- Files: `public/sfx/engine/loop_0.wav`, `loop_1_0.wav`, `loop_2_0.wav`, `loop_3_0.wav`, `loop_4_0.wav`, `loop_5_0.wav`.
- Recorded origin: OpenGameArt, `racing-car-engine-sound-loops`.
- License recorded by source: CC0.
- Intended use: generic RPM-layer prototype for vehicle audio architecture; not claimed to be a factual HMMWV/BTR-4 engine recording.
- Status: `APPROVED_SOURCE_CANDIDATE`.

### OC-INTAKE-AMBIENCE-001 — Forest ambience

- Upstream: OpenGameArt / TinyWorlds, `Forest Ambience`.
- URL: `https://opengameart.org/sites/default/files/Forest_Ambience.mp3`
- Verified source SHA-256: `9850AA1D0D5D66BD9C5DAF8BB77C6D852E01F2F4DE22F283BD5621E8BED13B75`
- License record: CC0 1.0.
- Status: `APPROVED_SOURCE`, source-format MP3; conversion/import into Unreal must be loss-aware and deterministic.

### OC-INTAKE-FIRE-001 — Fire ambience

- Upstream: OpenGameArt / PagDev, `Fireplace Sound Loop`.
- URL: `https://opengameart.org/sites/default/files/fire.wav`
- Verified source SHA-256: `85CA0CC60D0C037FFF8B185E31AD1FCDBDA6CE45EEE17C3EE1318D1B8F59E330`
- License record: CC0 1.0.
- Status: `APPROVED_SOURCE_CANDIDATE`.

## Content gaps intentionally not solved by downloading random packs

- **First-person arms visibility:** fix the existing `SKM_Arms` component/ownership/transform/AnimBP chain first.
- **Weapon-specific animation completeness:** external AK animation is only a donor. Remington pump, M249 feed/charge, M700 bolt and other weapon-specific actions still require correct rigs/retargeting or authored procedural animation.
- **Muzzle/smoke/impact/explosion VFX:** author with UE 5.8 Niagara inside Oster Conflict; do not import an unrelated monolithic VFX framework.
- **Gravity / ragdoll / collision / IK:** use UE 5.8 Character Movement, Chaos, PhysicsAsset and IK/Control Rig.
- **Exact firearm identity:** generic recordings may support prototypes/mix architecture, but final production mapping must not claim a source weapon is another weapon.

## Local acquisition

Run from the repository/project tree:

```bat
OsterConflict\Scripts\FETCH_GAMEPLAY_CONTENT_58.cmd
```

The script downloads immutable/pinned sources, validates the two upstream assets with known SHA-256 values, extracts only the relevant archive folders, and writes `LOCAL_CONTENT_RECEIPT.csv` with SHA-256 hashes for every acquired local file.

The downloader does **not** silently commit large binaries and does **not** declare them production-ready. Import/retarget/runtime acceptance comes after factual inspection of the acquired assets.
