# PASS45 Runtime Recovery — Persistent Work History

This is the current human-readable checkpoint index for `PASS45_RUNTIME_RECOVERY_TZ.md`. Git history remains the raw source of truth; this file records the newest factual continuation state so new chats do not replay completed analysis.

## Binding continuation and local-debug rules — 2026-09-03

Canonical continuation protocol:

`_DOCS/PASS45_CHECKPOINT_CONTINUATION_PROTOCOL.md`

Canonical component-first UE debugging protocol:

`_DOCS/PASS45_COMPONENT_FIRST_UE_DEBUGGING_PROTOCOL.md`

`AGENTS.md` rules 30–31 remain binding:

- continue from the latest factual checkpoint instead of restarting a broad audit;
- after a fail-closed local chain stops at one component, exhaust source/static/preflight checks first and rerun only that failed component until it passes;
- do not make the user repeat earlier phases that already passed;
- run the full bounded chain once only after affected components are individually green;
- run full gameplay only as consolidated acceptance, not as a Python/asset-authoring debugger.

## Canonical ownership

- Canonical TZ: `PASS45_RUNTIME_RECOVERY_TZ.md`.
- Active branch: `fix/pass45-runtime-rejection-material-closure-20260826`.
- Target baseline: `main@bca00f4046700f383af9f1742cc24b6a62401b1a`.
- Active PR: **#94 OPEN / UNMERGED**.
- Official checklist: **22/36 = 61.1% complete, 38.9% remaining**.
- First factual open item: **item 16**.
- Local user `Changes` remain outside assistant mutation scope.
- User-local UE execution remains paused by explicit instruction; active work is repository/source/CI only.

## Relevant recovery chain

Historical/local blocker sequence retained for non-regression:

1. `cf75b86ce5988ef489f0ef653d3f1b3f637278fd` — factual M700 local rejection on legacy bone-track creation;
2. `89bb635d67b24afdb5e32bccd91092401b6024d6` — M700 UE 5.8 `add_bone_curve()` recovery;
3. `3dc5d1b57a6b908b0bd5356e0b01b681e397d285` — Lever UE 5.8 `add_bone_curve()` recovery;
4. `3de85c46a9c12aa9dd43a3950a888872cf266e6f` — regression guard against deprecated direct bone-track creation;
5. `7b70c56e0c1e77c6642ba517d45310d7879be343` / `2ac5b9560b63a51be3f57c770c6a93d2c302373c` — Lever/M700 asset-compilation barriers;
6. `03ab7bded49fc23ea1c19c23586b86797aaeba93` — async-compilation barrier regression guard;
7. `3b66261b79a82deed7ebe698844205176cc92b20` — Lever 52-frame integral 30→60 resampling envelope;
8. `91da695e4dbc07d2a0890e0394e93ba066bc6a92` — resampling-grid regression guard;
9. `a03b128c902b4103eabf60b7a85f779992ff24cd` — Lever motion-duration versus technical-envelope validation bridge;
10. `48d097d801e4e515bec9d728d59a6eec55f758c0` — regression guard for the padded-envelope bridge;
11. `37361f61c6afa33ab094490f10ae74c35de3cceb` / `23ac9d1d8cc3677a9d4f9a56cbce63f72d4e91fc` — component-first UE debugging protocol and binding AGENTS rule;
12. `79d6363bb7cb1174a749969d009c00b50edf98fa` / `67d3e443783beb2f3b6b080c99601e9f84b3d801` — calibration review rejects stale pre-recovery evidence and current checkpoint verifier proves fail-closed behavior;
13. `e34a41bab79bed713bac7f5c77626224aef55907` / `73497cf7c4c23e7baf9df3786f3d37593541a6da` — exact M700/Lever Stein source identity and SHA-256 binding plus negative wrong-source tests;
14. `2d2eb910716f67223626ff244a068098bf43385c` — calibration approval is bound to a real ancestor commit and rejected if calibration-critical source changes after that evidence head;
15. `b389c21ac64b01ef9862662e87b0fc85a47382e9` — manual-action audio now emits factual playback dispatch/failure evidence from the real `HandleStateEventLocal()` path;
16. `119b18829e5f252392fbd0e3d0999279f83e4ae2` / `b7b5aeff9fddb6d8830cad196ab0bc1b8e06ade4` — runtime and source contracts require local playback dispatch, positive weapon bus/effective volume and reject playback failure;
17. `db9e252c33223f944b86b41165599d7601c2b664` / `3dc8cdffff6f8a92aee050bb75017438a19d05b5` — executable synthetic runtime contract covers playback evidence and its missing-playback adversarial case is a real removal rather than a substring false positive;
18. `e59f672286279377ee6a22779f3c9c9e8a7721cf` — runtime evidence is no longer satisfied by arbitrary `sound=/Game/`; it pins the exact expected current manual-action playback object for each required weapon;
19. `924867a156a47de3782b668fc6a94500ea298364` — executable contract rejects wrong M700 manual-action sound identity;
20. `46c400c228661a4200248ae35bb819c420ce5a2d` — static audio-dispatch guard binds source `LoadSound()` objects to the exact runtime expectations and rejects source/runtime identity drift.

