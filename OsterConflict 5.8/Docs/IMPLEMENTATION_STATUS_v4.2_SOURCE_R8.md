# Oster Conflict — Source R8 status

Date: 2026-08-15

- First real Windows UE 5.8 UBT execution reached project TargetRules.
- R7 failed before C++ compilation because backward-compatible TargetRules changed properties shared with UnrealEditor.
- R8 fixes all four target files to UE 5.8 defaults: BuildSettingsVersion.V7, Unreal5_8 include order, C++20.
- Existing R7 logic/physics hardening is preserved unchanged.
- Next evidence required: rerun real `OsterConflictEditor Win64 Development` on the user's Windows machine.
