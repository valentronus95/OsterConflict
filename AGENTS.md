# Oster Conflict — persistent work context

This repository is the canonical working project for the user.

## Repository

- GitHub: `valentronus95/OsterConflict`
- Primary integration branch: `main`
- Unreal Engine target: UE 5.8.x on Windows
- Project: `OsterConflict/OsterConflict.uproject`

## Authority

Current authority order:

1. latest explicit user requirement and latest user-observed runtime evidence;
2. current `AGENTS.md`;
3. current dedicated TZ / current targeted ledger state;
4. current implementation and active critical acceptance scripts;
5. historical verifier scripts, reports and old pass assumptions.

A lower historical rule must never silently undo a newer requirement or factual runtime result.

## Context loading — do not reread the world

### Normal project work

Read only the files required by the feature being changed:

1. `AGENTS.md`;
2. the relevant dedicated TZ/status file;
3. targeted `OSTER_CONFLICT_WORK_LEDGER.md` entries when needed;
4. relevant reference/provenance files.

Do not reread unrelated project history by default.

### PASS45 continuation fast path

When continuing `PASS45_RUNTIME_RECOVERY_TZ.md`, use this order:

1. reconcile canonical branch / HEAD / PR #94;
2. read the latest checkpoint in `PASS45_RUNTIME_RECOVERY_HISTORY.md`;
3. read the compact `PASS45_RUNTIME_RECOVERY_TZ.md` sections for the next remote-preparable batch;
4. read only targeted ledger/reference/provenance entries required by that batch.

Do **not** reread the entire ledger, all historical passes, all verifiers, all reports or the whole repository on every continuation. A broad re-audit is allowed only when architecture/ownership, engine/license, merge history, checkpoint integrity or newer direct runtime evidence materially invalidates the current checkpoint.

For PASS45, `_DOCS/PASS45_CHECKPOINT_CONTINUATION_PROTOCOL.md` and `_DOCS/PASS45_COMPONENT_FIRST_UE_DEBUGGING_PROTOCOL.md` are binding.

## Core repository workflow

1. Work directly in this GitHub repository. Do not use replacement ZIP archives as the normal delivery path.
2. Verify current branch/HEAD before writing. Consume newer parallel-chat commits instead of replaying old work.
3. Preserve history. No destructive rewrite/force-update unless explicitly justified.
4. Commit **coherent subsystem/content batches**. Do not fragment work into micro-commits merely so every tiny change can have its own verifier cycle.
5. Do not commit generated UE folders: `Binaries`, `Intermediate`, `Saved`, `DerivedDataCache`.
6. User test feedback outranks stale source assumptions. Fix the reported regression before expanding scope.
7. Reports stay lightweight. Write a report only for a meaningful milestone/test pass, not every microscopic edit.
8. Source/CI success is not UE runtime acceptance. Use `CODED_UNTESTED`/equivalent until factual runtime evidence exists.
9. If a user repeats an unresolved request, update the existing task rather than creating duplicate work.
10. Before creating primitive/blockout content, inventory existing imported assets and reuse a suitable real asset when one exists.
11. External code/content requires known license/provenance. Unknown provenance is **DO NOT IMPORT**.
12. Assistant-owned Git operations are performed by the assistant whenever connected tooling permits them. Do not offload remote Git/PR/CI work to the user.
13. Local uncommitted user `Changes` are outside remote mutation scope unless the user explicitly asks and factual local access exists.

## Branch discipline — binding

