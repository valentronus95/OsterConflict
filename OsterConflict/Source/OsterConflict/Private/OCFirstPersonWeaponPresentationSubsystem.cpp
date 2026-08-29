#include "OCFirstPersonWeaponPresentationSubsystem.h"

#include "OCCharacter.h"
#include "OCGameMode.h"
#include "OCWeaponAnimationProfiles.h"
#include "OCWeaponAudioComponent.h"
#include "OCWeaponBase.h"
#include "OCWeaponPresentationProfiles.h"

#include "Animation/AnimSequence.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

namespace
{
    const FName ProductionWeaponVisualTag(TEXT("OC_ProductionWeaponVisual"));
}

bool UOCFirstPersonWeaponPresentationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCFirstPersonWeaponPresentationSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (InWorld.GetNetMode() == NM_DedicatedServer) return;

    RifleIdleAnimation = LoadObject<UAnimSequence>(nullptr,
        TEXT("/Game/SampleAnimationPack/Animations/Rifle/AS_F_Rifle_Idle_01.AS_F_Rifle_Idle_01"));
    RifleADSIdleAnimation = LoadObject<UAnimSequence>(nullptr,
        TEXT("/Game/SampleAnimationPack/Animations/Rifle/AS_F_ADS_Rifle_Idle_01.AS_F_ADS_Rifle_Idle_01"));
    AKFireAnimation = LoadObject<UAnimSequence>(nullptr,
        TEXT("/Game/AK-47/Animations/AK-47_Fire_W.AK-47_Fire_W"));
    AKReloadAnimation = LoadObject<UAnimSequence>(nullptr,
        TEXT("/Game/AK-47/Animations/AK-47_Reload_W.AK-47_Reload_W"));
}

TStatId UOCFirstPersonWeaponPresentationSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCFirstPersonWeaponPresentationSubsystem, STATGROUP_Tickables);
}

void UOCFirstPersonWeaponPresentationSubsystem::Tick(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World || World->GetNetMode() == NM_DedicatedServer) return;

    if (const AOCGameMode* GameMode = World->GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    APlayerController* LocalPC = World->GetFirstPlayerController();
    AOCCharacter* Character = LocalPC ? Cast<AOCCharacter>(LocalPC->GetPawn()) : nullptr;
    if (Character && Character->IsLocallyControlled())
    {
        const TWeakObjectPtr<AOCCharacter> CharacterKey(Character);
        if (!Character->IsInVehicle())
        {
            UpdateLocalCharacter(*Character, DeltaTime);
        }
        else if (FOCFirstPersonWeaponState* ExistingState = StateByCharacter.Find(CharacterKey))
        {
            RestorePresentationState(*Character, *ExistingState);
            StateByCharacter.Remove(CharacterKey);
        }

        if (!bLocalPawnFastPathLogged)
        {
            bLocalPawnFastPathLogged = true;
            UE_LOG(LogTemp, Display,
                TEXT("PASS39_FP_LOCAL_PAWN_FAST_PATH_READY world_character_scan=0 local_pawn_only=1"));
        }
    }

    for (auto It = StateByCharacter.CreateIterator(); It; ++It)
    {
        if (!It.Key().IsValid() || It.Key().Get() != Character) It.RemoveCurrent();
    }
}

UPrimitiveComponent* UOCFirstPersonWeaponPresentationSubsystem::FindProductionWeaponVisual(AOCWeaponBase& Weapon) const
{
    TArray<UPrimitiveComponent*> Components;
    Weapon.GetComponents<UPrimitiveComponent>(Components);
    for (UPrimitiveComponent* Component : Components)
    {
        if (Component && Component->ComponentHasTag(ProductionWeaponVisualTag))
        {
            return Component;
        }
    }
    return nullptr;
}

USkeletalMeshComponent* UOCFirstPersonWeaponPresentationSubsystem::FindProductionSkeletalWeaponVisual(AOCWeaponBase& Weapon) const
{
    return Cast<USkeletalMeshComponent>(FindProductionWeaponVisual(Weapon));
}

