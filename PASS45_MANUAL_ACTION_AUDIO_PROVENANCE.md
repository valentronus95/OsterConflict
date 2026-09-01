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

Because Freesound requires account login for the original WAV download, repository intake must not fake an original-file acquisition. `PASS45_MANUAL_ACTION_AUDIO_INTAKE.py` instead uses the exact audited public Freesound preview URL as the transport for each donor.

The transport is treated only as a CC0 preview derivative source:

- source-page identity/license markers are revalidated before download;
- source-page provenance and transport-byte identity are verified independently: mutable HTML preview advertising is not authoritative after an exact public preview URL and checksum have been audited;
- write mode downloads only the exact pinned public-preview URL, never a dynamically discovered HQ/LQ variant or mirror;
- transport bytes are SHA-256 hashed and must equal the pinned audit value before conversion;
- conversion is deterministic: metadata removed, mono, 48 kHz, PCM signed 16-bit WAV;
- derivative WAV bytes are separately SHA-256 hashed and duration-checked against the source-page duration;
- no changed public byte stream may silently replace the audited donor;
- even after repository acquisition, the donor remains `runtime_ready=0` / `ue_import_pending=1` until an actual UE SoundWave is imported and accepted;
- preview transport is never described as the original Freesound WAV.

Workflow: `.github/workflows/pass45-manual-action-audio-intake.yml`.

## Audit-only checksum result

Audit commit: `17a3f1076c116edc32f5846f9abdecf5c7c9229f`.
Workflow run: `33482463387` — **SUCCESS**.

Lever public preview:

- transport: `https://cdn.freesound.org/previews/523/523401_9-lq.mp3`
- transport SHA-256: `7785b4db5b512cec45da227097789dab4510aafec1f7e5d9f260669f54ed75ab`
- deterministic derivative SHA-256: `1d59d0908bdc84d9bc648c79d5f2041d33f875ecaf4a1dbf2364ca7a5a3736fb`
- derivative size: `92046` bytes
- derivative duration: `0.958333 s`

Bolt public preview:

- transport: `https://cdn.freesound.org/previews/263/263459_3988807-lq.mp3`
- transport SHA-256: `635a4fd88454a032a476445237befb536ab532c1bdf573249653011bff4dde9e`
- deterministic derivative SHA-256: `fd328522972497fc98a8236a2efdc7d5b77515c54d6811e9e2873fb6ff15d09c`
- derivative size: `624078` bytes
- derivative duration: `6.500000 s`

Write mode is fail-closed on the exact audited LQ transport URLs and SHA-256 values above. A newly advertised HQ/LQ variant, mirror transport or changed byte stream must not silently replace the audited donor.

These exact audited LQ transport URLs and SHA-256 values are the only transport coordinates accepted by `--mode write`. If the source page changes its HTML preview presentation while the exact pinned URL remains downloadable with the same hash, intake may continue; if the pinned URL disappears or its bytes change, intake fails.

## Next factual intake

- Acquire the two pinned CC0 preview derivatives through Git LFS without bypassing repository content rules.
- Verify the committed LFS pointer OIDs equal the deterministic derivative SHA-256 values above.
- Import to repository-owned UE SoundWave assets, wire `BoltCycle` / `LeverCycle`, then run source guards and local UE 5.8 audibility/timing acceptance.
- Keep M700/Remington870/Lever authored animation content gaps explicit until accepted skeletal sequences are present.
