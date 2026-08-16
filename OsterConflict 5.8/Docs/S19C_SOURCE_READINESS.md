# S19C source readiness

Status: SOURCE LAYER IMPLEMENTED / PACKAGED LOCALIZATION & CONTROLLER QA PENDING.

## Implemented in source
- Critical UMG user-facing static text uses gatherable `FText` via `LOCTEXT`/`NSLOCTEXT`.
- Native/source language for newly authored critical UI is Ukrainian (`uk-UA`).
- Frontend, Deployment, Chat, Sandbox Admin and Settings have deterministic default focus targets.
- Frontend/Deployment/Admin and Settings navigation uses explicit UMG navigation rules for primary button groups.
- `Gamepad_Special_Right` opens the same menu path as Escape.
- `Gamepad_FaceButton_Right` acts as Back/Cancel in the root UI path.
- Network/travel failure messages are gatherable `FText`.
- Voice Chat remains P2 and is not exposed as a fake P0/P1 Settings control.

## Localization target contract
`Config/Localization/Game_Gather.ini` defines `uk-UA` as native and `en` as a release target. Run `Scripts/S19C/RUN_LOCALIZATION_GATHER.ps1` with a real UE 5.8 installation to generate/update manifest/archive/PO/resources.

The source archive deliberately does **not** claim packaged English coverage or compiled LocRes PASS. Those require Unreal localization commandlets, translation catalog review/import, cook/package and the S19C packaged culture matrix.

## Still execution-pending
- Real UE GatherText run and LocRes generation.
- Complete English translation catalog and 100% critical-screen review.
- Cyrillic font/glyph audit in packaged build.
- 720p/1080p/1440p/4K + 16:10/ultrawide layout sweep.
- Controller-only packaged traversal of Frontend -> Settings -> Deployment -> Pause/error paths.
- Device-switch prompt polish and final UI art skin.
