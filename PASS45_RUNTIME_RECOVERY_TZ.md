# OSTER CONFLICT — PASS 45 RUNTIME RECOVERY TZ

Date opened: 2026-08-24  
Execution policy compacted: 2026-09-05  
Target: Unreal Engine 5.8.x / Windows  
Canonical project: `OsterConflict/OsterConflict.uproject`  
Active branch: `fix/pass45-runtime-rejection-material-closure-20260826`  
Active PR: **#94 OPEN / UNMERGED**  
Integrated baseline: `main@bca00f4046700f383af9f1742cc24b6a62401b1a`

## 0. Purpose

This is the **short canonical execution TZ** for PASS45.

Historical investigation, old checkpoints, rejected experiments and detailed chronology belong in:

- `PASS45_RUNTIME_RECOVERY_HISTORY.md`;
- `OSTER_CONFLICT_WORK_LEDGER.md`;
- `RUNTIME_EVIDENCE/`;
- Git history;
- feature/reference-specific subordinate specs.

Do not copy historical analysis back into this file unless it changes a current production rule.

## 1. Authority

Authority order:

1. latest explicit user requirement + latest factual UE runtime evidence;
2. `AGENTS.md`;
3. this compact TZ;
4. current targeted ledger/history entries;
5. current implementation and active critical acceptance scripts;
6. historical reports/verifiers/old assumptions.

A historical verifier never outranks newer runtime truth or a newer user requirement.

## 2. Current factual state

- PASS45 remains **RUNTIME REJECTED** until a later current-head UE 5.8 integrated acceptance supersedes the latest rejection.
- Formal checklist progress remains **22/36 = 61.1% complete, 38.9% remaining**.
- First factual open checklist item remains **item 16**.
- User-local UE execution is deliberately deferred while remote-preparable work remains.
- PR #94 remains **OPEN / UNMERGED**.

Required truth flags while local acceptance is deferred:

```text
runtime_acceptance=0
item16_checked=0
merge_permitted=0
user_local_execution_requested=0
```

## 3. FAST EXECUTION POLICY — BINDING

### 3.1 Batch first, not micro-task first

The unit of work is a **meaningful subsystem/content batch**, not one weapon, one animation, one SoundWave, one verifier or one tiny commit.

Default cadence:

`reconcile latest checkpoint -> implement a coherent remote-preparable batch -> run only critical affected checks -> continue next remote-preparable batch -> integrated UE 5.8 acceptance later`

Do not fall back to:

`one weapon -> local test -> fix -> another weapon -> local test -> ...`

unless the user explicitly asks for it or a genuinely local-only fact blocks safe work across the remaining approved batch.

### 3.2 Deferred local evidence is not a queue blocker

If an item is blocked only by local UE visual/audio/gameplay evidence:

- keep it factually open;
- mark the local seam as deferred;
- continue later remote-preparable checklist items and direct dependencies;
- do not fabricate runtime acceptance;
- return to deferred items together in one integrated acceptance window.

The planned integrated local package should contain as much as safely possible of:

1. weapon models/mechanics/audio;
2. first-person hands/arms and weapon-hand presentation;
3. ADS preparation and weapon presentation;
4. grenade/ordnance presentation;
5. HMMWV/M2/BTR model and gameplay integration;
6. vegetation/environment replacement;
7. world/material/LOD/graphics quality improvements;
8. tactical/display/performance work that can be prepared remotely.

### 3.3 Critical-only verification budget

Verification exists to catch real regressions, not to become the project.

Rules:

- prefer **one canonical verifier per responsibility**;
- do not create a verifier-of-a-verifier unless it protects a concrete high-risk production invariant that no existing check covers;
- do not duplicate the same path/SHA/schema/namespace/timing assertion across several scripts;
- historical/calibration/local-evidence/documentation diagnostics are **manual/on-demand**, not automatic blockers for ordinary PASS45 commits;
- documentation-only changes must not trigger heavy source/runtime suites unless the documentation itself controls executable behavior;
- path-scoped checks should run only when their owned production surface changes;
- broad exact-head/source verification is a **batch/milestone/merge check**, not a reason to split implementation into micro-commits;
- if two automatic workflows substantially validate the same responsibility, keep the stronger/current owner automatic and demote or retire the duplicate;
- a stale verifier is updated, demoted or deleted. Production code is never distorted merely to keep an obsolete check green.

