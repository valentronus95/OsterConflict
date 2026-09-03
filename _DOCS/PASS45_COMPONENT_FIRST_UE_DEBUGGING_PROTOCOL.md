# PASS45 Component-First UE 5.8 Debugging Protocol

Status: **BINDING for PASS45 local-only UE debugging**.

Purpose: stop using the user's PC as a manual CI loop and prevent repeated execution of already-passing phases after a narrow runtime/tooling failure.

## 1. Core rule

When a fail-closed local UE chain stops at one component, that component becomes the only local rerun target until it passes on the current source head.

Do **not** ask the user to rerun earlier phases that already passed merely because source code for the failed component changed.

Example:

```text
M700 PASS
Remington PASS
Lever FAIL
```

The next local run is **Lever only**, not M700 -> Remington -> Lever again.

## 2. Mandatory preflight before asking for another local UE run

Before any new local execution request, exhaust everything that can be checked remotely or statically:

1. current branch / PR / head reconciliation;
2. exact source wrapper and base-pilot assertions;
3. current UE 5.8 API usage;
4. frame-rate, frame-count, key-count and resampling-grid arithmetic;
5. motion duration versus technical sequence-envelope duration;
6. async asset-compilation / DDC barriers and teardown ordering;
7. launcher return-code and required-marker contracts;
8. stale verifiers and historical literal expectations that could fail before or after the changed code;
9. relevant exact-head CI.

A source fix is not a reason by itself to ask for another user run. The source/preflight boundary must first be internally consistent.

## 3. Three test levels

### Level A — static / remote preflight

Run first. Expected cost: seconds/minutes, no local UE interaction.

Includes source verifiers, CI contracts, invariant arithmetic and stale-rule checks.

### Level B — single-component UE proof

Run only the component that most recently failed.

For item 16 the existing component launchers are authoritative:

- M700: `OsterConflict/TRY_PASS45_M700_DERIVED_BOLT_TRANSLATION_UE58_PILOT.cmd`
- Remington 870: `OsterConflict/TRY_PASS45_REMINGTON870_DERIVED_PUMP_UE58_ASSEMBLY_AUDIT.cmd`
- Lever Action: `OsterConflict/TRY_PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT.cmd`
- audio: `OsterConflict/PASS45_IMPORT_MANUAL_ACTION_AUDIO_UE58.cmd`
- calibration review: `OsterConflict/REVIEW_PASS45_ITEM16_M700_LEVER_CALIBRATION.cmd`

Do not replace a component proof with the whole chain while that component remains unstable.

### Level C — consolidated chain / gameplay acceptance

Only after every affected component has individually passed on the current head:

1. run `OsterConflict/RUN_PASS45_ITEM16_LOCAL_UE58_EVIDENCE.cmd` once to prove the complete bounded item-16 evidence chain;
2. perform required manual current-head UE 5.8 calibration/production authoring;
3. finish the intended weapon setup batch;
4. run one consolidated full gameplay/runtime acceptance.

The full gameplay route is an acceptance gate, not a Python/asset-authoring debugger.

## 4. Current item-16 checkpoint — 2026-09-03

Latest factual local chain result:

- M700 phase: **PASS — bounded translation proof only**;
- Remington 870 phase: **PASS — derived pump/assembly evidence only**;
- Lever Action: frame-grid recovery reached `compat_frames=52`, `source_frames=26`, and cleared the pre-sampling asset-compilation barrier;
- Lever then failed on the stale base-pilot duration assertion: expected motion `0.85`, actual legal technical sequence envelope `0.866666...`;
- audio phase: not reached;
- calibration-review phase: not reached.

Current source recovery separates factual motion duration from the UE 5.8 technical envelope:

```text
motion_duration=0.85 s
motion_end_frame=51 @ 60 fps
sequence_frames=52
sequence_duration=0.866666... s
tail_pad_frames=1
resampled_initial_30fps_frames=26
```

The compatibility shim temporarily arms the base duration validation for the legal sequence envelope and restores the factual `0.85 s` motion contract before sampling/evidence output.

Therefore the next local UE operation, after exact-head preflight is green, is **Lever only**:

`OsterConflict/TRY_PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT.cmd`

Do not rerun the five-phase chain until this Lever-only proof passes.

## 5. User interaction boundary

Repository-side work belongs to the assistant when connected GitHub tooling permits it.

The user should only be asked for genuinely local-only actions such as:

- GitHub Desktop `Fetch origin` / `Pull origin` when the local worktree must receive the remote head;
- double-clicking the one required local `.cmd` launcher;
- direct visual/audio observation in UE.

Do not offload multi-command Git procedures or repeated successful phases to the user.

## 6. Acceptance accounting

This protocol changes test cadence only. It does not alter PASS45 completion accounting.

Until factual current-head UE 5.8 acceptance closes item 16:

```text
official_progress=22/36=61.1%
item16_checked=0
runtime_acceptance=0
merge_permitted=0
```

PR #94 remains OPEN / UNMERGED.