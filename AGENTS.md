# Oster Conflict — persistent work context

This repository is the canonical working project for the user.

## Repository
- GitHub: `valentronus95/OsterConflict`
- Active development branch: `r13-content-gameplay-pass`
- Unreal Engine target: UE 5.8.x on Windows
- Project: `OsterConflict/OsterConflict.uproject`

## Mandatory context files
Before editing the project, read these files in this order:
1. `AGENTS.md`
2. `OSTER_CONFLICT_MASTER_TZ.md` — canonical requirements and acceptance criteria.
3. `OSTER_CONFLICT_WORK_LEDGER.md` — active issues, repeat counts, real completion state, commits and test status.
4. `OsterConflict/Docs/PROJECT_CONTEXT.md` — concise technical context.

The MASTER TZ and WORK LEDGER are persistent living documents. Update them whenever the user adds/changes a requirement, repeats an unresolved requirement, a meaningful code block is committed, or build/playtest changes the status of a task.

## Mandatory workflow
1. Work directly in this GitHub repository. Do not create replacement ZIP archives as the default delivery method.
2. Before editing, verify the current branch and read all mandatory context files listed above.
3. Preserve existing project history and avoid destructive rewrites or force-updating branches unless explicitly required.
4. Keep fixes small and reviewable. Do not mix unrelated regressions into one change.
5. Do not commit generated UE folders: `Binaries`, `Intermediate`, `Saved`, `DerivedDataCache`.
6. For user test feedback, fix the reported regression first, then update `OSTER_CONFLICT_WORK_LEDGER.md`. Add a short report in `OsterConflict/Docs/WorkReports/` only when a meaningful milestone or test pass is completed.
7. Reports must stay lightweight Markdown. Do not store copied build logs, screenshots, binaries, archives, or generated assets in the reports folder.
8. Never call an item `VERIFIED` or "done" merely because code was committed. Until UE build/runtime/user playtest confirms it, use `CODED_UNTESTED` in the ledger.
9. If the user repeats an unresolved request, increment its `Repeat` counter in the ledger instead of silently creating another duplicate task.
10. Before creating primitive/blockout geometry, inventory the already imported assets under `OsterConflict/Content` and prefer a suitable real asset when one exists.
11. For geography and landmark placement, user-confirmed local knowledge and verified photo/map evidence override old provisional code coordinates.
12. Do not let independent late runtime subsystems silently overwrite an already visible landmark. One site/landmark should have a clear placement owner.

## Current priority
R13 location repair, visual/gameplay stabilization, and replacement of placeholders with existing real assets. Highest-priority tracked work is maintained in `OSTER_CONFLICT_WORK_LEDGER.md`, currently centered on:
- separating and correctly placing Museum / Silpo / Culture House / water tower;
- museum stadium-left + rear-slope + lower-residential topology;
- photo-driven location fidelity;
- eliminating late runtime rebuild/flicker;
- using existing imported houses/fences/lights/weapons/vehicles instead of visible primitive placeholders;
- validating the complete spawn-relative weapon test rack.

The detailed acceptance requirements live in `OSTER_CONFLICT_MASTER_TZ.md`.