## First factual open item — item 16

Item 16 still requires accepted authored M700 / Remington 870 / Lever Action moving-part/manual-action presentation, factual mechanical audio and current-head UE 5.8 acceptance.

Source/docs/CI/pilot-only work does not check item 16 and does not increase the official percentage.

The eventual production-authoring boundary remains **MANUAL CURRENT-HEAD UE 5.8 VISUAL CALIBRATION** for final M700 travel/rotation and Lever angle. This acceptance boundary is not waived by source-only work.

### Remington 870

Current source has the guarded registered CC-BY-4.0 donor derivative, production skeletal path and PumpCycle bridge. The older 2026-09-02 gameplay result rejected only the pre-cutover presentation where the fore-end did not visibly pump.

Latest bounded local evidence passes the derived pump + imported assembly proof, but direct current-head visible-pump and mechanical-audio gameplay acceptance remain pending.

### M700

The Stein CC0 source has a factual weighted `BOLT` joint. `BOLT_STOP` is not accepted as an authored travel endpoint.

Latest bounded local evidence passes the M700 translation proof. Final bolt travel and bolt rotation remain manual current-head visual calibration work before production authoring/cutover.

### Lever Action

The Stein CC0 source has a factual addressable `LEVER` bone. The current `-45°` local-X excursion remains calibration-only.

The latest local run proved the 52-frame / 53-key UE 5.8 integral resampling envelope and pre-sampling compilation barrier, then stopped on the stale base-pilot duration assertion `expected=0.85 actual=0.866666...`. Current source separates the factual 0.85 s motion endpoint from the legal 52/60 technical sequence envelope and restores 0.85 s before sampling/evidence. This recovery remains **CODED_UNTESTED** locally.

## Current manual-action audio evidence contract

The source now distinguishes three different facts that must not be collapsed into one convenient green check:

1. the required action-family sound object loaded into the weapon audio profile;
2. the authoritative `bActionCycling` rising edge dispatched `ManualActionCycle` through the real audio component;
3. a non-null exact expected sound object reached the local 2D playback branch with weapon-bus volume and effective playback volume both greater than zero.

Runtime evidence requires:

```text
PASS45_MANUAL_ACTION_AUDIO_PLAYBACK_DISPATCHED
route=local2d
bus_gt_zero=1
effective_volume_gt_zero=1
second_gameplay_timer=0
runtime_acceptance=0
```

and binds the `sound=` field to the current expected object:

```text
OC_SNP1       -> /Game/PASS45/Audio/ManualAction/SW_PASS45_BoltAction_CC0_Donor.SW_PASS45_BoltAction_CC0_Donor
OC_SG1        -> /Game/R13/Audio/shotguncock.shotguncock
R13_LEVER4570 -> /Game/PASS45/Audio/ManualAction/SW_PASS45_LeverAction_CC0_Donor.SW_PASS45_LeverAction_CC0_Donor
```

These are exact **current playback-object identities**, not a claim that the M700/Lever donor recordings are exact real-weapon recordings. Direct audible quality/feel acceptance remains pending.

The runtime verifier also rejects `PASS45_MANUAL_ACTION_AUDIO_PLAYBACK_FAIL`, manual-action content gaps, authored-animation content gaps and authored source-bridge failures.

## Exact-head runtime-evidence integrity audit

The strict main acceptance wrapper already pins `git rev-parse HEAD` before runtime, rejects tracked staged/unstaged differences, reruns the source-head check after runtime/material gates, rejects HEAD drift and rejects tracked mutations created by runtime/import stages.

