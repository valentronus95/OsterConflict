# Oster Conflict — R8 UE 5.8 Target Rules Build Fix

Date: 2026-08-15

## Real UnrealBuildTool failure fixed

First real Windows UE 5.8 build reached UnrealBuildTool and failed before compiling game C++ because `OsterConflictEditor` inherited backward-compatible target defaults that conflict with shared UnrealEditor build products.

Observed UBT failure:

- `OsterConflictEditor modifies the values of properties ... This is not allowed, as OsterConflictEditor has build products in common with UnrealEditor.`
- UBT explicitly requested `DefaultBuildSettings = BuildSettingsVersion.V7`.
- UBT explicitly requested `IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8`.
- UBT reported C++17 is no longer supported and requested C++20.

## Fix

All four project targets now explicitly use the UE 5.8 contract:

- `DefaultBuildSettings = BuildSettingsVersion.V7;`
- `IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;`
- `CppStandard = CppStandardVersion.Cpp20;`

Updated targets:

- OsterConflict.Target.cs
- OsterConflictEditor.Target.cs
- OsterConflictClient.Target.cs
- OsterConflictServer.Target.cs

We intentionally did **not** use `TargetBuildEnvironment.Unique` or `bOverrideBuildEnvironment=true` as a workaround. The project now adopts the UE 5.8 target defaults instead of forcing legacy settings.

## Verification

- Existing R7 source verifier chain: PASS after target update.
- New `VERIFY_R8_UE58_TARGETS.py`: verifies all four target files.
- Real UE compilation must now be rerun on the user's Windows PC. The next compiler/UHT error, if any, is expected to be a deeper source issue rather than the old TargetRules blocker.
