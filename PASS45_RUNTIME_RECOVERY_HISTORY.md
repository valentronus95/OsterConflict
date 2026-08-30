# PASS45 Runtime Recovery — Persistent Work History

This file is the durable human-readable work ledger for `PASS45_RUNTIME_RECOVERY_TZ.md`.

It complements Git history; it does not replace it. The purpose is to make it possible to answer, without reconstructing hundreds of commits, **what was changed, on which branch/PR, what reached `main`, what remains source-only, what requires UE 5.8 runtime acceptance, and what auxiliary branches/PRs still exist**.

## Canonical ownership

- Canonical TZ: `PASS45_RUNTIME_RECOVERY_TZ.md`
- Active integration branch: `fix/pass45-runtime-rejection-material-closure-20260826`
- Active integration PR: **#94**
- Target branch: `main`
- Recovery audit snapshot before this ledger was created: `cfb452a0a7d24ad8daebb737864b4358ef624b9d`
- `main` at that snapshot: `bca00f4046700f383af9f1742cc24b6a62401b1a`
- Integration branch relation to that `main`: **ahead 509 / behind 0** at the snapshot.
- Latest factual local runtime verdict carried by PR #94: **RUNTIME REJECTED 2026-08-27**.
- Merge rule: **PR #94 must remain OPEN / UNMERGED until a current-head local UE 5.8 full runtime test passes import, build, gameplay, automated evidence gates and direct screenshot/visual acceptance.** Green GitHub source CI is not runtime acceptance.

## Recording rule from 2026-08-30 onward

Every substantive PASS45 work cycle must append or update this ledger before being reported as finished. Each entry should record:

1. date;
2. canonical checklist item(s);
3. branch and PR;
4. start head and end/source head;
5. commit SHA(s) and short purpose;
6. production/source files materially changed;
7. verifier/workflow changes;
8. exact CI state for the indexed head;
9. runtime evidence state separately from CI;
10. whether anything was merged to `main`;
11. blockers/content gaps left open;
12. any auxiliary branch/PR that must later be merged, superseded, or closed.

A source improvement may be recorded as `SOURCE-CODED` or `SOURCE-VERIFIED`; it may never be recorded as runtime accepted without factual UE 5.8 evidence.

## Current integration state — recovery audit 2026-08-30

### PR #94

- Branch: `fix/pass45-runtime-rejection-material-closure-20260826`
- Snapshot head: `cfb452a0a7d24ad8daebb737864b4358ef624b9d`
- State: OPEN / UNMERGED
- Snapshot compare to `main@bca00f4046700f383af9f1742cc24b6a62401b1a`: ahead 509 / behind 0.
- This means the active PASS45 integration line contains all 509 commits in its own history relative to that main snapshot, but does **not** mean every older divergent PR is safely disposable; those require content-equivalence audit first.

### Known recovered milestones in the active integration line

| Area | Commit / milestone | Recorded result | Runtime status |
|---|---|---|---|
| ParkPaths / Gate K | `0c5d0e769f93c6991ae1065f247b52d9136e13ec` and later guards | exactly five central-park path proxies separated from `Sidewalks`; authored `SM_Stonepath_Var01` upgrade path guarded | pending UE visual acceptance |
| Block0 regional ground detail | milestone around `a481bda13c30a67867a66751f8bb64b3e9993fdd` | blocked water/road/roof/building/concrete/path/bridge placement; bounded deterministic detail pass; dedicated verifier/workflow | source verified; runtime visual acceptance pending |
| Manual bolt/pump/lever truth | `806519498724426578b2b951eb39845a8c553f20`, `a707705de70f0a45ad5cc375429e04c834ccca22` | removed false `PASS45_MANUAL_ACTION_PRESENTATION_READY`; procedural whole-transform cue now reports fallback/content gap honestly | authored moving-part animation/audio still open |
| Museum approach / conifers | `7ebc1cb03d2aa445bdaaad7fd26b50a95b92c194`, `088766de8f70d1538a3fdf06560f52869462fd68` | authored pale approach and conifer corridor bound to Museum owner; no ParkPaths stone path reuse; no exact-species/coordinate claim | `MUS-CAM-01..07` still required |
| Museum deciduous/litter continuation | `736d7993...`, `1e2e05f0...` | mixed deciduous periphery and ground litter source slice + regression guard | broader Gate K/runtime visual acceptance still open |
| Smoke gameplay volume | `fe613ea2...`, `27e718e8...`, `e44c7b29...` | replaced XY-only infinite-height smoke containment semantics with bounded 3D gameplay volume + regression guard | smoke look/scale/performance still requires runtime acceptance |
| Character BasicShape proxy retirement | `241cecc8...`, `09b207e5...`, `cc10a3c96e42f7ee228f35b80419780a5f578237` | production-default source-only BasicShape character proxy disabled fail-closed; dedicated verifier/workflow added | production character visual acceptance still separate |
| Vegetation continuation | `492917e005c81f836b2a6fbcd02b696ab3b629b5`, `520bb207c5802942273eb376dc75854ebc5cc157`, `e276df6588761579e02909f187cec193642d1a45` | authored pine-family verifier/runtime-tree intake mapping and foliage workflow trigger aligned | item remains runtime/visual dependent |
| Manual-action/content-gap guard continuation | `cfb452a0a7d24ad8daebb737864b4358ef624b9d` | guards manual-action and weapon catalog content gaps so missing authored content cannot false-pass | content/runtime gaps remain factual |

