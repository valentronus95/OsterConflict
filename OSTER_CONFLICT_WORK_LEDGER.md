# OSTER CONFLICT — WORK LEDGER

> Current authoritative work state. Latest explicit user requirement + latest factual local UE runtime/build evidence always override older source/verifier claims.

The previous complete ledger remains preserved by Git history. This live ledger stays compact so new sessions resume from the current factual blocker instead of replaying completed work.

## 1. Current context — 2026-09-03

- Repository: `valentronus95/OsterConflict`.
- Integrated `main` baseline: `bca00f4046700f383af9f1742cc24b6a62401b1a`.
- Active corrective branch: `fix/pass45-runtime-rejection-material-closure-20260826`.
- Active PR: **#94 OPEN / UNMERGED**.
- Canonical active TZ: `PASS45_RUNTIME_RECOVERY_TZ.md`.
- Pass 45 remains the ACTIVE corrective pass.
- UE target: 5.8.x / Windows.
- Formal checklist accounting remains **22/36 = 61.1% complete, 38.9% remaining**.
- First factual open checklist item remains **item 16**.
- Local user `Changes`, including local `PC_TEST/TEST_RESULTS` evidence ZIPs, remain outside assistant mutation scope.
- Latest explicit user instruction: **do not require/request PC-side checks; continue PASS45 remotely from repository/CI/checkpoint state**.
- PR #94 must not merge until current-head UE 5.8 runtime acceptance passes required automated and direct visual/audio gates.

Current status token:

**PASS45 ACTIVE / ITEM16 OPEN / M700 BOUNDED UE58 PROOF PASS / REMINGTON BOUNDED UE58 PUMP+ASSEMBLY PROOF PASS / LEVER 52@60 INTEGRAL RESAMPLE GRID + PRE-SAMPLING COMPILATION BARRIER FACTUALLY PASS / CURRENT LEVER DURATION-ENVELOPE BRIDGE CODED_UNTESTED LOCALLY / MANUAL-ACTION AUDIO PLAYBACK DISPATCH INSTRUMENTED / EXACT CURRENT PLAYBACK OBJECT IDENTITY PINNED FOR M700+870+LEVER / WRONG-SOUND ADVERSARIAL CASE REJECTED / EXACT-HEAD PR CI GREEN / USER LOCAL CHECKS PAUSED / FULL GAMEPLAY DEFERRED / PR94 UNMERGED**

## 2. Status rules

- `IN_PROGRESS` — implementation/content closure is incomplete.
- `CODED_UNTESTED` — source correction exists but current factual local UE build/runtime has not accepted it.
- `CONTENT GAP` — required production content is absent/unverified; never fake READY.
- `AUDIO CONTENT GAP` — routing exists but accepted authored sound is absent/unverified.
- `RUNTIME REJECTED` — factual local gameplay or UE execution disproved the tested result.
- `VERIFIED BUILD` — factual local UBT/UE build succeeds on the tested head.
- `VERIFIED RUNTIME` — factual current-head UE/user playtest proves behavior/appearance.
- Green GitHub/source CI is structural evidence only, not UE runtime acceptance.
- Older runtime evidence remains factual for the head it tested, but cannot prove a later recovery head.
- Historical verifiers may preserve factual rejected evidence but may not resurrect retired owners or stale runtime behavior.

## 3. Historical local build/import rejection — retained regression evidence

This historical fact remains required non-regression evidence and is not the current item-16 blocker.

**LOCAL UE BUILD REJECTED — 2026-08-25.** Local UE 5.8.1 / MSVC rejected the tactical-road `FVector2D` table with **C2131** when it was authored as `constexpr`. The source fix changed that table to ordinary `const` while retaining `UE_ARRAY_COUNT`.

The same local recovery cycle also exposed the deprecated Interchange property **`auto_detect_mesh_type`** for HMMWV/M2 GLB intake. Current source instead forces the static-mesh type and disables skeletal import through the supported UE 5.8 policy.

Historical status for those source corrections remains **CODED_UNTESTED** unless a later factual local build/import result explicitly verifies that exact recovery.