- For one active TZ, keep exactly **one canonical work branch** plus `main`.
- For PASS45, the only canonical work branch is `fix/pass45-runtime-rejection-material-closure-20260826` until PR #94 is accepted and merged.
- Do **not** create separate remote branches for audits, checkpoints, backups, asset intake, individual weapons, individual fixes, verifier changes or temporary experiments.
- Git history is the rollback mechanism. Do not keep remote `backup/*`, `tmp-*`, duplicate pass/fix branches or similar branch clutter after their unique work has been reconciled.
- Before retiring an existing branch, verify whether it contains commits or assets not present in the canonical work branch. Transfer only unique required production/provenance work; never merge an old branch wholesale merely to preserve it.
- A new TZ may use a new branch only when the user creates/requests that branch. The assistant does not create an additional TZ branch on its own.
- Normal delivery cadence is: `work on canonical branch -> push coherent batch -> run only applicable critical checks -> merge to main as soon as the batch satisfies its factual acceptance gates`.
- Runtime/visual/audio changes that require UE evidence are not considered accepted merely because GitHub CI is green. Do not move known runtime-rejected work into `main` solely to reduce branch count.
- After an accepted TZ branch is merged, retire that work branch instead of leaving it as a permanent backup.
- Third-party/raw source downloads are temporary intake, not final game content. Accepted shipping assets must be imported into the Unreal project under `OsterConflict/Content/...` with required provenance retained; do not create a permanent remote branch just to warehouse source archives.

## Runtime/content non-regression

- One runtime responsibility has one current mutating owner.
- Superseded/rejected mutation owners are physically retired when their replacement owns the responsibility; Git history is the rollback.
- No historical verifier may require a runtime-rejected owner/fallback back into production.
- Missing production content must fail visibly. BasicShape/default/white fallback cannot impersonate production readiness.
- Server owns gameplay facts. Presentation/audio/animation may not become a second gameplay timer or authority.
- Normal local game has no implicit heavy bot fill.
- Heavy/optional production content must not reintroduce known startup-blocking synchronous constructor/CDO loads.

## Oster world authority

### Playable area

The user-approved compact central Oster area is authoritative:

`REFERENCE_PHOTOS/map_extent/oster_central_playable_area_20260824.jpg`

Do not restore the historical ~2.4 km procedural battlefield, far BASE compounds, peripheral roads/hydrography/residential grids or tactical-map auto-fit that expands beyond this reference.

Primary authoring must respect compact bounds. A late cleanup subsystem is only a safety net, not permission to create out-of-bounds geometry first.

### BASE

Museum BASE means the **actual live player pawn** spawns on the Museum approach. A source-only spawn actor near Museum is not acceptance.

Vehicle enter/exit is not fresh BASE deployment and must never teleport the player to Museum.

### Landmarks

Museum, Culture House, Silpo, Stadium and water-tower responsibilities must have clear placement/visual owners. A late subsystem may not silently overwrite an already visible landmark.

User-confirmed geography and verified photo/map evidence override provisional old coordinates.

Reference photos supplied for 3D/location work are preserved in named `REFERENCE_PHOTOS/<location>/` packs with an index/manifest.

### Tactical map

Tactical-map projection stays bounded by the compact playable reference. Out-of-bounds components must not force the map to zoom farther out.

## PASS45 fast execution rules

### Batch-first local acceptance

The user has explicitly deferred local UE checks until a **large integrated package** is worth testing.

While safe remote work remains, do not stop on one weapon, animation, SoundWave, ADS calibration seam, vehicle or graphics tweak merely because its final acceptance needs UE 5.8.

Prepare as much as safely possible of:

- weapons/mechanics/audio;
- first-person hands/arms;
- ADS/presentation;
- grenade/ordnance presentation;
- HMMWV/M2/BTR integration;
- vegetation/environment;
- world/material/LOD/graphics quality;
- tactical/performance preparation.

Then request one integrated current-head UE 5.8 session, collect one defect list, batch-fix it, rerun only failed components, and perform one final integrated acceptance.

A pre-batch single-component local run is allowed only if a genuinely local-only fact blocks safe remote work across the remaining approved batch or the user explicitly requests it.

### Critical-only verifier policy

For PASS45:

