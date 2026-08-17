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
- `3286977` — restored the intended R13 pickup handling contract (`90 km/h`, `DriveForce=1600000`) while retaining the first-person camera/opaque-windshield fix.
- `dd80b1e` — restored the intended neutral R13 atmosphere (`MieScatteringScale=0.004`, white sun, no height fog) so the yellow/orange cast fix matches the gameplay-polish verifier.

## Test status
- 2026-08-17 15:54 local compile attempt stopped in `VERIFY_R11_VISUAL_FOUNDATION.py`; fixed by `10899a9`.
- 2026-08-17 16:01 the R11 verifier passed, then `VERIFY_R13_GAMEPLAY_POLISH.py` found two regressions introduced by the earlier broad patch: pickup speed/drive force and atmosphere scattering values.
- Both R13 gameplay-polish mismatches are corrected by `3286977` and `dd80b1e`.
- 2026-08-17 16:13 local `START_HERE` option 1 completed successfully: `RESULT: PASS for Mode=Compile; every requested stage completed.`
- 2026-08-17 16:28 local `START_HERE` option 2 completed successfully: `RESULT: PASS for Mode=Full; every requested stage completed.`
- Compile and full-validation gates are green. The next gate is the R13 local listen-server gameplay test (`START_HERE` option 4).

## Remaining visual work
- Verify approved menu presentation in the local gameplay test.
- Verify ESC -> return-to-game restores mouse look, WASD, sprint and jump across repeated cycles.
- Verify neutral lighting in-game.
- Verify first-person vehicle visibility.
- Remove remaining visible helper/debug primitive(s).
- Replace crude placeholder house presentation and validate interactive doors.