void UOCFirstPersonWeaponPresentationSubsystem::RestorePresentationState(AOCCharacter& Character,
    FOCFirstPersonWeaponState& State)
{
    if (AOCWeaponBase* PreviousWeapon = State.Weapon.Get())
    {
        if (State.bWeaponAnimationActive)
        {
            if (USkeletalMeshComponent* Visual = FindProductionSkeletalWeaponVisual(*PreviousWeapon))
            {
                if (Visual->GetAnimationMode() != EAnimationMode::AnimationBlueprint)
                {
                    Visual->SetAnimation(nullptr);
                }
            }
        }

        if (!PreviousWeapon->IsWorldPickup())
        {
            PreviousWeapon->SetActorRelativeLocation(State.BaseWeaponLocation);
            PreviousWeapon->SetActorRelativeRotation(State.BaseWeaponRotation);
        }
    }

    if (USkeletalMeshComponent* Arms = Character.GetFirstPersonArms())
    {
        Arms->SetRelativeLocation(State.BaseArmsLocation);
        Arms->SetRelativeRotation(State.BaseArmsRotation);
        if (State.bRiflePoseApplied && Arms->GetAnimationMode() != EAnimationMode::AnimationBlueprint)
        {
            Arms->SetAnimation(nullptr);
        }
    }

    State.bWeaponAnimationActive = false;
    State.bAuthoredManualActionActive = false;
    State.bRiflePoseApplied = false;
    State.bWasActionCycling = false;
    State.bWasAiming = false;
    State.ActionCycleStartTime = 0.0;
    State.RecoilAlpha = 0.0f;
}

bool UOCFirstPersonWeaponPresentationSubsystem::PlayWeaponAnimation(AOCWeaponBase& Weapon, UAnimSequence* Sequence,
    FOCFirstPersonWeaponState& State, double ResetDelaySeconds)
{
    if (!Sequence) return false;
    USkeletalMeshComponent* Visual = FindProductionSkeletalWeaponVisual(Weapon);
    USkeletalMesh* Mesh = Visual ? Visual->GetSkeletalMeshAsset() : nullptr;
    if (!Visual || !Mesh || !Sequence->GetSkeleton() || Sequence->GetSkeleton() != Mesh->GetSkeleton()) return false;

    if (Visual->GetAnimationMode() == EAnimationMode::AnimationBlueprint && Visual->GetAnimClass()) return false;

    Visual->PlayAnimation(Sequence, false);
    State.bWeaponAnimationActive = true;
    State.WeaponAnimationResetTime = GetWorld() ? GetWorld()->GetTimeSeconds() + FMath::Max(0.05, ResetDelaySeconds) : 0.0;
    return true;
}

void UOCFirstPersonWeaponPresentationSubsystem::ApplyArmsPose(AOCCharacter& Character,
    FOCFirstPersonWeaponState& State, bool bADS)
{
    USkeletalMeshComponent* Arms = Character.GetFirstPersonArms();
    USkeletalMesh* ArmsMesh = Arms ? Arms->GetSkeletalMeshAsset() : nullptr;
    if (!Arms || !ArmsMesh) return;

    if (Arms->GetAnimationMode() == EAnimationMode::AnimationBlueprint && Arms->GetAnimClass()) return;

    UAnimSequence* Desired = bADS ? RifleADSIdleAnimation.Get() : RifleIdleAnimation.Get();
    if (!Desired || !Desired->GetSkeleton() || Desired->GetSkeleton() != ArmsMesh->GetSkeleton())
    {
        return;
    }

    if (!State.bRiflePoseApplied || State.bADSArmsPose != bADS)
    {
        Arms->PlayAnimation(Desired, true);
        State.bRiflePoseApplied = true;
        State.bADSArmsPose = bADS;
    }
}

