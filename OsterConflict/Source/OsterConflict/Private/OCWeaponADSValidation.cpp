#include "OCFirstPersonWeaponPresentationSubsystem.h"

#include "OCCharacter.h"
#include "OCWeaponBase.h"
#include "OCWeaponPresentationProfiles.h"

#include "Camera/CameraComponent.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"

namespace
{
    TAutoConsoleVariable<int32> CVarOCWeaponADSDebug(
        TEXT("oc.Weapon.ADS.Debug"),
        0,
        TEXT("Draw one-shot camera and authored sight-axis lines when entering ADS. 0=off, 1=on."),
        ECVF_Default);
}

void UOCFirstPersonWeaponPresentationSubsystem::ValidateADSAlignment(
    AOCCharacter& Character,
    AOCWeaponBase& Weapon,
    UPrimitiveComponent* ProductionVisual,
    const FOCFirstPersonWeaponProfile& Profile) const
{
    UCameraComponent* Camera = Character.GetFirstPersonCamera();
    const FString WeaponId = Weapon.GetWeaponId().ToString();
    if (!Camera)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_ADS_ALIGNMENT_FAIL weapon=%s reason=no_first_person_camera"), *WeaponId);
        return;
    }

    if (!Profile.bADSCalibrated)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("PASS45_ADS_PROFILE_UNCALIBRATED weapon=%s declared_profile=1 no_fake_ready=1"), *WeaponId);
        return;
    }

    if (!ProductionVisual)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_ADS_ALIGNMENT_FAIL weapon=%s reason=no_production_visual calibrated=1"), *WeaponId);
        return;
    }

    FVector SightOrigin = FVector::ZeroVector;
    FVector SightDirection = FVector::ZeroVector;
    FString SightSource;

    if (!Profile.ADSOpticSocket.IsNone())
    {
        if (!ProductionVisual->DoesSocketExist(Profile.ADSOpticSocket))
        {
            UE_LOG(LogTemp, Error,
                TEXT("PASS45_ADS_ALIGNMENT_FAIL weapon=%s reason=missing_optic_socket socket=%s calibrated=1"),
                *WeaponId, *Profile.ADSOpticSocket.ToString());
            return;
        }

        const FTransform OpticTransform = ProductionVisual->GetSocketTransform(Profile.ADSOpticSocket, RTS_World);
        SightOrigin = OpticTransform.GetLocation();
        SightDirection = OpticTransform.GetRotation().RotateVector(FVector::ForwardVector).GetSafeNormal();
        SightSource = FString::Printf(TEXT("optic:%s"), *Profile.ADSOpticSocket.ToString());
    }
    else
    {
        if (Profile.ADSRearSightSocket.IsNone() || Profile.ADSFrontSightSocket.IsNone())
        {
            UE_LOG(LogTemp, Error,
                TEXT("PASS45_ADS_ALIGNMENT_FAIL weapon=%s reason=calibrated_profile_missing_sight_socket_names"), *WeaponId);
            return;
        }
        if (!ProductionVisual->DoesSocketExist(Profile.ADSRearSightSocket) ||
            !ProductionVisual->DoesSocketExist(Profile.ADSFrontSightSocket))
        {
            UE_LOG(LogTemp, Error,
                TEXT("PASS45_ADS_ALIGNMENT_FAIL weapon=%s reason=missing_iron_sight_socket rear=%s front=%s calibrated=1"),
                *WeaponId, *Profile.ADSRearSightSocket.ToString(), *Profile.ADSFrontSightSocket.ToString());
            return;
        }

        const FVector Rear = ProductionVisual->GetSocketLocation(Profile.ADSRearSightSocket);
        const FVector Front = ProductionVisual->GetSocketLocation(Profile.ADSFrontSightSocket);
        SightOrigin = Rear;
        SightDirection = (Front - Rear).GetSafeNormal();
        SightSource = FString::Printf(TEXT("iron:%s->%s"),
            *Profile.ADSRearSightSocket.ToString(), *Profile.ADSFrontSightSocket.ToString());
    }

    if (SightDirection.IsNearlyZero())
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_ADS_ALIGNMENT_FAIL weapon=%s reason=zero_sight_direction calibrated=1"), *WeaponId);
        return;
    }

    const FVector CameraOrigin = Camera->GetComponentLocation();
    const FVector CameraDirection = Camera->GetForwardVector().GetSafeNormal();
    const float Dot = FMath::Clamp(FVector::DotProduct(CameraDirection, SightDirection), -1.0f, 1.0f);
    const float AngularErrorDegrees = FMath::RadiansToDegrees(FMath::Acos(Dot));
    const float CameraToSightLineCm = FVector::CrossProduct(CameraOrigin - SightOrigin, SightDirection).Size();

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_ADS_ALIGNMENT_SAMPLE weapon=%s source=%s calibrated=1 angle_deg=%.3f camera_line_offset_cm=%.2f runtime_visual_acceptance=pending"),
        *WeaponId, *SightSource, AngularErrorDegrees, CameraToSightLineCm);

#if !UE_BUILD_SHIPPING
    if (CVarOCWeaponADSDebug.GetValueOnGameThread() > 0 && GetWorld())
    {
        DrawDebugLine(GetWorld(), CameraOrigin, CameraOrigin + CameraDirection * 2000.0f,
            FColor::Green, false, 1.5f, 0, 1.5f);
        DrawDebugLine(GetWorld(), SightOrigin, SightOrigin + SightDirection * 2000.0f,
            FColor::Cyan, false, 1.5f, 0, 1.5f);
    }
#endif
}
