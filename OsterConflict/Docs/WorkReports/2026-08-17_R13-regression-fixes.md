# R13 regression repair — 2026-08-17

Branch: `r13-content-gameplay-pass`

## Reported defects
- Main menu background/presentation regression and UI focus spam.
- ESC menu could leave gameplay movement/look input locked.
- Outdoor scene had an excessive yellow/orange cast.
- Pickup first-person view was obstructed by placeholder vehicle geometry.

## Applied fixes
- `b0c2f1d` — transferred input, lighting and pickup-camera fixes into the R13 branch.
- `0923866` — corrected UE 5.8 menu focusability compile issue, moved focus from `SVerticalBox` to a button, prevented stacked input locks, and restored gameplay input on menu exit.

## Test status
The user committed the R13 content/assets locally and has a clean GitHub Desktop working tree. A new local compile/test is required after pulling the latest branch commits.

## Remaining visual work
- Verify approved menu presentation in the packaged/local test.
- Verify neutral lighting in-game.
- Verify first-person vehicle visibility.
- Remove remaining visible helper/debug primitive(s).
- Replace crude placeholder house presentation and validate interactive doors.