Therefore the manual-action verifier is executed inside an existing exact-head acceptance route rather than against an arbitrarily reusable stale working tree. This is source-contract evidence only; it still does not create the missing direct UE visual/audio acceptance.

## Historical 2026-08-25 build/import non-regression

Retain the factual historical **LOCAL UE BUILD REJECTED** evidence:

- UE 5.8.1 / MSVC `C2131` rejected the tactical `FVector2D` road table when it was `constexpr`;
- UE 5.8 Interchange rejected deprecated `auto_detect_mesh_type` usage for HMMWV/M2 intake.

These are historical regression guards, not current item-16 blockers. Current source fixes remain recorded as `CODED_UNTESTED` unless later factual local build/import evidence explicitly verifies them.

## Asset quarantine and bounded-before-consolidated runtime cadence

The `asset-intake-20260903` branch remains quarantine-only. Quarantine content is not production-ready, not runtime-accepted and must never be merged wholesale into PASS45.

The checklist architecture remains frozen at 36 canonical items: **no checklist items 37+** are created from asset intake or quarantine inventory.

Do **not** run the expensive full UE 5.8 gameplay acceptance after every small weapon tweak. Use bounded source/import/fresh-load/component checks while configuration is still moving, then run **one consolidated current-head weapon runtime acceptance** only after the intended weapon setup batch is ready.

## Explicit user-local execution boundary — 2026-09-03

Latest explicit user instruction: **do not require or request PC-side checks; continue `PASS45_RUNTIME_RECOVERY_TZ.md` from the repository/checkpoint instead.**

Therefore:

- no current next-step instruction may send the user to GitHub Desktop, CMD, UE Editor or a local launcher;
- local-only UE evidence remains pending factual evidence, not silently accepted;
- the assistant continues remote/source/CI/verifier cleanup and any item-16 preparation that does not require fabricated visual calibration;
- `runtime_acceptance=0`, `item16_checked=0`, `merge_permitted=0` remain unchanged;
- PR #94 remains OPEN / UNMERGED until the existing runtime acceptance rule is actually satisfied by factual evidence at some later point.

## Current CI boundary

Exact-head source/CI checkpoint `46c400c228661a4200248ae35bb819c420ce5a2d` is structurally green: **every pull-request workflow run returned by GitHub for this exact head completed SUCCESS**.

Relevant item-16/current-checkpoint gates include:

- `Pass 45 manual-action audio provenance`: SUCCESS, including provenance, third-party register, static playback-dispatch contract and executable runtime evidence contract;
- `Pass 45 checkpoint continuation and item16 calibration contract`: SUCCESS;
- `Pass 45 item16 production profile cutover`: SUCCESS;
- `Pass 45 item16 production cutover preflight`: SUCCESS;
- `Pass 45 item16 UE58 frame-rate compatibility`: SUCCESS;
- `Pass 45 item16 local UE58 evidence chain contract`: SUCCESS;
- `Pass 45 M700 derived bolt translation UE58 pilot contract`: SUCCESS;
- Remington source/import/derived-pump/assembly/production wiring contracts: SUCCESS;
- `Pass 45 strict runtime acceptance harness`: SUCCESS;
- `Main runtime acceptance launcher contracts`: SUCCESS;
- `Source verification`: SUCCESS.

This means the current remote/source contract set is internally consistent. It does **not** convert the locally untested Lever duration bridge, missing final calibration, or direct gameplay/visual/audio acceptance into runtime success. GitHub CI remains structural evidence only.

## Next factual operation

User-local execution is currently paused by explicit instruction.

Continue remotely in this order:

1. keep exact-head CI/source contracts green while hardening item-16 evidence;
2. audit the same-transition relationship between authoritative manual-action state, audio playback dispatch and authored moving-part presentation without inventing timing tolerances or claiming audible quality;
3. keep final M700 travel/rotation and Lever angle unaccepted until factual visual calibration exists;
4. keep Remington visible-pump/mechanical-audio acceptance pending factual runtime evidence;
5. do not merge PR #94 and do not promote runtime acceptance from CI alone.

PR #94 remains OPEN / UNMERGED.

```text
official_progress=22/36=61.1%
remaining=38.9%
item16_checked=0
runtime_acceptance=0
merge_permitted=0
user_local_execution_requested=0
```