### 3.4 No repeated broad audits

On continuation, read only:

1. current branch/HEAD/PR state;
2. `PASS45_RUNTIME_RECOVERY_HISTORY.md` latest checkpoint;
3. this compact TZ sections relevant to the next remote-preparable batch;
4. targeted ledger/reference/provenance entries only when that batch needs them.

Do not reread/re-audit the whole repository, whole ledger, all historical passes or every verifier unless a real architecture/history/runtime contradiction invalidates the checkpoint.

### 3.5 Reuse-first without research loops

Use:

`audit current owner -> reuse proven UE/current asset where suitable -> integrate -> critical source check -> defer local-only acceptance if needed -> continue`

Do not run a new framework/library/asset survey for every component. Search externally only when the current repository has no suitable production source or the existing source is legally/technically unusable.

Unknown external license/provenance remains **DO NOT IMPORT** and uses `_DOCS/THIRD_PARTY_CODE_AND_ASSET_REGISTER.md`.

### 3.6 Commit/checkpoint cadence

- Commit coherent production batches, not every tiny assertion cleanup.
- Update history at a meaningful checkpoint, not after every microscopic edit.
- Do not spend a cycle changing only status wording unless it prevents a factual execution error.
- Parallel chats must reconcile the newest branch HEAD before writing and must not replay already committed work.

### 3.7 ONE PASS45 WORK BRANCH ONLY — HARD RULE

For the entire lifetime of this TZ there is exactly **one canonical PASS45 work branch**:

`fix/pass45-runtime-rejection-material-closure-20260826`

Binding rules:

- all PASS45 code, asset-intake, launcher, verifier, documentation, checkpoint and runtime-recovery work is committed directly to this branch;
- **do not create another PASS45 remote branch** for audit, checkpoint, backup, temporary work, asset intake, individual fixes, experiments or CI repair;
- **do not open another PASS45 PR** while PR #94 is the active canonical PR;
- parallel chats must write to the same canonical branch after reconciling its newest HEAD;
- if accidental PASS45 work appears on another branch, first transfer only genuinely unique non-regressive work into the canonical branch, then close the duplicate PR and retire/delete the duplicate branch;
- an old branch may be inspected as evidence, but it is never resumed as a second active work line;
- a new work branch is allowed only for a **new TZ** or when the user explicitly orders a branch change;
- `main` remains the integration target, not a second development branch. PR #94 merges to `main` only after the factual acceptance gates in this TZ pass.

This rule overrides any older branch-per-fix, branch-per-pass, branch-per-asset or branch-per-checkpoint convention.

### 3.8 USER GIT WORKFLOW — GITHUB DESKTOP IS AUTHORITATIVE

The user works with the local repository through **GitHub Desktop**. This is the default user-facing Git workflow for PASS45.

Binding rules:

- instructions given to the user for branch selection, Fetch, Pull, Commit, Push, viewing Changes, History or PR-related local workflow must be described in **GitHub Desktop UI terms first**;
- do not assume the user is working from Git Bash, PowerShell, CMD or another terminal Git client;
- terminal Git commands are used only when GitHub Desktop does not expose the required operation or when the user explicitly requests command-line instructions;
- the selected local branch in GitHub Desktop must remain `fix/pass45-runtime-rejection-material-closure-20260826` for PASS45 work;
- local files shown under **Changes** in GitHub Desktop are user-local work and must not be discarded, reset, stashed, overwritten or silently absorbed into remote maintenance work;
- never instruct the user to press **Discard changes**, perform a destructive reset, delete local files, or switch branches in a way that risks the current Changes unless the user explicitly orders that destructive action after the risk is stated;
- remote GitHub maintenance must assume the user's local GitHub Desktop worktree can contain legitimate uncommitted Changes;
- when synchronization is needed, distinguish clearly between **Fetch origin**, **Pull origin**, **Push origin**, and **Commit**; do not describe them as interchangeable actions;
- if a remote branch is retired, GitHub Desktop may continue displaying a stale `origin/...` reference until the next fetch/prune. That visual residue does not make it an active PASS45 branch;
- all PASS45 continuation instructions must preserve the user's GitHub Desktop workflow and the single canonical branch rule above.

