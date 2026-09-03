# PASS45 Checkpoint Continuation Protocol

Date: 2026-09-03
Scope: `PASS45_RUNTIME_RECOVERY_TZ.md`
Status: BINDING for continuation work on canonical PASS45

## Trigger

When the user requests any equivalent of:

`Продовжуй PASS45_RUNTIME_RECOVERY_TZ.md з останнього фактичного/актуального checkpoint`

continue from the latest factual checkpoint. Do **not** restart a full-project audit by default.

## Mandatory continuation sequence

1. Read the current `PASS45_RUNTIME_RECOVERY_HISTORY.md` and the canonical TZ sections relevant to the recorded first-open item.
2. Reconcile only the current integration state needed to prove the checkpoint is still current:
   - canonical branch;
   - current HEAD;
   - PR #94 state/head/base;
   - recent commits since the recorded checkpoint;
   - exact-head CI conclusions;
   - binding reuse-first / third-party rules when the open item touches external content.
3. If another chat/process advanced the branch, consume those commits and continue from the newest factual state. Never replay superseded work merely because the current conversation is new.
4. Treat already `ACCEPTED`, `CLOSED`, or otherwise factually completed work as frozen unless its owning source/contracts materially changed or newer runtime evidence invalidated it.
5. Inspect the first factual open checklist item and its direct dependency surface first. Do not perform a new broad audit of unrelated systems.
6. If that first-open item is blocked **only** by deferred user-local UE evidence and all safe remote work inside it is exhausted, leave it unchecked, record the deferred evidence, and continue the next remote-preparable item/direct dependency in the same broad integrated PASS45 batch. Do not stop every continuation at the same user-only blocker.
7. Continue until the next factual **remote** blocker or meaningful checkpoint. A deferred local UE check is not a remote blocker while useful repository/source/content/CI work remains elsewhere in the approved PASS45 scope.
8. After substantive work, update `PASS45_RUNTIME_RECOVERY_HISTORY.md` with what changed, what remains open, the next factual operation, and the unchanged/updated official checklist accounting.

## Deferred user-local acceptance rule

A pending user gameplay/visual/audio check is an acceptance dependency, not automatically a reason to halt repository work.

For PASS45 continuation while the user has deferred local UE execution:

- keep every deferred runtime-dependent checklist item factually open;
- continue source/content/CI preparation into later remote-preparable checklist items where dependencies allow it;
- do **not** stop at one weapon, one asset, one model, one animation or one SoundWave merely because its final proof needs UE 5.8;
- do not fabricate calibration values, imported `.uasset` files, fresh-load results or runtime verdicts;
- do not merge PR #94 while required runtime acceptance remains pending;
- keep `runtime_acceptance=0`, `item16_checked=0`, `merge_permitted=0`, and `user_local_execution_requested=0` truthful until their real acceptance conditions are met;
- return to deferred local-only items together when the broad integrated PASS45 batch is ready for a useful end-to-end check.

### Broad integrated PASS45 batch target

The user has explicitly chosen **batch-first local verification**. The deferred local acceptance window is not limited to the current weapon item.

Before asking the user for the planned local UE 5.8 check, continue as far as safely possible through the current TZ so the integrated build contains, where the repository/TZ and available assets permit:

1. the intended weapon model/mechanics/audio preparation rather than one isolated weapon;
2. first-person hands/arms presentation and weapon-hand integration;
3. other production model/asset replacements that are already remote-preparable;
4. vehicle model/integration work, including the tracked military vehicle set where sources exist;
5. graphics/visual-quality improvements that are remote-preparable without inventing runtime evidence.

A local-only acceptance seam inside any one of those areas must be recorded as `DEFERRED_LOCAL_ACCEPTANCE` (or the equivalent factual open state) and **must not block moving to the next remote-preparable area**.

The intended cadence is therefore:

`remote-preparable broad integration -> one meaningful integrated UE 5.8 check -> one consolidated defect list -> batch fixes -> targeted failed-component retests -> final integrated acceptance`

It is specifically prohibited to fall back to:

`one weapon -> ask user to test -> fix -> another weapon -> ask user to test -> ...`

unless the user explicitly requests that cadence or a genuinely local-only fact makes further remote work unsafe/impossible across the remaining approved batch.

## Full re-audit is allowed only when justified

A broad re-audit is permitted only if at least one of these is true:

- subsystem ownership or architecture materially changed;
- Unreal Engine target/version or relevant external license changed;
- merge/rebase/history rewrite invalidated prior provenance or checkpoint identity;
- new direct runtime evidence contradicts an accepted assumption;
- the checkpoint/history is missing, corrupt, internally inconsistent, or cannot be reconciled to Git history;
- a newly discovered dependency materially changes the first-open item.

When none applies, a full-project replay is wasted work and is prohibited.

## Parallel-chat rule

Parallel chats are allowed to advance the same canonical branch only if each continuation first reconciles current HEAD/history. The newest committed factual state wins. Never assume the SHA remembered by the conversation is still current.

## Runtime truth

Source green, verifier green, importer green, or CI green does not substitute for required local UE 5.8 runtime evidence. Conversely, an old runtime rejection does not automatically reject a newer current-head implementation that materially changed after that test; it requires a new current-head run when the consolidated acceptance window is reached.

## Local user Changes

Uncommitted/stashed files that exist only on the user's PC remain outside assistant mutation scope unless the user explicitly asks to change them and the assistant has factual local access. Remote GitHub work must not overwrite or require cleanup of those local Changes.

## Progress accounting

Do not increase the official PASS45 percentage for documentation, source preparation, CI contracts, quarantine intake, pilot-only evidence, or for merely deferring a required local acceptance. Only canonical checklist closure with its required acceptance may change the 36-item accounting.

## Integrated runtime cadence

For item 16 and later remotely-preparable PASS45 work:

- run all possible repository/source/static/CI checks while configuring individual components;
- do **not** ask the user to run UE after every small weapon tweak, asset import preparation, vehicle change, hands change, visual change, or after each individual component reaches a local-only boundary;
- prepare the broad integrated PASS45 set as far as safely possible first;
- then run one consolidated current-head UE 5.8 visual/audio/gameplay acceptance over the integrated batch;
- collect one defect list and fix it as a batch;
- during corrective debugging, rerun only the component(s) that actually failed;
- run one final consolidated acceptance after corrections.

A targeted single-component user run before that batch is allowed only when a genuinely local-only fact is a hard blocker that prevents further safe remote work across the remaining batch, or when the user explicitly asks to test it now.

## User-facing communication rule

The user works on multiple projects and is not expected to decode long technical reports.

Every continuation summary must lead with plain-language Ukrainian: what was done, what remains, what comes next, and the official percentage. Technical verifier/workflow/SHA details are secondary.

If no action is required from the user, state clearly:

`ВІД ТЕБЕ ЗАРАЗ НІЧОГО НЕ ПОТРІБНО.`

If a local check becomes truly necessary, state clearly and prominently:

`ПОТРІБНА ТВОЯ ПЕРЕВІРКА.`

Then name `Oster Conflict / PASS45`, explain why remote work cannot continue safely without that check, give the smallest exact action, and say what evidence/result is needed. Never bury a required user action inside technical prose.

This cadence and communication rule change efficiency and clarity, not acceptance strictness.