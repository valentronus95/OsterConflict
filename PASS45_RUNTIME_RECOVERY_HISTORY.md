# PASS45 Runtime Recovery — Persistent Work History

This is the current human-readable checkpoint index for `PASS45_RUNTIME_RECOVERY_TZ.md`. Git history remains the raw source of truth; this file records the newest factual continuation state so new chats do not replay completed analysis.

## Binding continuation and local-debug rules — 2026-09-03

Canonical continuation protocol:

`_DOCS/PASS45_CHECKPOINT_CONTINUATION_PROTOCOL.md`

Canonical component-first UE debugging protocol:

`_DOCS/PASS45_COMPONENT_FIRST_UE_DEBUGGING_PROTOCOL.md`

`AGENTS.md` rules 30–31 are binding:

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
11. `37361f61c6afa33ab094490f10ae74c35de3cceb` — component-first UE debugging protocol;
12. `23ac9d1d8cc3677a9d4f9a56cbce63f72d4e91fc` — `AGENTS.md` rule 31 binds component-first local UE debugging;
13. `ed37f3378275a0346af745c2b2889d36907ca714` — live ledger updated to the latest Lever-only boundary and historical build/import regression markers.

## First factual open item — item 16

Item 16 still requires accepted authored M700 / Remington 870 / Lever Action moving-part/manual-action presentation, factual mechanical audio and current-head UE 5.8 acceptance.

Source/docs/CI/pilot-only work does not check item 16 and does not increase the official percentage.

### Remington 870

Current source has the guarded registered CC-BY-4.0 donor derivative, production skeletal path and PumpCycle bridge. The older 2026-09-02 gameplay result rejected only the pre-cutover presentation where the fore-end did not visibly pump.

Latest bounded local evidence now passes the derived pump + imported assembly proof, but direct current-head visible-pump and mechanical-audio gameplay acceptance remain pending.

### M700

The Stein CC0 source has a factual weighted `BOLT` joint. `BOLT_STOP` is not accepted as an authored travel endpoint.

Latest bounded local evidence passes the M700 translation proof. Final bolt travel and bolt rotation remain manual current-head visual calibration work before production authoring/cutover.

### Lever Action

The Stein CC0 source has a factual addressable `LEVER` bone. The current `-45°` local-X excursion remains calibration-only.

## Latest factual local item-16 run — 2026-09-03

The newest supplied five-phase chain advanced as follows:

### 1/5 M700

**PASS — bounded translation proof only.**

### 2/5 Remington 870

**PASS — derived pump + assembly evidence only.**

### 3/5 Lever Action

The 52-frame frame-grid correction factually worked far enough to emit:

```text
PASS45_LEVERACTION_UE58_RESAMPLE_GRID_READY initial_fps=30 compat_fps=60 compat_frames=52 source_frames=26 motion_end_frame=51 tail_pad_frames=1
PASS45_LEVERACTION_UE58_ASSET_COMPILATION_BARRIER_BEGIN stage=after_set_bone_track_keys_before_sampling
PASS45_LEVERACTION_UE58_ASSET_COMPILATION_BARRIER_END stage=after_set_bone_track_keys_before_sampling
```

The previous `frame remainder of 0.50000000` compression assertion did not remain the current blocker.

The base pilot then rejected the intentional technical tail pad:

```text
PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT_FAIL pilot_sequence_duration_mismatch=1 expected=0.85 actual=0.8666666746139526
ERROR: Lever Action UE 5.8 pilot failed with code -1.
ERROR: item-16 evidence chain stopped at Lever Action. rc=7
```

This is a stale validation-contract mismatch:

```text
factual_motion_duration=0.85 s
motion_end_frame=51 @ 60 fps
technical_sequence_frames=52
technical_sequence_duration=0.866666... s
tail_pad_frames=1
initial_30fps_resampled_frames=26
```

Phases 4/5 audio and 5/5 calibration review were not reached.

## Current Lever source recovery

The compatibility shim now arms the base duration validation for the legal technical envelope only after the factual 0.85 s motion keys are authored. Before sampling/evidence/PASS output it restores `0.85 s` as the authoritative motion duration.

Required source markers include:

```text
PASS45_LEVERACTION_UE58_SEQUENCE_ENVELOPE_CONTRACT_ARMED
PASS45_LEVERACTION_UE58_MOTION_DURATION_RESTORED
```

The regression guard requires:

- 52 sequence frames / 53 keys;
- 26 integral source frames on the initial 30 fps grid;
- motion endpoint frame 51 at exactly 0.85 s;
- one bind-pose tail frame;
- `add_bone_curve()` + `set_bone_track_keys()`;
- pre-sampling and post-pilot compilation barriers;
- padded-envelope validation bridge;
- no production cutover or acceptance promotion.

Current recovery is **CODED_UNTESTED** locally until a Lever-only UE 5.8 proof passes.

## Historical 2026-08-25 build/import non-regression

Retain the factual historical **LOCAL UE BUILD REJECTED** evidence:

- UE 5.8.1 / MSVC `C2131` rejected the tactical `FVector2D` road table when it was `constexpr`;
- UE 5.8 Interchange rejected deprecated `auto_detect_mesh_type` usage for HMMWV/M2 intake.

These are historical regression guards, not current item-16 blockers. Current source fixes remain recorded as `CODED_UNTESTED` unless later factual local build/import evidence explicitly verifies them.

## Current CI boundary

On `48d097d801e4e515bec9d728d59a6eec55f758c0`:

- item-16 frame-rate compatibility: SUCCESS;
- item-16 local evidence-chain source contract: SUCCESS;
- M700/Remington/Lever related narrow source contracts: SUCCESS where present;
- two wider workflows failed because the compact live ledger had dropped the historical `LOCAL UE BUILD REJECTED`, `C2131` and `auto_detect_mesh_type` markers required by `VERIFY_PASS45_LOCAL_BUILD_IMPORT_REGRESSION.py`.

The ledger now restores those factual historical markers. Do not claim full exact-head CI green until the new head settles.

## Next factual operation

Do **not** run full gameplay.

Do **not** rerun the five-phase item-16 chain while Lever remains the only unstable component.

First settle exact-head source/preflight CI on the current documentation/process head. Then the next user-local operation is exactly the existing Lever component launcher:

`OsterConflict/TRY_PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT.cmd`

The Lever-only proof must show:

1. integral 52-frame / 26-source-frame grid marker;
2. pre-sampling compilation barrier PASS;
3. sequence-envelope contract armed;
4. factual 0.85 s motion contract restored;
5. no duration mismatch;
6. normal Lever pilot PASS;
7. post-pilot compilation barrier PASS.

Only after Lever-only PASS should `OsterConflict/RUN_PASS45_ITEM16_LOCAL_UE58_EVIDENCE.cmd` run once to exercise all five bounded phases. After that, proceed to manual M700/Lever visual calibration, accepted production authoring/cutover and one consolidated full weapon runtime acceptance.

PR #94 remains OPEN / UNMERGED.

```text
official_progress=22/36=61.1%
remaining=38.9%
item16_checked=0
runtime_acceptance=0
merge_permitted=0
```