## 4. Latest direct local item-16 UE truth — 2026-09-03

### Phase 1/5 — M700

**PASS — bounded BOLT translation proof only.**

UE 5.8 preserved the factual `BOLT` joint and non-trivial bounded translation. Final bolt travel, bolt rotation, production cutover and runtime acceptance remain pending.

### Phase 2/5 — Remington 870

**PASS — derived pump + imported assembly evidence only.**

UE 5.8 preserved the derived pump proof and recorded the imported Remington assembly shape. Production visual completeness and direct gameplay/audio acceptance remain pending.

### Phase 3/5 — Lever Action

The run proved:

```text
PASS45_LEVERACTION_UE58_RESAMPLE_GRID_READY initial_fps=30 compat_fps=60 compat_frames=52 source_frames=26 motion_end_frame=51 tail_pad_frames=1
PASS45_LEVERACTION_UE58_ASSET_COMPILATION_BARRIER_BEGIN stage=after_set_bone_track_keys_before_sampling
PASS45_LEVERACTION_UE58_ASSET_COMPILATION_BARRIER_END stage=after_set_bone_track_keys_before_sampling
```

It then stopped on the old duration assertion:

```text
expected=0.85
actual=0.8666666746139526
```

Current source correctly separates factual `0.85 s` motion duration from the legal `52/60 = 0.866666... s` technical sequence envelope and restores `0.85 s` before sampling/evidence. That bridge remains **CODED_UNTESTED** locally.

### Phases 4/5 and 5/5

Not reached in the latest factual local run. No new local audio/calibration verdict may be inferred from source CI.

## 5. Item 16 current production boundary

Item 16 still requires accepted authored moving-part/manual-action presentation and factual mechanical audio for M700, Remington 870 and Lever Action plus current-head UE 5.8 acceptance.

### Remington 870

Production source remains:

- `/Game/Production/Weapons/Remington870/SKM_Remington870.SKM_Remington870`;
- `/Game/Production/Weapons/Remington870/AN_Remington870_PumpCycle.AN_Remington870_PumpCycle`.

The bounded local assembly proof passes, but direct current-head visible-pump and mechanical-audio gameplay acceptance remain pending.

### M700

- factual weighted `BOLT` moving part exists;
- bounded translation proof passes locally;
- `BOLT_STOP` is not an accepted travel endpoint;
- final travel + bolt rotation still require direct current-head visual calibration before production authoring/cutover.

### Lever Action

- factual `LEVER` moving part exists;
- 52-frame UE 5.8 technical envelope and pre-sampling barrier have factual local evidence;
- current `-45°` excursion remains calibration-only;
- final lever angle remains unaccepted.

The eventual production-authoring boundary remains **MANUAL CURRENT-HEAD UE 5.8 VISUAL CALIBRATION**. The user's current pause on PC checks does not convert pending evidence into acceptance.

## 6. Current manual-action audio evidence hardening

Current source now emits a factual result from the real `HandleStateEventLocal()` playback path:

```text
PASS45_MANUAL_ACTION_AUDIO_PLAYBACK_DISPATCHED
weapon=...
action=...
sound=...
route=local2d
bus_gt_zero=1
effective_volume_gt_zero=1
second_gameplay_timer=0
runtime_acceptance=0
```

Failure to dispatch a usable sound emits `PASS45_MANUAL_ACTION_AUDIO_PLAYBACK_FAIL`, which the runtime verifier rejects.

The runtime gate no longer accepts generic `sound=/Game/` evidence. It requires the exact current expected playback object:

- `OC_SNP1` → `/Game/PASS45/Audio/ManualAction/SW_PASS45_BoltAction_CC0_Donor.SW_PASS45_BoltAction_CC0_Donor`;
- `OC_SG1` → `/Game/R13/Audio/shotguncock.shotguncock`;
- `R13_LEVER4570` → `/Game/PASS45/Audio/ManualAction/SW_PASS45_LeverAction_CC0_Donor.SW_PASS45_LeverAction_CC0_Donor`.

The static guard binds those runtime expectations to the actual source `LoadSound()` object paths. The executable adversarial contract proves that a wrong M700 playback object is rejected.

