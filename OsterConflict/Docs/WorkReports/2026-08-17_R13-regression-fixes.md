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
- `8805dd2` + `4acd91d` — added a small runtime UI theme layer: guaranteed opaque world blocker under the approved menu art, clean re-render of the existing menu backdrop above the old coarse gradient strips, softer left-side feather, and one graphite/warm-sand theme for menu/settings buttons, combo boxes, text and sliders.
- `7e29a1b` — corrected the gameplay white-out by restoring the readable source-scene directional-light intensity while retaining neutral white sun, low Mie scattering and no-fog R13 art-QA mode.

## Test status
- 2026-08-17 15:54 local compile attempt stopped in `VERIFY_R11_VISUAL_FOUNDATION.py`; fixed by `10899a9`.
- 2026-08-17 16:01 the R11 verifier passed, then `VERIFY_R13_GAMEPLAY_POLISH.py` found two regressions introduced by the earlier broad patch: pickup speed/drive force and atmosphere scattering values.
- Both R13 gameplay-polish mismatches are corrected by `3286977` and `dd80b1e`.
- 2026-08-17 16:13 local `START_HERE` option 1 completed successfully: `RESULT: PASS for Mode=Compile; every requested stage completed.`
- 2026-08-17 16:28 local `START_HERE` option 2 completed successfully: `RESULT: PASS for Mode=Full; every requested stage completed.`
- 2026-08-17 gameplay smoke: ESC -> return-to-game is stable; mouse look and movement continue working after return.
- Gameplay smoke exposed two visual blockers: the menu still allowed live scene content to show through / interfere with the approved artwork, and gameplay was almost completely white from overexposure. Settings controls also mixed default bright combo-box styling with the dark panel, producing black/unreadable text and no coherent visual language.
- Those three visual blockers are addressed by `8805dd2`, `4acd91d`, and `7e29a1b`; a new compile/full/gameplay test is required.

## Remaining visual work
- Re-test approved main menu after the opaque-backdrop/theme pass.
- Re-test settings readability and consistent graphite/warm-sand control states.
- Re-test gameplay exposure and neutral daylight.
- Verify first-person vehicle visibility.
- Remove remaining visible helper/debug primitive(s).
- Replace crude placeholder house presentation and validate interactive doors.
