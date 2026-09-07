# PASS45 Checkpoint Continuation Protocol

Date updated: 2026-09-04  
Scope: `PASS45_RUNTIME_RECOVERY_TZ.md`  
Status: **BINDING**

## Goal

Continue PASS45 from the newest factual checkpoint without restarting analysis, without stopping on deferred local UE evidence, and without spending most of the cycle proving the same source fact several times.

## Continuation sequence

1. Reconcile only:
   - canonical branch;
   - current HEAD;
   - PR #94 head/base/state;
   - commits since the recorded checkpoint;
   - relevant batch-level CI conclusion.
2. Read the latest section of `PASS45_RUNTIME_RECOVERY_HISTORY.md`.
3. Read only the compact TZ sections required by the next remote-preparable batch.
4. Read targeted ledger/reference/provenance entries only when that batch needs them.
5. Consume newer parallel-chat commits and continue from them. Never replay superseded work.
6. Freeze already accepted/source-closed work unless newer code/runtime evidence invalidates it.
7. If the earliest open checklist item is blocked only by deferred local UE evidence, leave it open and continue the next safe remote-preparable item.
8. Stop only at a genuine **remote** blocker, a meaningful checkpoint, or completion of the prepared integrated batch.

## Batch target

The user has explicitly chosen broad batch-first verification.

Before asking for the planned local UE 5.8 session, continue as far as safely possible through:

- weapon models/mechanics/audio;
- first-person hands/arms;
- ADS/presentation;
- grenade/ordnance presentation;
- HMMWV/M2/BTR integration;
- vegetation/environment;
- world/material/LOD/graphics quality;
- tactical/performance preparation.

A local-only seam is recorded as deferred and **does not block later remote work**.

Forbidden default cadence:

`one weapon -> ask user to test -> fix -> next weapon -> ask again`

Required cadence:

`broad remote integration -> one useful integrated UE session -> one defect list -> batch fixes -> failed-component retests -> final integrated acceptance`

## Critical-only verification

During ordinary continuation:

- run only checks owned by the changed production surface;
- use one canonical verifier per responsibility where practical;
- do not add verifier-of-verifier chains for bookkeeping;
- do not rerun broad source/exact-head suites after every micro-change;
- broad suites belong at meaningful batch checkpoints and merge preparation;
- historical/calibration/local-evidence/documentation diagnostics are manual/on-demand unless they directly protect current production behavior;
- stale or duplicate workflows/verifiers are demoted or retired instead of forcing production code to satisfy history.

## Full re-audit trigger

A broad re-audit is allowed only when one of these is factual:

- runtime/subsystem ownership materially changed;
- UE version or relevant external license changed;
- merge/rebase/history rewrite invalidated provenance/checkpoint identity;
- new direct runtime evidence contradicts a current assumption;
- checkpoint/history is corrupt or cannot be reconciled;
- a newly discovered dependency changes the execution architecture materially.

Otherwise a full-project replay is prohibited.

## Runtime truth

Source/CI/importer green is not UE runtime acceptance.

Conversely, deferred runtime acceptance is not permission to stop repository work while other safe remote work remains.

Do not fabricate calibration values, `.uasset` import results, fresh-load results, screenshots or runtime verdicts.

## Local user Changes

Uncommitted/stashed files that exist only on the user's PC are outside remote mutation scope unless explicitly requested and factually accessible.

## Progress accounting

Formal PASS45 percentage changes only when a canonical checklist item meets its actual acceptance condition.

Documentation, source preparation, CI contracts, quarantine intake and deferred local acceptance do not increase the official percentage by themselves.

Current truth while item 16 local acceptance is deferred:

```text
official_progress=22/36=61.1%
runtime_acceptance=0
item16_checked=0
merge_permitted=0
user_local_execution_requested=0
```

PR #94 remains OPEN / UNMERGED.

## User communication

If no local action is needed:

`ВІД ТЕБЕ ЗАРАЗ НІЧОГО НЕ ПОТРІБНО.`

If a genuine local hard blocker appears:

`ПОТРІБНА ТВОЯ ПЕРЕВІРКА.`

Then name `Oster Conflict / PASS45`, give the smallest exact action and the exact result/evidence needed.