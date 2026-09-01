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

## Next factual intake

- Acquire the two pinned CC0 source payloads without bypassing repository LFS/content rules.
- Prepare minimal deterministic bolt/lever derivatives only if needed for timing/clean cycle isolation.
- Import to repository-owned UE SoundWave assets, wire `BoltCycle` / `LeverCycle`, then run source guards and local UE 5.8 audibility/timing acceptance.
- Keep M700/Remington870/Lever authored animation content gaps explicit until accepted skeletal sequences are present.
