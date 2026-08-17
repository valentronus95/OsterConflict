# Oster Conflict — persistent work context

This repository is the canonical working project for the user.

## Repository
- GitHub: `valentronus95/OsterConflict`
- Active development branch: `r13-content-gameplay-pass`
- Unreal Engine target: UE 5.8.x on Windows
- Project: `OsterConflict/OsterConflict.uproject`

## Mandatory workflow
1. Work directly in this GitHub repository. Do not create replacement ZIP archives as the default delivery method.
2. Before editing, verify the current branch and read `OsterConflict/Docs/PROJECT_CONTEXT.md`.
3. Preserve existing project history and avoid destructive rewrites or force-updating branches unless explicitly required.
4. Keep fixes small and reviewable. Do not mix unrelated regressions into one change.
5. Do not commit generated UE folders: `Binaries`, `Intermediate`, `Saved`, `DerivedDataCache`.
6. For user test feedback, fix the reported regression first, then record a short report in `OsterConflict/Docs/WorkReports/` only when a meaningful milestone or test pass is completed.
7. Reports must stay lightweight Markdown. Do not store copied build logs, screenshots, binaries, archives, or generated assets in the reports folder.

## Current priority
R13 regression repair and visual/gameplay stabilization. Current tracked issues include main-menu presentation/input focus, ESC input restoration, neutral outdoor lighting, vehicle first-person visibility, placeholder/debug geometry, house quality, and interactive doors.

The detailed current state and acceptance notes live in `OsterConflict/Docs/PROJECT_CONTEXT.md`.
