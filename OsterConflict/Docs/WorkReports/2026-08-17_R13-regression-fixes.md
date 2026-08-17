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
- `10899a9` — restored the R13 neutral no-fog art-QA contract (`FogDensity=0`, `FogMaxOpacity=0`) after the lighting correction accidentally used an unsupported intermediate fog density.

## Test status
- 2026-08-17 15:54 local compile attempt stopped in the static verifier before C++ compilation.
- Root cause: `VERIFY_R11_VISUAL_FOUNDATION.py` accepts either the retained R11 fog baseline or the explicit R13 neutral no-fog mode; the lighting patch had set `FogDensity=0.0060` and `FogMaxOpacity=0.62`, matching neither contract.
- The verifier mismatch is fixed in `10899a9`.
- Next required test: pull the branch and run START_HERE option 1 again.

## Remaining visual work
- Verify approved menu presentation in the packaged/local test.
- Verify neutral lighting in-game.
- Verify first-person vehicle visibility.
- Remove remaining visible helper/debug primitive(s).
- Replace crude placeholder house presentation and validate interactive doors.