This rule is binding for all later PASS45 chats and overrides older terminal-first instructions.

## 4. Non-regression rules that remain critical

These are the minimum hard rules that must survive all acceleration work:

- one runtime responsibility has one mutating owner;
- no rejected BasicShape/default/white material may impersonate production-ready content;
- no resurrection of rejected Pass44/Pass37 runtime owners/fallbacks;
- compact central Oster playable area remains authoritative; do not restore historical ~2.4 km expansion;
- normal local game has no implicit heavy bot fill;
- actual Museum BASE pawn placement is required, not a source-only spawn marker;
- heavy/optional production content must not reintroduce known startup-blocking synchronous constructor/CDO loads;
- gameplay authority stays server-owned; presentation cannot become a second gameplay timer/authority;
- local `Changes` on the user's PC are not modified by remote GitHub work;
- `asset-intake-20260903` is quarantine-only and is never merged wholesale;
- no checklist items 37+ are created from audits, asset intake or helper protocols;
- PR #94 is not merged before factual integrated current-head UE 5.8 runtime acceptance.

## 5. Current weapon/item-16 boundary

Checklist item 16 remains open, but it must **not stop later remote work**.

Current remote/source state includes:

- M700: factual weighted `BOLT` source exists; bounded translation preparation exists; final travel/rotation remains local visual calibration;
- Remington 870: production skeletal source and pump sequence path exist; final direct visible-pump/gameplay acceptance is deferred;
- Lever Action: factual `LEVER` source exists; 0.85 s motion contract and UE 5.8 resampling compatibility are prepared; final angle remains local visual calibration;
- manual-action audio routing exists by action family;
- pinned M700/Lever donor audio/source preparation exists; UE SoundWave import/fresh-load/runtime audibility must remain factual and cannot be invented remotely;
- the rejected whole-weapon procedural manual-action fallback remains physically retired.

Do not spend another development cycle solely proving the same deferred M700/870/Lever local boundary while other checklist work is remotely available.

## 6. Corrective execution checklist

Completed/source-closed items stay frozen unless newer evidence invalidates them.

1. [x] Preserve latest rejection evidence and factual runtime verdict.
2. [x] Keep newer evidence authoritative over older reports.
3. [x] Retain daylight/exposure source correction and material-stability contract.
4. [x] Retain initial-character-only vehicle BASE recovery architecture.
5. [x] Retain proportional vehicle fit and HMMWV forward-axis improvement.
6. [x] Retain Stein R3 authored-material/fresh-load source path.
7. [x] Source-audit weapon firing/recoil/muzzle/drop defects.
8. [x] Production-muzzle weapon FX/audio source path.
9. [x] Launcher production-muzzle projectile/FX/audio and no-ammo-on-failed-spawn.
10. [x] Authority-simulated deliberate weapon drops.
11. [x] Confirmed-shot recoil migration.
12. [x] Critical source guard for firing/muzzle/drop ownership.
13. [x] Retire duplicate Character local recoil/feedback owner.
14. [x] Data-driven selector/action matrix and finite opt-in Burst3 architecture.
15. [x] Replicated-gate authored manual-action bridge; procedural whole-weapon fallback retired.
16. [ ] Finish authored M700 / Remington 870 / Lever Action moving-part presentation and real mechanical audio; **local final acceptance deferred into integrated batch**.
17. [x] Fail-visible per-weapon ADS/sight architecture and diagnostics.
18. [ ] Prepare/calibrate exact production ADS references/transforms; final visual acceptance may be deferred into integrated batch.
19. [x] Source-close silent-shot path with current repository fallback.
20. [ ] Replace temporary generic weapon audio with accepted per-weapon shot/reload/distant/mechanical profiles.
21. [x] Source-retire visible primitive weapon/pickup/launcher fallbacks.
22. [x] Source-retire primitive grenade/smoke visuals and wire authored smoke owner.
23. [x] Correct grenade spawn/commit/throw semantics.
24. [ ] Finish first-person grenade hand/throw/recover and distinct frag/smoke/flash presentation/VFX.
25. [x] Source-close Museum/Culture House/Silpo identity ownership.
26. [x] Bind separate landmark reference contracts.
27. [ ] Replace/repair rejected vegetation family and environment presentation.
28. [ ] Complete HMMWV M2 ring/shield/gunner hierarchy, pivot, yaw and camera.
29. [ ] Prepare HMMWV road-speed/handling target >=80 km/h; final feel acceptance deferred if necessary.
30. [ ] Close BTR white/default material state.
31. [ ] Prepare BTR orientation and remote operator optic/monitor gameplay.
32. [ ] Raise core world/material/LOD/graphics fidelity above prototype state, including ParkPaths/ground/landmark surroundings.
33. [ ] Validate fullscreen + 60 FPS + thermal soak after visual batch is ready.
34. [ ] Validate tactical map.
35. [ ] Run current-head integrated `START_HERE.cmd -> 2. ПОВНИЙ RUNTIME-ТЕСТ` only when the broad package is worth testing.
36. [ ] Merge PR #94 only after factual integrated current-head runtime acceptance.

