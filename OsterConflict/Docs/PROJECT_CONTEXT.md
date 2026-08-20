# Oster Conflict — project context

## Canonical project location
- Repository: `valentronus95/OsterConflict`
- Active branch for current work: `r13-content-gameplay-pass`
- Unreal project: `OsterConflict/OsterConflict.uproject`
- Target engine: Unreal Engine 5.8.x, Windows

This repository is the source of truth. Future work should be applied directly here rather than delivered as a separate replacement archive unless the user explicitly asks for an archive.

## Persistent project tracking
The detailed project state is intentionally split so requirements and actual implementation status cannot drift together:

- `OSTER_CONFLICT_MASTER_TZ.md` — canonical requirements and acceptance criteria.
- `OSTER_CONFLICT_WORK_LEDGER.md` — active issues, repeat counts, commits, remaining work and build/playtest status.
- `AGENTS.md` — mandatory workflow that requires both files to be read and updated.

This file stays concise. If there is a conflict, the latest explicit user requirement recorded in MASTER TZ and the actual tested state recorded in WORK LEDGER take precedence over old provisional notes here.

## Project direction
`Oster Conflict` is a first-person multiplayer shooter prototype set in Остер, Чернігівська область. Current work is focused on stabilizing the existing project, correcting location topology/geography, and progressively replacing placeholder visuals with already imported believable assets without breaking gameplay systems.

## Current R13 priorities
1. Separate and correctly place Museum / Silpo / Culture House / water tower.
2. Refine these landmarks from user references and verified public photos/maps.
3. Museum topology: stadium on the left when facing the entrance, descent behind the museum, lower residential area below.
4. Eliminate late runtime replacement/rebuild flicker.
5. Prefer existing imported houses, fences, street lights, weapons and vehicles over visible primitives.
6. Validate the complete spawn-relative weapon test rack.

Exact status and remaining blockers are maintained in `OSTER_CONFLICT_WORK_LEDGER.md`.

## Main menu acceptance direction
- Approved menu direction uses the committed `Oster_Menu_BG` artwork.
- The gameplay world must not visibly bleed through the main-menu background.
- No large opaque swamp/green panel behind the menu.
- Only restrained local darkening/gradient on the left side is allowed for text readability.
- Branding direction: thin `OSTER`, large bold `CONFLICT`, subtitle `ОСТЕР • ГОЛОВНЕ МЕНЮ`.
- Main buttons: `СТАРТ`, `ЛОКАЛЬНА ГРА`, `МЕРЕЖЕВА ГРА`, `НАЛАШТУВАННЯ`, `ВИЙТИ З ГРИ`.
- No grey-shell flash or visible layout jumping during initial load/hover.

## Input / ESC regression
- Opening and closing ESC/pause UI must never leave movement or look input locked.
- Returning to gameplay must restore WASD, run, jump and Mouse X/Y.
- UI focus must target a focusable widget such as a button, never `SVerticalBox`.
- Repeated ESC cycles must not stack `IgnoreMoveInput` / `IgnoreLookInput` state.

## Development rules
- Modify the existing project. Do not restart from zero.
- Avoid regressions in working movement, shooting, bot behavior, networking and existing systems while fixing presentation.
- Keep commits focused.
- Do not commit generated folders: `Binaries`, `Intermediate`, `Saved`, `DerivedDataCache`.
- Large imported content belongs under Git LFS where configured.
- Keep documentation concise so it does not bloat the repository or working context.
- A committed change is `CODED_UNTESTED` until UE build/runtime/user playtest verifies it.
- Old code coordinates are not proof of geography. User-confirmed local knowledge and verified photo/map evidence take precedence.

## Work reports
Short milestone reports live in `OsterConflict/Docs/WorkReports/`. They record only useful milestone state. Routine progress and unresolved user requirements belong in the root `OSTER_CONFLICT_WORK_LEDGER.md`, not in a proliferation of one-off reports.
