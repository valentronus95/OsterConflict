# PASS45 Component-First UE 5.8 Debugging Protocol

Status: **BINDING for PASS45 local-only UE debugging and user runtime handoff**.

Purpose: stop using the user's PC as a manual CI loop, prevent repeated execution of already-passing phases, and reserve user gameplay checks for a consolidated weapon acceptance window rather than per-weapon development churn.

## 1. Core rule — remote first, user runtime in batches

The user's PC is **not** the first test environment and is **not** the default next step after each source change.

Before requesting any local UE action, exhaust all work that can be done through repository/source/static/CI tooling. A pending local visual/audio check does not freeze the whole PASS45 queue when other remote-preparable work in the same acceptance batch can continue safely.

For the weapon block, do **not** ask the user to test M700, Remington 870, Lever Action or another individual weapon merely because that one component reached a local-only acceptance boundary. Prepare the intended weapon set as far as possible first, then perform one consolidated current-head UE 5.8 weapon gameplay/runtime acceptance session.

If a fail-closed consolidated/local chain later stops at one component, that failed component becomes the only rerun target while fixing that specific failure. Do not make the user repeat earlier phases that already passed.

Example after a consolidated run:

```text
M700 PASS
Remington PASS
Lever FAIL
```

The corrective rerun is **Lever only**, not M700 -> Remington -> Lever again.

## 2. Mandatory preflight before any user-local request

Before asking the user to launch anything locally, exhaust everything that can be checked remotely or statically:

1. current branch / PR / head reconciliation;
2. exact source wrapper and base-pilot assertions;
3. current UE 5.8 API usage;
4. frame-rate, frame-count, key-count and resampling-grid arithmetic;
5. motion duration versus technical sequence-envelope duration;
6. async asset-compilation / DDC barriers and teardown ordering;
7. launcher return-code and required-marker contracts;
8. stale verifiers and historical literal expectations that could fail before or after the changed code;
9. relevant exact-head CI;
10. all other remote-preparable weapon/source/content work that can be completed before the same consolidated acceptance session.

A source fix is not a reason by itself to ask for another user run.

## 3. Three test levels

### Level A — static / remote preflight

Run first. Expected cost: seconds/minutes, no local UE interaction.

Includes source verifiers, CI contracts, invariant arithmetic, provenance/content checks and stale-rule checks.

### Level B — targeted component UE proof

This is **not** the default user step during ordinary weapon preparation.

Use a targeted component proof only when one of these is true:

- a consolidated local/runtime run already identified that exact component as the failure;
- a genuinely local-only UE fact is a hard blocker that prevents any further safe remote preparation;
- the user explicitly asks to test that component now.

For item 16 the existing component launchers remain available for such targeted recovery:

- M700: `OsterConflict/TRY_PASS45_M700_DERIVED_BOLT_TRANSLATION_UE58_PILOT.cmd`
- Remington 870: `OsterConflict/TRY_PASS45_REMINGTON870_DERIVED_PUMP_UE58_ASSEMBLY_AUDIT.cmd`
- Lever Action: `OsterConflict/TRY_PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT.cmd`
- audio: `OsterConflict/PASS45_IMPORT_MANUAL_ACTION_AUDIO_UE58.cmd`
- calibration review: `OsterConflict/REVIEW_PASS45_ITEM16_M700_LEVER_CALIBRATION.cmd`

Do not turn these component launchers into a mandatory per-weapon ritual before the overall weapon batch is ready.

### Level C — consolidated weapon chain / gameplay acceptance

After the intended weapon setup batch is remotely ready as far as repository/source/CI work permits:

1. run the bounded item-16 evidence chain only if it is still needed by the current acceptance contract;
2. perform the required current-head UE 5.8 visual/audio/calibration checks for all affected weapons in the same local session/window where practical;
3. collect one consolidated defect list;
4. fix that defect list as a batch;
5. rerun only failed components during corrective debugging;
6. perform one final consolidated full weapon gameplay/runtime acceptance after corrections.

The full gameplay route is an acceptance gate, not a Python/asset-authoring debugger.

## 4. Current item-16 checkpoint — 2026-09-03

Latest factual local evidence remains:

- M700 phase: **PASS — bounded translation proof only**; final bolt travel/rotation still unaccepted;
- Remington 870 phase: **PASS — derived pump/assembly evidence only**; direct visible pump/mechanical-audio gameplay acceptance still pending;
- Lever Action reached the UE 5.8 integral resampling recovery (`compat_frames=52`, `source_frames=26`) and cleared the pre-sampling asset-compilation barrier; the old local chain then stopped on the stale base-pilot duration assertion;
- current source recovery separates factual `0.85 s` Lever motion from the legal `52/60 = 0.866666... s` transient sequence envelope and restores the factual motion contract before evidence sampling.

Latest explicit user requirement supersedes the earlier instruction that made a Lever-only run the immediate next action:

- **do not request any PC-side weapon check now**;
- continue all safe remote/source/CI preparation instead;
- keep item 16 open and honest while its local acceptance is deferred;
- when the intended weapon set is ready, ask for one consolidated weapon gameplay/runtime check rather than separate checks after each weapon change.

Therefore **Lever-only is no longer the current user action**. It remains a targeted recovery tool if the later consolidated acceptance actually fails on Lever.

## 5. Deferred-acceptance continuation rule

A local-only acceptance dependency may remain open without stopping all progress.

If the first factual open checklist item has reached a point where its only remaining blocker is deferred user UE evidence, and no further safe remote work remains inside that item, the assistant must:

1. leave that checklist item unchecked;
2. record the exact deferred evidence;
3. continue the next remote-preparable item or direct dependency in the **same consolidated acceptance batch**;
4. never claim the deferred item is complete;
5. return to the deferred acceptance items together when the batch is ready.

For the current PASS45 weapon batch this allows remote preparation to continue across the weapon presentation/ADS/audio/runtime-readiness items instead of repeating the same item-16 blocker in every chat.

This rule changes execution order for efficiency, not acceptance strictness or official completion accounting.

## 6. User interaction and communication boundary

Repository-side work belongs to the assistant whenever connected GitHub tooling permits it.

Normal progress replies must be written in plain Ukrainian first: what was done, what remains, what is next, and official progress. Avoid burying the result under internal verifier names, SHA details or engine jargon; technical detail may follow only when useful.

When **no user action is required**, say plainly:

`ВІД ТЕБЕ ЗАРАЗ НІЧОГО НЕ ПОТРІБНО.`

When a user-local check becomes a genuine hard blocker, the reply must make it impossible to miss. Start the action section with:

`ПОТРІБНА ТВОЯ ПЕРЕВІРКА.`

Then explain in ordinary language:

1. why remote work cannot safely continue without it;
2. the smallest exact action the user must perform;
3. what weapons/features will be checked together;
4. what result/evidence to return.

Do not hide a required user action inside a long technical status report. Because the user works on multiple projects in parallel, always name **Oster Conflict / PASS45** explicitly in such a handoff.

## 7. Acceptance accounting

This protocol changes test cadence and continuation efficiency only. It does not alter PASS45 completion accounting.

Until factual current-head UE 5.8 acceptance closes the relevant checklist items:

```text
official_progress=22/36=61.1%
item16_checked=0
runtime_acceptance=0
merge_permitted=0
```

PR #94 remains OPEN / UNMERGED.