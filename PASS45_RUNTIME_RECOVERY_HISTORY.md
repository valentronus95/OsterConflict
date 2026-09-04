# PASS45 Runtime Recovery — Current Checkpoint

Updated: 2026-09-04  
Purpose: fast continuation index for `PASS45_RUNTIME_RECOVERY_TZ.md`.

Detailed old chronology remains in Git history and `RUNTIME_EVIDENCE/`. Do not rebuild that chronology here.

## Canonical state

- Branch: `fix/pass45-runtime-rejection-material-closure-20260826`
- PR: **#94 OPEN / UNMERGED**
- Baseline: `main@bca00f4046700f383af9f1742cc24b6a62401b1a`
- Formal progress: **22/36 = 61.1% complete, 38.9% remaining**
- First factual open checklist item: **16**
- Local UE verification: **deferred by user until a broad integrated package is ready**
- Local user `Changes`: do not touch remotely

Truth while acceptance is deferred:

```text
runtime_acceptance=0
item16_checked=0
merge_permitted=0
user_local_execution_requested=0
```

## 2026-09-04 execution-policy reset

The previous workflow was too granular. PASS45 now uses a compact batch-first policy.

Binding files were simplified:

- `PASS45_RUNTIME_RECOVERY_TZ.md`
- `AGENTS.md`
- `_DOCS/PASS45_CHECKPOINT_CONTINUATION_PROTOCOL.md`
- `_DOCS/PASS45_COMPONENT_FIRST_UE_DEBUGGING_PROTOCOL.md`

Current rules:

1. continue from this checkpoint, not from a full-project re-audit;
2. do not stop on a local-only seam while later safe remote work exists;
3. implement coherent subsystem/content batches;
4. run only critical checks owned by the changed production surface;
5. broad verification belongs at meaningful batch/milestone/merge boundaries;
6. local UE is one broad integrated session later, followed by one defect list and targeted failed-component retests;
7. no fake runtime acceptance and no PR #94 merge before factual integrated acceptance.

## CI/workflow cleanup completed in this policy pass

The following historical/local/pilot/duplicate workflows were removed from ordinary automatic PR/push cadence and retained as manual diagnostics:

- `pass45-item16-local-ue58-evidence-chain.yml`
- `pass45-checkpoint-continuation-item16-calibration.yml`
- `pass45-component-first-ue-debugging.yml`
- `pass45-item16-production-cutover-preflight.yml`
- `pass45-item16-ue58-frame-rate-compat.yml`
- `pass45-m700-source-motion-audit.yml`
- `pass45-leveraction-source-motion-audit.yml`
- `pass45-m700-bolt-geometry-audit.yml`
- `pass45-remington870-donor-motion-audit.yml`
- `pass45-m700-derived-bolt-translation-ue58-pilot-contract.yml`
- `pass45-remington870-derived-pump-ue58-pilot-contract.yml`
- `pass45-remington870-derived-pump-ue58-assembly-audit.yml`

The M700/Lever source audits also no longer auto-commit generated audit reports back into the active branch.

Production-critical current owner remains automatic where appropriate, especially:

- `pass45-item16-production-profile-cutover.yml`
- feature-specific production/source guards for code actually being changed.

`source-verify.yml` is already not an automatic PASS45-branch gate; its broad suite is manual/on-demand or main/integration scoped.

## Current item-16 boundary

### M700

- weighted `BOLT` source exists;
- bounded translation preparation exists;
- final travel/rotation is deferred local visual calibration.

### Remington 870

- production skeletal source exists;
- pump animation/source preparation exists;
- direct visible-pump/gameplay acceptance is deferred.

### Lever Action

- weighted `LEVER` exists;
- factual manual-action duration remains 0.85 s;
- UE 5.8 resampling compatibility preparation exists;
- final lever angle is deferred local visual calibration.

### Manual-action audio

- action-family routing exists;
- pinned M700/Lever donor WAV/source preparation exists;
- actual UE SoundWave import/fresh-load/audibility remains factual local/content work and must not be invented remotely.

Item 16 remains open but **does not block the rest of the remote queue**.

## Next practical remote queue

Continue without requesting local UE:

1. remaining weapon/audio/ADS/hands preparation across items 16/18/20;
2. grenade first-person presentation/VFX item 24;
3. vegetation/environment item 27;
4. HMMWV/M2 hierarchy and handling items 28–29;
5. BTR material/orientation/remote optic items 30–31;
6. world/material/LOD/graphics quality item 32;
7. tactical/performance preparation items 33–34;
8. only then hand off one useful integrated UE 5.8 acceptance session.

## Continuation rule

At the start of the next chat/cycle:

1. reconcile current branch HEAD and PR #94;
2. read this checkpoint;
3. read only the relevant compact TZ section;
4. continue the first remotely actionable item in the practical queue;
5. do not replay item-16 calibration analysis merely because item 16 is still formally open.

## User action

`ВІД ТЕБЕ ЗАРАЗ НІЧОГО НЕ ПОТРІБНО.`