void UOCFirstPersonWeaponPresentationSubsystem::ValidateADSAlignment(AOCCharacter& Character,
    AOCWeaponBase& Weapon, UPrimitiveComponent* ProductionVisual, const FOCFirstPersonWeaponProfile& Profile) const
{
    const FString WeaponName = Weapon.GetWeaponId().ToString();
    if (!Profile.bADSCalibrated)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("PASS45_ADS_PROFILE_UNCALIBRATED weapon=%s calibrated=0 mutation=0 runtime_visual_acceptance=pending"),
            *WeaponName);
        return;
    }

    if (!ProductionVisual)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_ADS_ALIGNMENT_FAIL weapon=%s reason=missing_production_visual calibrated=1 mutation=0 runtime_visual_acceptance=pending"),
            *WeaponName);
        return;
    }

    APlayerController* PlayerController = Cast<APlayerController>(Character.GetController());
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_ADS_ALIGNMENT_FAIL weapon=%s reason=missing_local_player_controller calibrated=1 mutation=0 runtime_visual_acceptance=pending"),
            *WeaponName);
        return;
    }

    FVector SightForward = FVector::ZeroVector;
    const TCHAR* ReferenceMode = TEXT("none");
    if (!Profile.ADSOpticSocket.IsNone())
    {
        if (!ProductionVisual->DoesSocketExist(Profile.ADSOpticSocket))
        {
            UE_LOG(LogTemp, Error,
                TEXT("PASS45_ADS_ALIGNMENT_FAIL weapon=%s reason=missing_optic_socket socket=%s calibrated=1 mutation=0 runtime_visual_acceptance=pending"),
                *WeaponName, *Profile.ADSOpticSocket.ToString());
            return;
        }

        SightForward = ProductionVisual->GetSocketTransform(Profile.ADSOpticSocket, RTS_World)
            .GetRotation().GetForwardVector().GetSafeNormal();
        ReferenceMode = TEXT("optic_socket");
    }
    else
    {
        if (Profile.ADSRearSightSocket.IsNone() || Profile.ADSFrontSightSocket.IsNone())
        {
            UE_LOG(LogTemp, Error,
                TEXT("PASS45_ADS_ALIGNMENT_FAIL weapon=%s reason=missing_authored_sight_reference calibrated=1 mutation=0 runtime_visual_acceptance=pending"),
                *WeaponName);
            return;
        }
        if (!ProductionVisual->DoesSocketExist(Profile.ADSRearSightSocket) ||
            !ProductionVisual->DoesSocketExist(Profile.ADSFrontSightSocket))
        {
            UE_LOG(LogTemp, Error,
                TEXT("PASS45_ADS_ALIGNMENT_FAIL weapon=%s reason=missing_iron_sight_socket rear=%s front=%s calibrated=1 mutation=0 runtime_visual_acceptance=pending"),
                *WeaponName, *Profile.ADSRearSightSocket.ToString(), *Profile.ADSFrontSightSocket.ToString());
            return;
        }

        const FVector Rear = ProductionVisual->GetSocketLocation(Profile.ADSRearSightSocket);
        const FVector Front = ProductionVisual->GetSocketLocation(Profile.ADSFrontSightSocket);
        SightForward = (Front - Rear).GetSafeNormal();
        ReferenceMode = TEXT("rear_front_sockets");
    }

    if (SightForward.IsNearlyZero())
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_ADS_ALIGNMENT_FAIL weapon=%s reason=degenerate_sight_axis calibrated=1 mutation=0 runtime_visual_acceptance=pending"),
            *WeaponName);
        return;
    }

    FVector ViewLocation = FVector::ZeroVector;
    FRotator ViewRotation = FRotator::ZeroRotator;
    PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
    const FVector ViewForward = ViewRotation.Vector().GetSafeNormal();
    const float AlignmentDot = FMath::Clamp(FVector::DotProduct(ViewForward, SightForward), -1.0f, 1.0f);
    const float AngularErrorDeg = FMath::RadiansToDegrees(FMath::Acos(AlignmentDot));

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_ADS_ALIGNMENT_SAMPLE weapon=%s reference=%s angle_error_deg=%.3f calibrated=1 mutation=0 runtime_visual_acceptance=pending"),
        *WeaponName, ReferenceMode, AngularErrorDeg);
}

