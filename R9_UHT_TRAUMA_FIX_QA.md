# Oster Conflict — R9 UHT Trauma Fix

Date: 2026-08-15

## Real UE 5.8 error fixed

User-side UnrealHeaderTool stopped on:

`OCTraumaTypes.h(72): Type 'uint16' is not supported by blueprint. Struct: FOCReplicatedTraumaEvent Property: Sequence`

## Root cause

`FOCReplicatedTraumaEvent` is `USTRUCT(BlueprintType)` and `Sequence` was exposed with `UPROPERTY(BlueprintReadOnly)` as `uint16`. UE 5.8 UHT rejects that Blueprint-exposed integer type.

## Fix

- `FOCReplicatedTraumaEvent::Sequence`: `uint16` -> `int32`.
- `UOCCombatVisualComponent::ServerSequence`: `uint16` -> `int32`.
- `UOCCombatVisualComponent::LastRenderedSequence`: `uint16` -> `int32`.
- Added a project-wide verifier that rejects Blueprint-exposed `uint16 UPROPERTY` declarations.
- No gameplay/network semantic change intended; this sequence is presentation-event deduplication only.

## Status

Static/source regression: PASS.
Real UE 5.8 UHT/C++ compile: must be rerun on the user's Windows machine.
