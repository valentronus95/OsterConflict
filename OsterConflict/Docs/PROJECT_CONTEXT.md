# Oster Conflict — project context

## Canonical project location
- Repository: `valentronus95/OsterConflict`
- Active branch for current work: `r13-content-gameplay-pass`
- Unreal project: `OsterConflict/OsterConflict.uproject`
- Target engine: Unreal Engine 5.8.x, Windows

This repository is the source of truth. Future work should be applied directly here rather than delivered as a separate replacement archive unless the user explicitly asks for an archive.

## Project direction
`Oster Conflict` is a first-person multiplayer shooter prototype set in Остер, Чернігівська область. Current work is focused on making the existing project stable and progressively replacing placeholder visuals with believable assets and proportions without breaking existing gameplay systems.

## Current R13 acceptance direction
### Main menu
- Approved menu direction uses the committed `Oster_Menu_BG` artwork.
- The gameplay world must not visibly bleed through the main-menu background.
- No large opaque swamp/green panel behind the menu.
- Only restrained local darkening/gradient on the left side is allowed for text readability.
- Branding direction: thin `OSTER`, large bold `CONFLICT`, subtitle `ОСТЕР • ГОЛОВНЕ МЕНЮ`.
- Main buttons: `СТАРТ`, `ЛОКАЛЬНА ГРА`, `МЕРЕЖЕВА ГРА`, `НАЛАШТУВАННЯ`, `ВИЙТИ З ГРИ`.

### Input / ESC regression
- Opening and closing ESC/pause UI must never leave movement or look input locked.
- Returning to gameplay must restore WASD, run, jump and Mouse X/Y.
- UI focus must target a focusable widget such as a button, never `SVerticalBox`.
- Repeated ESC cycles must not stack `IgnoreMoveInput` / `IgnoreLookInput` state.

### Current visual/gameplay defects reported in R13
- Excessive yellow/orange outdoor cast.
- Pickup first-person camera/geometry can block the view.
- Strange visible debug/helper sphere or primitive near interactive geometry.
- Existing source-built houses read as crude placeholder/blockout architecture and must not be treated as final Остер housing.
- House doors must be genuinely interactive where intended, with collision and open/close behavior.

## Changes already applied to `r13-content-gameplay-pass`
- `b0c2f1d` — input, lighting and pickup camera fixes transferred into the R13 branch.
- `0923866` — R13 menu focus and input restore repair:
  - UE 5.8 `UButton` focusability compile fix.
  - focus moved away from `SVerticalBox` to a real button.
  - repeated UI input locking no longer stacks indefinitely.
  - gameplay input is reset/restored when leaving menu UI.

## Development rules
- Modify the existing project. Do not restart from zero.
- Avoid regressions in working movement, shooting, bot behavior, networking and existing systems while fixing presentation.
- Keep commits focused.
- Do not commit generated folders: `Binaries`, `Intermediate`, `Saved`, `DerivedDataCache`.
- Large imported content belongs under Git LFS where configured.
- Keep documentation concise so it does not bloat the repository or working context.

## Work reports
Short milestone reports live in `OsterConflict/Docs/WorkReports/`. They record only the useful state: what changed, why, relevant commit(s), test result and remaining blockers. Full logs remain outside this folder unless a specific log is needed for a tracked bug.