void UOCFirstPersonWeaponPresentationSubsystem::UpdateLocalCharacter(AOCCharacter& Character, float DeltaTime)
{
    AOCWeaponBase* Weapon = Character.GetCurrentWeapon();
    USkeletalMeshComponent* Arms = Character.GetFirstPersonArms();
    const TWeakObjectPtr<AOCCharacter> CharacterKey(&Character);

    if (!Weapon || Weapon->IsWorldPickup() || !Arms)
    {
        if (FOCFirstPersonWeaponState* ExistingState = StateByCharacter.Find(CharacterKey))
        {
            RestorePresentationState(Character, *ExistingState);
            StateByCharacter.Remove(CharacterKey);
        }
        return;
    }

    FOCFirstPersonWeaponState* ExistingState = StateByCharacter.Find(CharacterKey);
    if (ExistingState && ExistingState->Weapon.Get() != Weapon)
    {
        RestorePresentationState(Character, *ExistingState);
        StateByCharacter.Remove(CharacterKey);
        ExistingState = nullptr;
    }

    FOCFirstPersonWeaponState& State = StateByCharacter.FindOrAdd(CharacterKey);
    const double Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
    const FName WeaponId = Weapon->GetWeaponId();
    const bool bDeclaredProfile = OCHasDeclaredFirstPersonWeaponProfile(WeaponId);
    const FOCFirstPersonWeaponProfile Profile = OCResolveFirstPersonWeaponProfile(WeaponId);

    if (!ExistingState)
    {
        State = FOCFirstPersonWeaponState();
        State.Weapon = Weapon;
        State.LastAmmo = Weapon->GetAmmoInMagazine();
        State.bWasReloading = Weapon->IsReloading();
        State.bWasActionCycling = false;
        State.bWasAiming = false;
        State.ReloadStartTime = Now;

        State.BaseWeaponLocation = Profile.CameraLocation;
        State.BaseWeaponRotation = Profile.CameraRotation;
        State.BaseArmsLocation = Arms->GetRelativeLocation() + Profile.ArmsBaseOffset;
        State.BaseArmsRotation = Arms->GetRelativeRotation() + Profile.ArmsBaseRotationOffset;

        UPrimitiveComponent* ProductionVisual = FindProductionWeaponVisual(*Weapon);
        if (!ProductionVisual)
        {
            UE_LOG(LogTemp, Error,
                TEXT("Pass 8 first-person presentation has no production visual for weapon id %s."),
                *WeaponId.ToString());
        }
        else if (Cast<UStaticMeshComponent>(ProductionVisual))
        {
            UE_LOG(LogTemp, Verbose,
                TEXT("Pass 8 first-person presentation accepted StaticMesh production visual for %s."),
                *WeaponId.ToString());
        }

        if (!bDeclaredProfile)
        {
            UE_LOG(LogTemp, Error,
                TEXT("R14 first-person presentation has no declared grip profile for weapon id %s."),
                *WeaponId.ToString());
        }
        else if (!Profile.bGripCalibrated)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("R14 first-person grip profile is UNCALIBRATED for %s; legacy camera-space baseline is preserved until visual approval."),
                *WeaponId.ToString());
        }
    }

    const bool bRequestedADS = Character.IsAiming();
    if (bRequestedADS && !State.bWasAiming && bDeclaredProfile)
    {
        ValidateADSAlignment(Character, *Weapon, FindProductionWeaponVisual(*Weapon), Profile);
        if (!Profile.bADSCalibrated)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("PASS45_ADS_PRESENTATION_FAIL_CLOSED weapon=%s requested_ads=1 calibrated=0 hip_transform_preserved=1 runtime_visual_acceptance=pending"),
                *WeaponId.ToString());
        }
    }
    State.bWasAiming = bRequestedADS;

    const bool bADS = bRequestedADS && Profile.bADSCalibrated;

    const EOCWeaponClass WeaponClass = Weapon->GetWeaponClass();
    const bool bLongGun = WeaponClass != EOCWeaponClass::Pistol;
    if (bLongGun)
    {
        ApplyArmsPose(Character, State, bADS);
    }
    else if (State.bRiflePoseApplied && Arms->GetAnimationMode() != EAnimationMode::AnimationBlueprint)
    {
        Arms->SetAnimation(nullptr);
        State.bRiflePoseApplied = false;
    }

    const int32 CurrentAmmo = Weapon->GetAmmoInMagazine();
    if (State.LastAmmo != INDEX_NONE && CurrentAmmo < State.LastAmmo && !Weapon->IsReloading())
    {
        State.RecoilAlpha = 1.0f;
        if (WeaponId == FName(TEXT("OC_AR1")))
        {
            PlayWeaponAnimation(*Weapon, AKFireAnimation, State, 0.11);
        }
    }
    State.LastAmmo = CurrentAmmo;

    const bool bReloading = Weapon->IsReloading();
    if (bReloading && !State.bWasReloading)
    {
        State.ReloadStartTime = Now;
        if (WeaponId == FName(TEXT("OC_AR1")))
        {
            PlayWeaponAnimation(*Weapon, AKReloadAnimation, State, Weapon->GetReloadDuration());
        }
    }

    if (!bReloading && State.bWasReloading && State.bWeaponAnimationActive)
    {
        if (USkeletalMeshComponent* Visual = FindProductionSkeletalWeaponVisual(*Weapon))
        {
            if (Visual->GetAnimationMode() != EAnimationMode::AnimationBlueprint)
            {
                Visual->SetAnimation(nullptr);
            }
        }
        State.bWeaponAnimationActive = false;
    }
    State.bWasReloading = bReloading;

    if (State.bWeaponAnimationActive && !bReloading && Now >= State.WeaponAnimationResetTime)
    {
        if (USkeletalMeshComponent* Visual = FindProductionSkeletalWeaponVisual(*Weapon))
        {
            if (Visual->GetAnimationMode() != EAnimationMode::AnimationBlueprint)
            {
                Visual->SetAnimation(nullptr);
            }
        }
        State.bWeaponAnimationActive = false;
    }

    const bool bActionCycling = Weapon->IsActionCycling();
    if (bActionCycling && !State.bWasActionCycling)
    {
        State.ActionCycleStartTime = Now;
        State.bAuthoredManualActionActive = false;
        const EOCWeaponActionType ActionType = Weapon->GetWeaponActionType();
        if (UOCWeaponAudioComponent* Audio = Weapon->GetWeaponAudioComponent())
        {
            const int32 EventSeed = CurrentAmmo * 31 + static_cast<int32>(ActionType) * 101;
            Audio->HandleStateEventLocal(EOCWeaponAudioEvent::ManualActionCycle, Weapon->GetActorLocation(), EventSeed);
        }

        const FOCWeaponAnimationProfile AnimationProfile = OCResolveWeaponAnimationProfile(WeaponId);
        if (AnimationProfile.HasManualActionAnimation())
        {
            UAnimSequence* ManualActionSequence = LoadObject<UAnimSequence>(
                nullptr, *AnimationProfile.ManualActionAnimationObjectPath);
            const double ResetDelay = FMath::Max(0.05f, Weapon->GetManualActionCycleDuration());
            if (PlayWeaponAnimation(*Weapon, ManualActionSequence, State, ResetDelay))
            {
                State.bAuthoredManualActionActive = true;
                UE_LOG(LogTemp, Display,
                    TEXT("PASS45_MANUAL_ACTION_AUTHORED_SOURCE_BRIDGE_READY weapon=%s action=%s path=%s replicated_gate=1 second_gameplay_timer=0 runtime_acceptance=0"),
                    *WeaponId.ToString(), *UEnum::GetValueAsString(ActionType),
                    *AnimationProfile.ManualActionAnimationObjectPath);
            }
            else
            {
                UE_LOG(LogTemp, Error,
                    TEXT("PASS45_MANUAL_ACTION_AUTHORED_SOURCE_BRIDGE_FAIL weapon=%s action=%s path=%s reason=load_or_skeleton_or_animation_mode runtime_acceptance=0"),
                    *WeaponId.ToString(), *UEnum::GetValueAsString(ActionType),
                    *AnimationProfile.ManualActionAnimationObjectPath);
            }
        }

        if (!State.bAuthoredManualActionActive)
        {
            // This path is deliberately not production READY. It moves the whole local weapon/arms transform
            // and cannot prove authored bolt/pump/lever moving-part animation. Keep it as a visible fallback until
            // an accepted skeletal/moving-part asset is wired and verified in local UE 5.8.
            UE_LOG(LogTemp, Warning,
                TEXT("PASS45_MANUAL_ACTION_PROCEDURAL_FALLBACK_ACTIVE weapon=%s action=%s cue_declared=%d replicated_gate=1 whole_transform_only=1 authored_moving_part=0 second_gameplay_timer=0 runtime_acceptance=0"),
                *WeaponId.ToString(), *UEnum::GetValueAsString(ActionType), Profile.bManualActionCueDeclared ? 1 : 0);
            UE_LOG(LogTemp, Warning,
                TEXT("PASS45_MANUAL_ACTION_AUTHORED_CONTENT_GAP weapon=%s action=%s authored_moving_part=0 procedural_fallback=1 runtime_acceptance=0"),
                *WeaponId.ToString(), *UEnum::GetValueAsString(ActionType));
        }
    }
    else if (!bActionCycling && State.bWasActionCycling)
    {
        State.ActionCycleStartTime = 0.0;
        State.bAuthoredManualActionActive = false;
    }
    State.bWasActionCycling = bActionCycling;

    State.RecoilAlpha = FMath::FInterpTo(State.RecoilAlpha, 0.0f, DeltaTime, 19.0f);

    FVector WeaponLocation = State.BaseWeaponLocation;
    FRotator WeaponRotation = State.BaseWeaponRotation;
    FVector ArmsLocation = State.BaseArmsLocation;
    FRotator ArmsRotation = State.BaseArmsRotation;

    if (bADS)
    {
        WeaponLocation += Profile.ADSWeaponOffset;
        WeaponRotation += Profile.ADSWeaponRotationOffset;
        ArmsLocation += Profile.ADSArmsOffset;
        ArmsRotation += Profile.ADSArmsRotationOffset;
    }

    WeaponLocation += Profile.RecoilWeaponLocation * State.RecoilAlpha;
    WeaponRotation += Profile.RecoilWeaponRotation * State.RecoilAlpha;
    ArmsLocation += Profile.RecoilArmsLocation * State.RecoilAlpha;
    ArmsRotation += Profile.RecoilArmsRotation * State.RecoilAlpha;

    if (bReloading)
    {
        const float Duration = FMath::Max(0.05f, Weapon->GetReloadDuration());
        const float Alpha = FMath::Clamp(static_cast<float>((Now - State.ReloadStartTime) / Duration), 0.0f, 1.0f);
        const float Arc = FMath::Sin(Alpha * PI);
        WeaponLocation += Profile.ReloadWeaponLocation * Arc;
        WeaponRotation += Profile.ReloadWeaponRotation * Arc;
        ArmsLocation += Profile.ReloadArmsLocation * Arc;
        ArmsRotation += Profile.ReloadArmsRotation * Arc;
    }

    if (bActionCycling && Profile.bManualActionCueDeclared && !State.bAuthoredManualActionActive)
    {
        const float Duration = FMath::Max(0.05f, Weapon->GetManualActionCycleDuration());
        const float Alpha = FMath::Clamp(static_cast<float>((Now - State.ActionCycleStartTime) / Duration), 0.0f, 1.0f);
        const float Arc = FMath::Sin(Alpha * PI);
        WeaponLocation += Profile.ManualActionWeaponLocation * Arc;
        WeaponRotation += Profile.ManualActionWeaponRotation * Arc;
        ArmsLocation += Profile.ManualActionArmsLocation * Arc;
        ArmsRotation += Profile.ManualActionArmsRotation * Arc;
    }

    Weapon->SetActorRelativeLocation(WeaponLocation);
    Weapon->SetActorRelativeRotation(WeaponRotation);
    Arms->SetRelativeLocation(ArmsLocation);
    Arms->SetRelativeRotation(ArmsRotation);
}