# PASS45 Manual-Action Audio Provenance

This file is the source-of-truth intake contract for canonical `PASS45_RUNTIME_RECOVERY_TZ.md` item 16.

## Status

- Canonical item: **16 — bolt / pump / lever manual-action presentation and real mechanical audio**.
- Current checklist state: **OPEN**.
- Current runtime verdict: **RUNTIME REJECTED 2026-08-31**.
- This document does **not** promote item 16 to READY or RUNTIME ACCEPTED.
- Current repository-owned PumpCycle remains `/Game/R13/Audio/shotguncock.shotguncock`.
- Current repository-owned BoltCycle: **CONTENT GAP**.
- Current repository-owned LeverCycle: **CONTENT GAP**.

## Accepted provenance candidates

### Lever-cycle donor

- Source page: `https://freesound.org/people/C-V/sounds/523401/`
- Title: `Lever action cocking.wav`
- Author/uploader: `C-V`
- License on source page: **Creative Commons 0 (CC0)**.
- Source description: a real `.22 caliber lever action rifle` being cocked.
- Source duration: `0.958 s`.
- Source format: WAV, 48 kHz, 24-bit, stereo.
- Intended role: **real lever-action mechanical donor** for `LeverCycle` after the payload is actually acquired, repository-tracked, imported and accepted in UE 5.8.
- Identity limitation: this is **not** evidence of an exact Stein/Marlin/Model-1894 recording and must never be labeled exact weapon identity.

### Bolt-cycle donor

- Source page: `https://freesound.org/people/rammbostein/sounds/263459/`
- Title: `Mosin Nagant Bolt.wav`
- Author/uploader: `rammbostein`
- License on source page: **Creative Commons 0 (CC0)**.
- Source description: a real old bolt-action Russian rifle being cocked at a slow/moderate pace.
- Source duration: `6.500 s`.
- Source format: WAV, 16 kHz, 16-bit, mono.
- Intended role: **real bolt-action mechanical donor** for `BoltCycle` after an accepted deterministic edit selects the factual bolt cycle, the result is repository-tracked, imported and accepted in UE 5.8.
- Identity limitation: this is a Mosin-Nagant donor. It is **not** an M700 recording and must never be labeled exact M700 identity.

## Secondary CC0 library

- OpenGameArt page: `https://opengameart.org/content/the-free-firearm-sound-library`
- License: **CC0 / NO RIGHTS RESERVED**.
- Recorded by Ben Jaszczak, Brian Nelson, Kevin Heras and Matthew Nanney.
- Public GitHub mirror audited during this pass: `https://github.com/buddingmonkey/FreeFirearmsSFXLibrary`.
- Mirror branch audited: `main`.
- Mirror commit audited: `beb2f4041f3d6740fa0aeaf0e71159bd65a78c1b`.
- The prepared SFX sheet is gunshot-oriented. Folder names such as Model 1894 / Savage 10 / Mosin Nagant are **not sufficient evidence** that a particular file is a lever/bolt mechanical cycle. No file from that mirror may be wired to `BoltCycle` or `LeverCycle` without matching metadata or direct audition evidence.

## Intake rules

1. No URL, title, tag, filename or folder name by itself counts as runtime content.
2. Audio bytes must be intentionally acquired from the pinned source, then tracked under the repository content policy before any runtime profile is wired.
3. `*.wav` is Git LFS-controlled by this repository. Do not bypass LFS by committing raw WAV bytes through a normal Git blob.
4. Any transcoded/trimmed derivative must record the source page, author, CC0 license, deterministic edit parameters and checksum of the derivative.
5. A donor may prove a **real action family** sound without proving **exact weapon identity**. Do not promote donor identity beyond what the source proves.
6. `BoltCycle` and `LeverCycle` remain unassigned/fail-visible until the accepted payload exists in the canonical branch and UE 5.8 can load it.
7. Item 16 remains unchecked until the required authored moving-part/skeletal animations, real mechanical audio and local UE 5.8 first-person acceptance all pass.
8. PR #94 remains OPEN / UNMERGED until the cumulative runtime acceptance rule is satisfied.

## Controlled public-preview acquisition — 2026-09-01

Because Freesound requires account login for the original WAV download, repository intake must not fake an original-file acquisition. `PASS45_MANUAL_ACTION_AUDIO_INTAKE.py` uses an exact audited public Freesound LQ preview URL as the transport for each donor.

The transport is treated only as a CC0 preview derivative source:

- source-page identity/license markers are revalidated before download;
- source-page provenance and transport-byte identity are verified independently;
- the workflow separately audits currently advertised preview candidates, but those candidates never auto-replace a pin;
- write mode downloads only the exact pinned public-preview URL, never a dynamically selected HQ/LQ variant or mirror;
- transport bytes are SHA-256 hashed and must equal the pinned audit value before conversion;
- conversion is deterministic: metadata removed, mono, 48 kHz, PCM signed 16-bit WAV;
- derivative WAV bytes are separately SHA-256 hashed and duration-checked against the source-page duration;
- no changed public byte stream may silently replace the audited donor;
- even after repository acquisition, the donor remains `runtime_ready=0` / `ue_import_pending=1` until an actual UE SoundWave is imported and accepted;
- preview transport is never described as the original Freesound WAV.

Workflow: `.github/workflows/pass45-manual-action-audio-intake.yml`.

## Retired initial transport audit

The first audit at commit `17a3f1076c116edc32f5846f9abdecf5c7c9229f` / run `33482463387` used older Freesound preview coordinates. On 2026-09-01 both previously pinned preview identities were found stale after Freesound regenerated its preview filenames. The lever old pin returned HTTP 404 during run `33500960707`; it is no longer accepted by write mode.

## Current transport re-audit — 2026-09-01

Workflow run `33501389638` revalidated both source pages, identity/license markers, every currently advertised preview byte stream, and reported exact SHA-256 values. The canonical write pin remains deliberately LQ MP3 for deterministic continuity.

Lever public preview:

- transport: `https://cdn.freesound.org/previews/523/523401_8956746-lq.mp3`
- transport SHA-256: `ae257485c6d55f4a4587f99389882cf74eae6779db807eaa0aa0f968e711f965`
- transport size: `7536` bytes
- derivative SHA-256: **pending controlled write acquisition**

Bolt public preview:

- transport: `https://cdn.freesound.org/previews/263/263459_4174990-lq.mp3`
- transport SHA-256: `d9f4ee7633275f911f3521b5b7b319d634022944aafb9e7f51660a8a342d3040`
- transport size: `59508` bytes
- derivative SHA-256: **pending controlled write acquisition**

These current audited LQ transport URLs and SHA-256 values are the only transport coordinates accepted by `--mode write`. If a pin disappears or its bytes change, intake fails closed and reports current page candidates for a new explicit audit. It never silently follows a changed page.

## Next factual intake

- Acquire the two current pinned CC0 preview derivatives through Git LFS without bypassing repository content rules.
- Verify the committed LFS pointer OIDs against the deterministic derivative SHA-256 values emitted by the manifest.
- Import to repository-owned UE SoundWave assets, wire `BoltCycle` / `LeverCycle`, then run source guards and local UE 5.8 audibility/timing acceptance.
- Keep M700/Remington870/Lever authored animation content gaps explicit until accepted skeletal sequences are present.
