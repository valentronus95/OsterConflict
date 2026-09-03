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
5. Inspect only the first factual open checklist item and its direct dependency surface. Do not perform a new broad audit of unrelated systems.
6. Continue until the next factual blocker or meaningful checkpoint.
7. After substantive work, update `PASS45_RUNTIME_RECOVERY_HISTORY.md` with what changed, what remains open, the next factual operation, and the unchanged/updated official checklist accounting.

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

Source green, verifier green, importer green, or CI green does not substitute for required local UE 5.8 runtime evidence. Conversely, an old runtime rejection does not automatically reject a newer current-head implementation that materially changed after that test; it requires a new current-head run.

## Local user Changes

Uncommitted/stashed files that exist only on the user's PC remain outside assistant mutation scope unless the user explicitly asks to change them and the assistant has factual local access. Remote GitHub work must not overwrite or require cleanup of those local Changes.

## Progress accounting

Do not increase the official PASS45 percentage for documentation, source preparation, CI contracts, quarantine intake, or pilot-only evidence. Only canonical checklist closure with its required acceptance may change the 36-item accounting.

## Weapon runtime cadence

For item 16 and related weapon setup:

- run bounded source/import/fresh-load/calibration checks while configuring individual weapons;
- do not run the expensive full gameplay runtime after every small weapon tweak;
- after the intended weapon setup set is ready, run one consolidated current-head UE 5.8 weapon runtime acceptance with direct visual/audio/gameplay evidence.

This cadence changes efficiency, not acceptance strictness.