Abbreviated SHAs above are indexing aids where the full SHA was not recovered in this ledger pass; Git remains the authoritative raw commit record.

## Open legacy / auxiliary PR audit

These PRs were found still OPEN during the 2026-08-30 audit. They must not be forgotten or blindly merged.

### PR #84 — `chore/pass45-postmerge-state-sync-2-20260825`

- Head: `c35c868c2014aacd52fe1cd555c5ed31e5636181`
- OPEN / UNMERGED; GitHub currently reports non-mergeable.
- Docs-only by its own scope.
- Commit ancestry comparison against active PR #94 snapshot is **diverged**; the old branch has 2 unique commits relative to the active line.
- Required disposition: content-equivalence/doc-history audit, then explicitly mark superseded/close or port any still-valid history. Do not merge blindly.

### PR #85 — `fix/pass45-postmerge-content-closure-20260825`

- Head: `24b8f5a8b869ede9bd64be1faddec8e9e52dd414`
- OPEN / UNMERGED; GitHub currently reports non-mergeable.
- Scope included rejected generic residential retirement, yard/shed owner retirement, BTR material dependency guard, verifier/workflow changes and historical runtime evidence preservation.
- Commit ancestry comparison against active PR #94 snapshot is **diverged**; the old branch has 22 unique commits relative to the active line.
- Required disposition: compare effective file/content state against current #94 before closure. Do not assume commit divergence means the fixes are missing, and do not assume later independently implemented fixes make all 22 commits obsolete without checking.

### PR #90 — `fix/pass45-content-gap-truth-20260825`

- Head: `4d6817b4f929c345701c6230a23816d10ad6f554`
- OPEN / UNMERGED.
- Scope included Remington870/M249 explicit content-gap truth, required-local-content preflight, strict harness wiring and blocker report.
- Commit ancestry comparison against active PR #94 snapshot is **diverged**; the old branch has 7 unique commits relative to the active line.
- Required disposition: content-equivalence audit against current content-gap guards and current model registry before explicitly superseding/closing or porting anything.

### PR #95 — `content/free-gameplay-assets-intake-20260828`

- Head: `ffcb6294665942cd755d3ce8f5a16a3c1b2e0799`
- DRAFT / OPEN / UNMERGED.
- Base is the PASS45 integration branch, not `main`.
- Scope is intentionally separate content acquisition/provenance tooling: arms/audio/ambience/vehicle sound candidates; it does not itself change active runtime implementation.
- Commit ancestry comparison against active #94 snapshot is **diverged**; #95 has 4 unique commits relative to the active line because the integration branch continued moving after the content branch split.
- Required disposition: keep isolated until provenance/license/local acquisition/import validation is accepted; then deliberately rebase/port/merge the desired intake work. Never lose it merely because #94 advances.

## Main-merge history relevant to PASS45

Known historical merged milestones include:

- PR #83 merged to `main` at `f5e883fb69ae8bdd35c754dc895d8b06e4843e08` as a Pass45 runtime-recovery source milestone.
- PR #89 merged to `main` at `a375f52635fbe9fa07c1000aa706e28c53eb42f4` for the strict runtime acceptance harness.
- The current PR #94 is intentionally **not merged** because the latest factual local runtime remains rejected and current-head UE 5.8 acceptance has not yet been produced.

The active `main` snapshot at the 2026-08-30 audit is newer than those historical merge points: `bca00f4046700f383af9f1742cc24b6a62401b1a`.

## Current checklist accounting

At the last canonical checklist count before this recovery audit:

- total checklist items: 36;
- checked: 22;
- open: 14;
- official progress: **61.1%**;
- remaining: **38.9%**.

Source work on runtime-required items does not increase this official percentage until the checklist's factual acceptance condition is met.

Major remaining runtime-heavy blocks include authored/manual weapon-action closure, weapon content/material gaps, vegetation/world visual fidelity, HMMWV M2 hierarchy runtime acceptance, HMMWV road speed, BTR material/orientation/remote optic, fullscreen/native-scale/thermal soak, tactical-map screenshot, current-head full runtime test and protected merge.

## Final closure checklist for branch hygiene

Before PASS45 is declared finished, perform all of the following in addition to the canonical technical acceptance:

- re-read this history ledger and the canonical TZ;
- enumerate all open PRs/branches containing `PASS45` or based on the active integration branch;
- content-diff every divergent legacy PR against the final accepted integration head;
- port any unique still-valid change deliberately;
- explicitly close/supersede obsolete PRs instead of leaving them ambiguous;
- verify the accepted integration head is the exact head merged to `main`;
- record final merge commit in this ledger;
- record post-merge `main` SHA;
- record the exact local UE 5.8 runtime evidence set that authorized the merge;
- leave no unclassified PASS45 branch carrying required production changes.

## Status vocabulary

Use only these meanings:

- `SOURCE-CODED`: implementation exists in source.
- `SOURCE-VERIFIED`: relevant source/CI verification passed for the exact indexed head.
- `CONTENT GAP`: required authored content is absent/unaccepted.
- `RUNTIME PENDING`: no current-head local UE 5.8 acceptance exists.
- `RUNTIME REJECTED`: factual local runtime evidence rejected the state.
- `RUNTIME ACCEPTED`: current-head local UE 5.8 import/build/gameplay/evidence/direct visual acceptance passed.
- `MERGED`: exact accepted head was merged to `main` and merge SHA is recorded.

Never collapse these states into one generic `PASS`.