## 7. Execution priority while local UE is deferred

Do not block on item-number order when the earlier item is local-only. Continue remote-preparable work in this practical order:

1. finish remaining weapon/audio/ADS/hands preparation across items 16/18/20;
2. finish grenade presentation preparation item 24;
3. vegetation/environment item 27;
4. HMMWV/M2 items 28–29;
5. BTR items 30–31;
6. world/material/LOD/graphics item 32;
7. tactical/performance preparation for items 33–34;
8. one integrated local acceptance for the prepared package;
9. batch-fix the returned defect list;
10. rerun only failed components, then one final integrated acceptance.

## 8. Final acceptance gates — compact

### A. Build/content
Current-head UE 5.8 build/import/cook-required content loads without hidden local state or rejected fallback content.

### B. Weapons/ordnance
Recognizable production visuals; factual shot/recoil/ammo/muzzle/audio ownership; authored manual actions; accepted ADS; grenade presentation; no visible primitive fallback.

### C. World/landmarks/graphics
Usable daylight; stable materials; compact Oster bounds; Museum/Culture/Silpo separation; acceptable vegetation/LOD/ground/landmark fidelity; no major white/default/proxy visuals.

### D. Vehicles
HMMWV direction/speed/M2 hierarchy/camera and BTR material/orientation/remote operator are coherent; vehicle exit never teleports the player to Museum.

### E. Performance/map
Intended display mode, approximately 60 FPS target under representative gameplay, thermal soak, usable compact tactical map.

### F. Integrated user acceptance
One current-head UE 5.8 visual/audio/gameplay session over the prepared package, one consolidated defect list, targeted failed-component retests after fixes, then final integrated acceptance.

## 9. Supporting files

Binding execution helpers:

- `_DOCS/PASS45_CHECKPOINT_CONTINUATION_PROTOCOL.md`
- `_DOCS/PASS45_COMPONENT_FIRST_UE_DEBUGGING_PROTOCOL.md`
- `_DOCS/PASS45_REUSE_FIRST_REPLACEMENT_SPEC.md`
- `_DOCS/THIRD_PARTY_CODE_AND_ASSET_REGISTER.md`
- `PASS45_RUNTIME_RECOVERY_HISTORY.md`

Reference/photo fidelity remains owned by `PASS45_REFERENCE_PACK_BINDINGS.md` and its subordinate location specs. Those details stay out of this compact execution TZ unless they materially change the current production rule.

## 10. Current verdict

**PASS45 = ACTIVE / RUNTIME ACCEPTANCE DEFERRED WHILE REMOTE-PREPARABLE WORK CONTINUES.**

Formal progress remains:

```text
22/36 = 61.1% complete
38.9% remaining
runtime_acceptance=0
item16_checked=0
merge_permitted=0
user_local_execution_requested=0
```

PR #94 remains **OPEN / UNMERGED**.
