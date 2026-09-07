# PASS45 UE 5.8 Debugging Protocol

Date updated: 2026-09-04  
Status: **BINDING for local-only UE debugging and runtime handoff**

## Core rule

The user's PC is not a rolling CI machine.

Do not request local UE after each weapon, model, animation, sound or graphics change.

Prepare the broad PASS45 package remotely first. Use local UE only when:

1. the integrated batch is worth evaluating end-to-end;
2. a genuinely local-only fact blocks safe remote work across the remaining batch; or
3. the user explicitly asks for a targeted check.

## Normal development cadence

`remote/source/content work -> critical affected checks -> next remote-preparable work -> integrated local session later`

Not:

`edit -> local test -> edit -> local test -> edit -> local test`

## Integrated acceptance target

The planned local UE 5.8 session should cover as much as safely prepared of:

- weapon models and mechanics;
- weapon shot/reload/mechanical audio;
- first-person hands/arms and weapon-hand presentation;
- ADS/sights;
- grenade/ordnance presentation;
- HMMWV/M2/BTR presentation/gameplay;
- vegetation/environment;
- world/material/LOD/graphics quality;
- tactical map/display/performance.

Collect one consolidated defect list.

Then:

1. fix defects remotely as a batch;
2. rerun only the components that actually failed when targeted debugging is useful;
3. perform one final integrated acceptance.

## Targeted component launchers

Existing item-16 local launchers remain available as diagnostics, not as mandatory development steps:

- M700: `OsterConflict/TRY_PASS45_M700_DERIVED_BOLT_TRANSLATION_UE58_PILOT.cmd`
- Remington 870: `OsterConflict/TRY_PASS45_REMINGTON870_DERIVED_PUMP_UE58_ASSEMBLY_AUDIT.cmd`
- Lever Action: `OsterConflict/TRY_PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT.cmd`
- manual-action audio import: `OsterConflict/PASS45_IMPORT_MANUAL_ACTION_AUDIO_UE58.cmd`
- calibration review: `OsterConflict/REVIEW_PASS45_ITEM16_M700_LEVER_CALIBRATION.cmd`
- integrated item-16 evidence chain: `OsterConflict/RUN_PASS45_ITEM16_LOCAL_UE58_EVIDENCE.cmd`

Run these before the broad integrated session only when a specific local-only fact is a real cross-batch blocker or when correcting a component that already failed.

## Current item-16 truth

- M700 bounded translation preparation exists; final travel/rotation is deferred local visual calibration.
- Remington 870 pump source/sequence preparation exists; direct visible pump/gameplay acceptance is deferred.
- Lever Action 0.85 s motion contract and UE 5.8 compatibility preparation exist; final lever angle is deferred local visual calibration.
- manual-action audio routing/source preparation exists; exact UE import/fresh-load/audibility must remain factual.

Item 16 stays open, but it does not freeze later remote-preparable PASS45 items.

## Verification rule

Before a local request, use only the critical checks needed to establish that the prepared package is safe enough to test.

Do not require all historical calibration/checkpoint/verifier-contract scripts to run automatically after every small change.

Historical diagnostics may be run manually when investigating the exact regression they own.

## Runtime truth

Never convert source/CI green into a fake local pass.

Until factual integrated current-head acceptance:

```text
official_progress=22/36=61.1%
runtime_acceptance=0
item16_checked=0
merge_permitted=0
```

PR #94 stays OPEN / UNMERGED.

## User handoff

When no local action is needed:

`ВІД ТЕБЕ ЗАРАЗ НІЧОГО НЕ ПОТРІБНО.`

When local UE becomes a real blocker:

`ПОТРІБНА ТВОЯ ПЕРЕВІРКА.`

The handoff must name `Oster Conflict / PASS45`, describe one integrated action where possible, and state exactly what evidence/result to return.