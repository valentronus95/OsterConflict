# PASS45 Runtime Recovery — Persistent Work History

This is the current human-readable checkpoint index for `PASS45_RUNTIME_RECOVERY_TZ.md`.

The immediately preceding detailed checkpoint remains preserved in Git at the parent history blob and in:
`PASS45_RUNTIME_RECOVERY_HISTORY_ARCHIVE_PRE_CURRENT_HEAD_PREFLIGHT_2026-09-02.md`.

Git history remains the raw source of truth. This live file intentionally records only the newest factual continuation state.

## Canonical ownership

- Canonical TZ: `PASS45_RUNTIME_RECOVERY_TZ.md`.
- Active integration branch: `fix/pass45-runtime-rejection-material-closure-20260826`.
- Target baseline: `main@bca00f4046700f383af9f1742cc24b6a62401b1a`.
- Active integration PR: **#94 OPEN / UNMERGED**.
- Pre-intake-governance head: `7243a049513e981604758a494ec2ce40df7c1cda`.
- PR #94 must remain unmerged until current-head local UE 5.8 runtime acceptance and remaining branch-hygiene gates pass.
- Official canonical checklist accounting remains **22/36 = 61.1% complete, 38.9% remaining**.
- Local user `Changes`, including the two local `PC_TEST/TEST_RESULTS/*.zip` worktree differences, remain outside assistant mutation scope.

## First factual open item

The first canonical unchecked checklist item remains **item 16**: accepted authored M700 / Remington 870 / Lever Action moving-part or skeletal manual-action presentation, factual bolt/pump/lever mechanical audio, and local UE 5.8 acceptance.

Source/docs/intake work does not check item 16 and does not increase the official percentage.

## Current item-16 source state

Current canonical source already contains the one-shot local evidence-chain launcher:

`OsterConflict/RUN_PASS45_ITEM16_LOCAL_UE58_EVIDENCE.cmd`

It covers bounded M700, Remington 870, Lever Action and manual-action audio proof while explicitly preserving:

```text
runtime_visual_acceptance=0
runtime_acceptance=0
item16_checked=0
merge_permitted=0
```

The latest direct user runtime evidence from 2026-09-02 remains authoritative for the pre-cutover run: gameplay was reached, M700 and Lever Action were usable, Remington fired/recoiled, but its fore-end did not visibly pump. Later source work added the guarded Remington skeletal/PumpCycle path, so current-head direct visual/audio acceptance remains pending.

## 2026-09-03 downloaded asset intake

A user-downloaded candidate batch is now quarantined separately from production:

- transport branch: `asset-intake-20260903`;
- quarantine head after duplicate cleanup: `3d8b88aa47c41923603174b474a7f8d583990130`;
- current inventory: **18 ZIP archives**;
- Git LFS upload reported approximately **3.4 GB**;
- exact duplicate `kar98k-free-model (1).zip` was removed;
- quarantine branch is **not** a production/runtime dependency and must never be wholesale merged into PR #94.

Binding intake specification:

`_DOCS/PASS45_ASSET_INTAKE_2026-09-03.md`

Local audit launcher:

`OsterConflict/RUN_PASS45_ASSET_INTAKE_20260903.cmd`

Auditor:

`PASS45_ASSET_INTAKE_20260903.py`

The intake includes weapon candidates such as AK-74M, AR-15/M4A1 variants, M1911, FN Ballista, Kar98k, M72, Makarov, RPG-26, shotgun and Tommy Gun, plus world candidates such as a five-storey post-Soviet building, fences, light/telephone poles and small shop/household props.

No raw downloaded ZIP is accepted merely because it exists. Unknown provenance/license remains fail-closed.

## Binding reuse-first / legal / quality sequence

For every candidate:

`quarantine -> SHA/duplicate/safe-ZIP audit -> model/texture/rig/animation inventory -> exact provenance/license/public-repo permission -> quality audit -> REJECT/DONOR_ONLY/PRODUCTION_CANDIDATE -> selective SOURCE_ASSETS promotion -> individual third-party register record -> isolated UE 5.8 proof -> integration -> cutover -> obsolete-owner cleanup -> regression/runtime acceptance`

Unknown source/license or unsafe archive blocks promotion.

“Looks like Oster” allows candidate intake only. Evidence-bound landmarks retain their reference/geo requirements. Generic fences, poles and support props may become modular kits only after style/performance acceptance.

## Weapon runtime cadence — clarified 2026-09-03

Do **not** run the expensive full UE 5.8 runtime acceptance after every tiny weapon tweak.

Required cadence:

1. use bounded source/import/fresh-load/animation/audio checks while each weapon is configured;
2. finish the intended weapon setup set;
3. then run **one consolidated current-head weapon runtime acceptance** across that configured set;
4. retain direct visual/audio/gameplay evidence;
5. only factual accepted results may close checklist items.

This changes test cadence only. It does not lower acceptance requirements.

New/optional candidates such as M72 and RPG-26 do not preempt the first mandatory open gate, item 16, and do not silently become new checklist blockers.

## Checklist binding

The asset intake remains inside the frozen 36-item PASS45 architecture:

- item 16 — existing M700/Remington870/Lever manual-action gaps;
- item 18 — accepted weapon ADS/presentation;
- item 20 — exact accepted per-weapon audio;
- item 28 — M2 assembly when an acceptable M2 source exists;
- item 32 — reusable world/environment fidelity assets;
- item 35 — final current-head runtime acceptance.

No checklist items 37+ are created.

## Next factual operation

**Do not run the full weapon runtime yet.**

First run the local candidate audit:

`OsterConflict\RUN_PASS45_ASSET_INTAKE_20260903.cmd`

Expected result: JSON + Markdown reports under ignored `PC_TEST/TEST_RESULTS/`, with archive SHA-256, duplicate detection, safe-ZIP status, model/texture/animation/license clues and candidate status.

Then:

1. resolve exact provenance/license for each candidate;
2. reject unsafe/unusable/legally unclear payloads;
3. promote only selected assets into `SOURCE_ASSETS/PASS45/...`;
4. create per-item third-party register records;
5. perform isolated UE 5.8 import/fresh-load checks;
6. continue first-open item 16 and the agreed weapon setup batch;
7. after weapon setup is complete, run one consolidated current-head weapon runtime acceptance;
8. keep PR #94 OPEN / UNMERGED until all required runtime/hygiene gates pass.

Current formal state:

```text
official_progress=22/36=61.1%
remaining=38.9%
item16_checked=0
runtime_acceptance=0
merge_permitted=0
```
