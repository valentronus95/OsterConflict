# Oster Conflict — R10 C++ Batch Fix QA

Date: 2026-08-15
Input evidence: real UE 5.8 `OsterConflictEditor Win64 Development` compile log from R9.

## What the real compile proved

- UnrealHeaderTool passed the previous R9 `uint16` trauma blocker.
- UnrealBuildTool entered the real C++ compile stage and attempted 65 actions.
- The build failed during C++ compilation, not asset loading/cook/runtime.
- The uploaded log contained **40 `error C...` lines**, but they collapse into a much smaller set of root causes.
- No error in this compile log reports a missing model, texture, material, skeletal mesh, animation, map package, or other content asset.

## Root causes fixed in R10

1. **C4458 shadowing under UE 5.8 / BuildSettings V7**
   - `Role` in `AOCAIController` and `AOCGameMode` shadowed `AActor::Role`.
   - `Character` in `AOCAIController` / `AOCPlayerController` shadowed controller members.
   - `Slot` in `UOCGameUIRootWidget` shadowed `UWidget::Slot`.
   - Renamed to explicit semantic names (`BotRole`, `SensedCharacter`, `ControlledCharacter`, `ButtonSlot`, `CanvasSlot`, `ComboSlot`).

2. **Missing Engineer role helper**
   - Several systems called `AOCPlayerState::IsEngineer()` but the method was absent.
   - Added `IsEngineer()` beside `IsMedic()`.

3. **Incomplete `UBoxComponent` type in armed vehicle attachment**
   - Added `Components/BoxComponent.h` and attached via `PhysicsBody.Get()`.

4. **Ambiguous `TSubclassOf<UDamageType>` ternary**
   - Replaced mixed `TSubclassOf` / `UClass*` conditional expression with explicit fallback assignment.

5. **Incomplete `FDamageEvent` in BTR**
   - Added `Engine/DamageEvents.h` before dereferencing `DamageEvent.DamageTypeClass`.

6. **Missing `WorldAudioComponent` members**
   - Added reflected component members to breakable window, door, gate and light headers to match their constructors/runtime calls.

7. **`TObjectPtr<UTextBlock>` passed to `UTextBlock*&`**
   - Kept the helper API on raw `UTextBlock*&` and bridged member `TObjectPtr` values through explicit raw temporary pointers.

8. **Incomplete `UCameraComponent` in grenade code**
   - Added `Camera/CameraComponent.h` before using camera component methods.

9. **UE 5.8 deprecated direct Actor networking fields**
   - Replaced project-owned writes to `NetUpdateFrequency`, `MinNetUpdateFrequency`, and `NetCullDistanceSquared` with UE 5.8 setter APIs.
   - These were warnings, not build blockers, but cleaning them now reduces future upgrade breakage and log noise.

10. **Verifier truth update**
   - Updated S18A static verifier to validate the new setter API rather than require deprecated field writes.
   - Added `VERIFY_R10_CXX_BATCH_FIX.py` and included it in both source verifier runners.

## Static regression after patch

- Main verifier chain: PASS.
- Internal source verifier chain: **28/28 PASS**.
- R10 C++ batch-fix checks: **26/26 PASS**.
- C++ source/header inventory remains 129 files.
- RPC static audit remains 44 declarations.

## What remains unknown until the user's next UE compile

R10 is a source correction based directly on the R9 compiler output. It is **not** claimed as UE compile PASS because UnrealBuildTool cannot run in this environment. The next Windows run may expose the next C++/link layer after these 40 reported errors are removed.

## Next action on Windows

Extract R10 to a new folder and run:

`BUILD_EDITOR_LAUNCHER_UE58.cmd`

If it fails, return the new full console output / UBT log. Do not diagnose warnings about models unless the compiler/cooker actually reports a missing package/asset path.