These are exact current playback-object identities. They are **not** a claim that the M700/Lever donor recordings are exact real-weapon recordings, and they are not direct audible-quality acceptance.

Relevant commits:

- `b389c21ac64b01ef9862662e87b0fc85a47382e9` — playback dispatch/failure instrumentation;
- `119b18829e5f252392fbd0e3d0999279f83e4ae2` — runtime playback evidence requirement;
- `b7b5aeff9fddb6d8830cad196ab0bc1b8e06ade4` — static dispatch/positive-volume guard;
- `db9e252c33223f944b86b41165599d7601c2b664` / `3dc8cdffff6f8a92aee050bb75017438a19d05b5` — executable playback contract and real missing-marker adversarial case;
- `e59f672286279377ee6a22779f3c9c9e8a7721cf` — exact runtime playback-object binding;
- `924867a156a47de3782b668fc6a94500ea298364` — wrong M700 sound adversarial rejection;
- `46c400c228661a4200248ae35bb819c420ce5a2d` — source/runtime exact sound identity binding.

## 7. Exact-head acceptance integrity

`RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd` already:

1. resolves and pins current `git HEAD` before runtime;
2. rejects tracked unstaged or staged changes before runtime;
3. executes the actual playflow/runtime route;
4. resolves HEAD again after runtime/material gates and rejects head drift;
5. rejects tracked mutations produced during runtime/import stages;
6. only then executes strict evidence verifiers, including the M700/870/Lever manual-action verifier.

Therefore a stale arbitrary working-tree log is not accepted as an exact-head runtime result by the strict main route. This source audit does not replace the deferred actual runtime execution.

## 8. Binding component-first cadence and local execution boundary

`_DOCS/PASS45_COMPONENT_FIRST_UE_DEBUGGING_PROTOCOL.md` and `AGENTS.md` rules 30–31 remain binding.

User-local execution is currently paused by explicit instruction. Therefore:

- do not send the user to GitHub Desktop, CMD, UE Editor or local launchers as the active next step;
- continue assistant-owned GitHub/source/CI work whenever technically possible;
- local runtime/visual evidence may remain pending, but must never be fabricated or promoted from CI;
- do not merge PR #94 while runtime acceptance remains pending;
- local execution can return to the queue only if the user later explicitly resumes it.

The `asset-intake-20260903` branch remains quarantine-only and must never be merged wholesale. No checklist items 37+ are created from quarantine inventory.

## 9. Current exact-head CI truth

Canonical source checkpoint before this ledger update: `46c400c228661a4200248ae35bb819c420ce5a2d`.

GitHub returned the full pull-request workflow matrix for that exact head as **completed / SUCCESS**. Relevant item-16 gates include:

- `Pass 45 manual-action audio provenance`: SUCCESS, including provenance, third-party register, playback-dispatch source contract and executable runtime contract;
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

Green exact-head CI proves the repository contracts are internally consistent. It does **not** prove direct UE visual/audio acceptance.

## 10. Next factual operation

The active operation remains remote, not user-local:

1. audit and, if necessary, harden the same-transition relationship between authoritative manual-action state, mechanical-audio playback dispatch and authored moving-part presentation without adding a second gameplay timer;
2. keep final M700 travel/rotation and Lever angle unaccepted until factual visual calibration exists;
3. keep Remington visible-pump/mechanical-audio acceptance pending factual current-head gameplay evidence;
4. preserve one later consolidated runtime acceptance instead of repeated full-game launches;
5. do not merge PR #94 and do not promote runtime acceptance from CI alone.

Do **not** run or request `START_HERE.cmd -> 2. ПОВНИЙ RUNTIME-ТЕСТ` now.

## 11. Protected merge/accounting state

- PR #94: **OPEN / UNMERGED**.
- Item 16: **UNCHECKED**.
- Official checklist: **22/36 = 61.1% complete**.
- Remaining: **38.9%**.
- `runtime_acceptance=0`.
- `item16_checked=0`.
- `merge_permitted=0`.
- `user_local_execution_requested=0`.
- Local user `Changes`: **DO NOT TOUCH**.