# R13 regression repair — 2026-08-17

Branch: `r13-content-gameplay-pass`

## Reported defects
- Main menu background/presentation regression and UI focus spam.
- ESC menu could leave gameplay movement/look input locked.
- Outdoor scene had an excessive yellow/orange cast.
- Pickup first-person view was obstructed by placeholder vehicle geometry.
- R13 environment still exposed source/proxy presentation: almost no visible grass, flat yellow sky and blocky fallback characters/buildings.
- Legacy runtime UI polish continued fighting the dedicated R13 frontend/theme, leaving old opaque grey buttons and a bright white local-player text entry.

## Applied fixes
- `b0c2f1d` — transferred input, lighting and pickup-camera fixes into the R13 branch.
- `0923866` — corrected UE 5.8 menu focusability compile issue, moved focus from `SVerticalBox` to a button, prevented stacked input locks, and restored gameplay input on menu exit.
- `10899a9` — restored the R13 neutral no-fog art-QA contract (`FogDensity=0`, `FogMaxOpacity=0`) after the lighting correction accidentally used an unsupported intermediate fog density.
- `3286977` — restored the intended R13 pickup handling contract (`90 km/h`, `DriveForce=1600000`) while retaining the first-person camera/opaque-windshield fix.
- `8805dd2` + `4acd91d` — introduced the dedicated R13 runtime theme layer.
- `ed83f8e` — retired the legacy runtime menu/deployment/settings restyling path. It now keeps only non-visual input-context cleanup and chat positioning, so it can no longer overwrite the dedicated R13 frontend every tick.
- `02b1d96` — unified dedicated frontend fields, deployment and settings under one graphite / warm-sand UI language while preserving the main-menu button states from the approved menu specification. Local/network editable fields are no longer bright default-white controls.
- `0e7d9dd` — stopped re-enabling the primitive cube/cylinder Dashboard/SteeringWheel/Windshield on imported road vehicles and moved the driver camera away from those proxy parts. The huge black first-person obstruction was the resurrected primitive cockpit, not the imported pickup itself.
- `f923131` + `1dfc7e1` — added the committed PN foliage collection to cook and made PN grass the preferred whole-Oster grass source, with bounds-aware scale and 5x5 coverage per source grass tile. AdvancedVillage grass remains fallback.
- `d94330f` — replaced the mustard/yellow atmosphere setup with explicit Earth-like Rayleigh/Mie coefficients, neutral ground albedo, higher daytime sun angle and white skylight while retaining no-height-fog art-QA mode.
- `cd8ef95` — updated the R13 gameplay verifier for the revised foliage/daylight contract instead of preserving obsolete hard-coded atmosphere values.

## Test status
- 2026-08-17 16:13 local `START_HERE` option 1 completed successfully: `RESULT: PASS for Mode=Compile; every requested stage completed.`
- 2026-08-17 16:28 local `START_HERE` option 2 completed successfully: `RESULT: PASS for Mode=Full; every requested stage completed.`
- Gameplay smoke confirmed ESC -> return-to-game is stable. Mouse look, movement, sprint/jump and shooting continue working after repeated menu cycles.
- Second gameplay visual audit confirmed the world is no longer white-out, but it remains strongly yellow/orange, visible grass is inadequate, proxy characters/buildings are still prominent, and the imported pickup's first-person camera is obstructed by source cockpit primitives.
- Second UI audit confirmed the old runtime polish layer was still overriding parts of the dedicated R13 theme. That conflict is removed in `ed83f8e` and the remaining controls are re-themed in `02b1d96`.
- The large translucent angular shapes marked in red on the menu/deployment screenshots are not live gameplay bleed. They remain in identical screen-space positions over the same cinematic artwork, and the import pipeline explicitly converts the custom menu source to opaque 24-bit RGB before Unreal import. No runtime UI code draws those angular shapes. They are therefore part of the current custom `Oster_Menu_BG` source artwork. Removing them requires a clean version of the approved artwork rather than another opacity/z-order patch.

## Next test gate
- Pull the latest `r13-content-gameplay-pass`.
- Run `START_HERE` option 1 (Compile) because C++ and verifier contracts changed.
- If compile passes, run option 4 and verify only the changed visual areas: main/local/network/pause/settings/deployment theme, daylight/sky, visible grass, and pickup interior camera.

## Remaining visual work after this gate
- Replace the current custom menu background source with a clean approved copy if/when one is available; do not generate replacement artwork.
- Replace remaining primitive character presentation with the committed character/animation content where licensing and rig compatibility allow.
- Continue replacing crude whole-city house proxies beyond the current real-mesh bridge.
- Validate interactive doors after the environment pass.
