#pragma once

#include "CoreMinimal.h"

/**
 * Camera-space presentation values for one weapon id.
 *
 * R14 starts from the legacy shared transform so this refactor does not invent visual
 * calibration data. Each weapon is explicitly marked uncalibrated until it has been
 * inspected in the UE 5.8 visual Sandbox and its own values are approved.
 */
struct FOCFirstPersonWeaponProfile
{
    FName WeaponId = NAME_None;

    FVector CameraLocation = FVector(38.0f, 12.0f, -14.0f);
    FRotator CameraRotation = FRotator::ZeroRotator;

    FVector ArmsBaseOffset = FVector::ZeroVector;
    FRotator ArmsBaseRotationOffset = FRotator::ZeroRotator;

    FVector ADSWeaponOffset = FVector(-5.5f, -9.0f, 4.0f);
    FRotator ADSWeaponRotationOffset = FRotator::ZeroRotator;
    FVector ADSArmsOffset = FVector(-2.0f, -3.0f, 1.5f);
    FRotator ADSArmsRotationOffset = FRotator::ZeroRotator;

    FVector RecoilWeaponLocation = FVector(-4.5f, 0.0f, 1.4f);
    FRotator RecoilWeaponRotation = FRotator(-4.0f, 0.0f, 0.8f);
    FVector RecoilArmsLocation = FVector(-2.0f, 0.0f, 0.6f);
    FRotator RecoilArmsRotation = FRotator(-2.0f, 0.0f, 0.4f);

    FVector ReloadWeaponLocation = FVector(-8.0f, 3.0f, -11.0f);
    FRotator ReloadWeaponRotation = FRotator(-12.0f, 4.0f, 19.0f);
    FVector ReloadArmsLocation = FVector(-5.0f, 2.0f, -7.0f);
    FRotator ReloadArmsRotation = FRotator(-8.0f, 3.0f, 11.0f);

    bool bGripCalibrated = false;
};

/** Returns an explicit profile for every currently implemented weapon id. */
OSTERCONFLICT_API FOCFirstPersonWeaponProfile OCResolveFirstPersonWeaponProfile(FName WeaponId);

/** Used by R14 validation/diagnostics to prove the weapon id has a declared profile. */
OSTERCONFLICT_API bool OCHasDeclaredFirstPersonWeaponProfile(FName WeaponId);