- prefer one canonical verifier per responsibility;
- do not create verifier-of-verifier chains without a concrete uncovered high-risk production invariant;
- do not duplicate path/SHA/schema/namespace/timing checks across several scripts;
- historical/calibration/local-evidence/documentation diagnostics are manual/on-demand unless they directly protect current production behavior;
- docs-only changes should not trigger heavy runtime/source workflows by default;
- broad source/exact-head verification is a batch/milestone/merge check, not a micro-change ritual;
- if automatic workflows overlap substantially, keep the stronger/current owner automatic and demote or retire the duplicate;
- stale verifiers are updated/demoted/deleted. Never distort production code to satisfy obsolete tests.

### PASS45 merge truth

Do not merge PR #94 merely to simplify local testing.

Until factual integrated current-head UE 5.8 acceptance passes:

```text
runtime_acceptance=0
item16_checked=0
merge_permitted=0
```

## User-facing communication

PASS45 continuation summaries lead with plain Ukrainian:

- what was done;
- what remains;
- what is next;
- official percentage.

If no user action is required, state:

`ВІД ТЕБЕ ЗАРАЗ НІЧОГО НЕ ПОТРІБНО.`

If local work becomes a real hard blocker, state prominently:

`ПОТРІБНА ТВОЯ ПЕРЕВІРКА.`

Name `Oster Conflict / PASS45`, give the smallest exact action, and explain what evidence is needed. Do not bury the user action in technical prose.

## Persistent progress-control/reporting protocol

This is a project-wide user requirement, not a one-chat preference.

- The assistant owns progress tracking and must keep the current state consistent across continuations without requiring the user to reconstruct it.
- After every meaningful work cycle, provide one **short status table** in Ukrainian. Use exactly: `✅` = fully done for the stated scope, `🟡` = in progress / coded but still awaiting required integration or UE runtime acceptance, `❌` = not done or factually blocked.
- A row may be marked `✅ 100%` only when every acceptance gate required by that row has actually passed. Source code alone does not make runtime/visual/audio work 100%.
- Never inflate progress to make the report look better. If a prior estimate was wrong, correct it explicitly from current evidence.
- Keep two concepts separate when needed: **formal TZ progress** and **asset/runtime integration progress**. Do not mix their percentages.
- The assistant must compare the new report against the previous factual state, detect regressions or stalled items itself, and continue the first safe unfinished item without waiting for the user to remind it.
- The report must show only: block/status, concise state or percentage, what remains when relevant. No long technical memoirs.
- User action is requested only for a real local-only blocker. Otherwise continue autonomously.

## Canonical asset inventory

`ASSET_STATUS.md` in the repository root is the binding inventory for imported/downloaded game content.

- Every newly observed user import, Fab/Marketplace pack, local donor root, vehicle, weapon, grenade, character, animation, building, prop, vegetation pack, UI pack, VFX or audio source must be added to `ASSET_STATUS.md` immediately when observed.
- A listed import root covers all of its files recursively. Do not flood the ledger with one row per texture/material unless a specific sub-asset needs its own runtime identity.
- Never remove an asset from the inventory silently. Superseded assets are marked for retirement, then physically removed when the replacement is factual; Git history is rollback.
- Before claiming an asset batch complete, reconcile `ASSET_STATUS.md` against the canonical Git tree, ignored local-import roots in `.gitignore`, user-observed Content Browser/git-status evidence and the latest explicit import/download report.
- GitHub cannot see ignored local payload on the user's PC. Such content remains explicitly tracked as `LOCAL/REPORT` until exact local identity is factually available; absence from Git is never treated as proof that the user did not import it.

## Current priority

For active PASS45 work, execute the compact `PASS45_RUNTIME_RECOVERY_TZ.md` queue and latest checkpoint instead of expanding architecture surveys.

For dedicated location work, use that location's TZ/status/reference pack. For Stadion Oster, use `STADION_OSTER_TZ.md`, `STADION_OSTER_IMPLEMENTATION_STATUS.md`, and `REFERENCE_PHOTOS/stadion_oster/`.
