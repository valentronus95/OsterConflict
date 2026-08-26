#include "OCWeaponAudioComponent.h"

#include "OCCharacter.h"
#include "OCAudioUserSettings.h"
#include "OCWeaponBase.h"
#include "OCWeaponAudioProfile.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    TAutoConsoleVariable<int32> CVarOCAudioDebug(
        TEXT("oc.Audio.Debug"),
        0,
        TEXT("Weapon audio debug labels. 0=off, 1=events."),
        ECVF_Default);
}

UOCWeaponAudioComponent::UOCWeaponAudioComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(false);
}

void UOCWeaponAudioComponent::SetAudioProfile(UOCWeaponAudioProfile* NewProfile)
{
    AudioProfile = NewProfile;
}

UOCWeaponAudioProfile* UOCWeaponAudioComponent::EnsureRepositoryFallbackProfile()
{
    if (RepositoryFallbackProfile)
    {
        return RepositoryFallbackProfile;
    }
    if (bRepositoryFallbackAttempted)
    {
        return nullptr;
    }
    bRepositoryFallbackAttempted = true;

    RepositoryFallbackProfile = NewObject<UOCWeaponAudioProfile>(this, TEXT("PASS45RepositoryFallbackAudioProfile"));
    if (!RepositoryFallbackProfile)
    {
        UE_LOG(LogTemp, Error, TEXT("PASS45_WEAPON_AUDIO_CONTENT_GAP reason=fallback_profile_allocation_failed"));
        return nullptr;
    }

    const AOCWeaponBase* Weapon = Cast<AOCWeaponBase>(GetOwner());
    const FName WeaponId = Weapon ? Weapon->GetWeaponId() : NAME_None;
    const EOCWeaponActionType ActionType = Weapon ? Weapon->GetWeaponActionType() : EOCWeaponActionType::GasOperated;
    RepositoryFallbackProfile->ProfileId = FName(*FString::Printf(TEXT("PASS45_RepositoryFallback_%s"), *WeaponId.ToString()));

    auto LoadSound = [](const TCHAR* AssetPath) -> USoundBase*
    {
        return LoadObject<USoundBase>(nullptr, AssetPath);
    };

    USoundBase* Shot = nullptr;
    USoundBase* Reload = nullptr;
    USoundBase* DryFire = nullptr;

    // The AK package already carries dedicated project audio. Prefer it for the exact represented AK.
    if (WeaponId == FName(TEXT("OC_AR1")))
    {
        Shot = LoadSound(TEXT("/Game/AK-47/Sound/AK-47/Cues/AK47_Fire_Cue.AK47_Fire_Cue"));
        Reload = LoadSound(TEXT("/Game/AK-47/Sound/AK-47/Cues/Reload_Cue.Reload_Cue"));
        DryFire = LoadSound(TEXT("/Game/AK-47/Sound/AK-47/Cues/AK47_Empty_Cue.AK47_Empty_Cue"));
    }

    // Other current weapons have no guaranteed assigned DataAsset yet. Reuse repository-owned audio rather than
    // accepting a silent factual shot. This is an explicit temporary content fallback, not a claim of final sound identity.
    if (!Shot)
    {
        Shot = LoadSound(TEXT("/Game/R13/Audio/gunfire_sfx.gunfire_sfx"));
    }
    if (!Reload)
    {
        Reload = Weapon && Weapon->GetWeaponClass() == EOCWeaponClass::AssaultRifle
            ? LoadSound(TEXT("/Game/R13/Audio/assaultriflereload1.assaultriflereload1"))
            : LoadSound(TEXT("/Game/R13/Audio/gunreload1.gunreload1"));
    }

    if (Shot)
    {
        RepositoryFallbackProfile->ShotNearOutdoor.Add(Shot);
        RepositoryFallbackProfile->ShotNearIndoor.Add(Shot);
    }
    if (Reload)
    {
        RepositoryFallbackProfile->ReloadStart.Add(Reload);
    }
    if (DryFire)
    {
        RepositoryFallbackProfile->DryFire.Add(DryFire);
    }

    if (ActionType == EOCWeaponActionType::PumpAction)
    {
        if (USoundBase* Pump = LoadSound(TEXT("/Game/R13/Audio/shotguncock.shotguncock")))
        {
            RepositoryFallbackProfile->PumpCycle.Add(Pump);
        }
    }

    if (USoundBase* Impact = LoadSound(TEXT("/Game/R13/Audio/snd_bullethit.snd_bullethit")))
    {
        RepositoryFallbackProfile->ImpactFlesh.Add(Impact);
        RepositoryFallbackProfile->ImpactGlass.Add(Impact);
        RepositoryFallbackProfile->ImpactWood.Add(Impact);
        RepositoryFallbackProfile->ImpactMetal.Add(Impact);
        RepositoryFallbackProfile->ImpactMasonry.Add(Impact);
        RepositoryFallbackProfile->ImpactDirt.Add(Impact);
    }

    if (RepositoryFallbackProfile->ShotNearOutdoor.IsEmpty())
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_WEAPON_AUDIO_CONTENT_GAP weapon=%s event=shot repository_fallback_load=0 runtime_acceptance=0"),
            *WeaponId.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_WEAPON_AUDIO_FALLBACK_READY weapon=%s shot=1 reload=%d pump_cycle=%d exact_profile_override=0 authoritative_mutation=0 runtime_acceptance=0"),
            *WeaponId.ToString(),
            RepositoryFallbackProfile->ReloadStart.IsEmpty() ? 0 : 1,
            RepositoryFallbackProfile->PumpCycle.IsEmpty() ? 0 : 1);
    }

    return RepositoryFallbackProfile;
}

EOCAcousticEnvironment UOCWeaponAudioComponent::DetectEnvironmentAt(const FVector& SourceLocation) const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return EOCAcousticEnvironment::Outdoor;
    }

    FCollisionQueryParams Params(SCENE_QUERY_STAT(OCAudioEnvironment), false, GetOwner());
    const bool bRoof = World->LineTraceTestByChannel(SourceLocation + FVector(0,0,25),
        SourceLocation + FVector(0,0,1200), ECC_Visibility, Params);

    int32 WallHits = 0;
    static const FVector Directions[] = { FVector::ForwardVector, FVector::BackwardVector, FVector::RightVector, FVector::LeftVector };
    for (const FVector& Dir : Directions)
    {
        if (World->LineTraceTestByChannel(SourceLocation, SourceLocation + Dir * 650.0f, ECC_Visibility, Params))
        {
            ++WallHits;
        }
    }

    if (bRoof && WallHits >= 2)
    {
        return EOCAcousticEnvironment::Indoor;
    }
    if (bRoof || WallHits >= 3)
    {
        return EOCAcousticEnvironment::SemiIndoor;
    }
    return EOCAcousticEnvironment::Outdoor;
}

USoundBase* UOCWeaponAudioComponent::Pick(const TArray<TObjectPtr<USoundBase>>& Sounds, int32 EventSeed) const
{
    if (Sounds.IsEmpty())
    {
        return nullptr;
    }
    const int32 Index = FMath::Abs(EventSeed) % Sounds.Num();
    return Sounds[Index].Get();
}

FVector UOCWeaponAudioComponent::GetListenerLocation(bool& bOutHasListener) const
{
    bOutHasListener = false;
    UWorld* World = GetWorld();
    if (!World || World->GetNetMode() == NM_DedicatedServer)
    {
        return FVector::ZeroVector;
    }

    if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
    {
        bOutHasListener = true;
        return PC->PlayerCameraManager ? PC->PlayerCameraManager->GetCameraLocation() : PC->GetFocalLocation();
    }
    return FVector::ZeroVector;
}

bool UOCWeaponAudioComponent::IsLocalWeaponOwner() const
{
    const AActor* WeaponActor = GetOwner();
    const AOCCharacter* Character = WeaponActor ? Cast<AOCCharacter>(WeaponActor->GetOwner()) : nullptr;
    return Character && Character->IsLocallyControlled();
}

void UOCWeaponAudioComponent::PlayAt(USoundBase* Sound, const FVector& Location, float Volume) const
{
    if (Sound && GetWorld() && GetWorld()->GetNetMode() != NM_DedicatedServer)
    {
        const float Bus = UOCAudioUserSettings::Get()->GetBusVolume(EOCAudioBus::Weapons);
        if (Bus > 0.0f) UGameplayStatics::PlaySoundAtLocation(this, Sound, Location, FMath::Max(0.0f, Volume) * Bus);
    }
}

void UOCWeaponAudioComponent::Play2D(USoundBase* Sound, float Volume) const
{
    if (Sound && GetWorld() && GetWorld()->GetNetMode() != NM_DedicatedServer)
    {
        const float Bus = UOCAudioUserSettings::Get()->GetBusVolume(EOCAudioBus::Weapons);
        if (Bus > 0.0f) UGameplayStatics::PlaySound2D(this, Sound, FMath::Max(0.0f, Volume) * Bus);
    }
}

void UOCWeaponAudioComponent::EmitDebugEvent(const FString& Label, const FVector& Location) const
{
#if !UE_BUILD_SHIPPING
    if (CVarOCAudioDebug.GetValueOnGameThread() > 0 && GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 0.75f, FColor(120,210,255),
            FString::Printf(TEXT("AUDIO %s @ %.0f %.0f %.0f"), *Label, Location.X, Location.Y, Location.Z));
    }
#endif
}

void UOCWeaponAudioComponent::HandleShotLocal(const FVector& ShotOrigin, const FVector& TraceEnd, bool bSuppressed, bool bSupersonic,
    EOCAcousticEnvironment Environment, int32 EventSeed)
{
    if (!GetWorld() || GetWorld()->GetNetMode() == NM_DedicatedServer)
    {
        return;
    }

    const bool bIndoor = Environment == EOCAcousticEnvironment::Indoor;
    UOCWeaponAudioProfile* ShotProfile = AudioProfile;
    const auto HasRequestedNearShot = [bIndoor](const UOCWeaponAudioProfile* Profile)
    {
        if (!Profile) return false;
        return !(bIndoor ? Profile->ShotNearIndoor : Profile->ShotNearOutdoor).IsEmpty();
    };

    if (!HasRequestedNearShot(ShotProfile))
    {
        ShotProfile = EnsureRepositoryFallbackProfile();
    }
    if (!ShotProfile || !HasRequestedNearShot(ShotProfile))
    {
        EmitDebugEvent(TEXT("SHOT(content gap)"), ShotOrigin);
        return;
    }

    bool bHasListener = false;
    const FVector Listener = GetListenerLocation(bHasListener);
    if (!bHasListener)
    {
        return;
    }

    const float Distance = FVector::Distance(Listener, ShotOrigin);

    const TArray<TObjectPtr<USoundBase>>* NearSet = nullptr;
    float ReportVolume = 1.0f;
    if (bSuppressed)
    {
        NearSet = bIndoor ? &ShotProfile->ShotSuppressedIndoor : &ShotProfile->ShotSuppressedOutdoor;
        if (NearSet->IsEmpty())
        {
            NearSet = bIndoor ? &ShotProfile->ShotNearIndoor : &ShotProfile->ShotNearOutdoor;
            ReportVolume = ShotProfile->SuppressedFallbackVolume;
        }
    }
    else
    {
        NearSet = bIndoor ? &ShotProfile->ShotNearIndoor : &ShotProfile->ShotNearOutdoor;
    }

    if (Distance <= ShotProfile->NearShotMaxDistanceCm)
    {
        PlayAt(Pick(*NearSet, EventSeed), ShotOrigin, ReportVolume);
        EmitDebugEvent(bIndoor ? TEXT("SHOT INDOOR") : TEXT("SHOT OUTDOOR"), ShotOrigin);
    }
    else if (Distance <= ShotProfile->DistantTailMaxDistanceCm)
    {
        // A dedicated authored distant tail is preferred. If a temporary repository fallback does not have one,
        // use its factual near report at reduced level rather than making the entire weapon disappear acoustically.
        USoundBase* Tail = Pick(ShotProfile->DistantTails, EventSeed + 17);
        if (!Tail) Tail = Pick(*NearSet, EventSeed + 17);
        PlayAt(Tail, ShotOrigin, bSuppressed ? 0.22f : 0.48f);
        EmitDebugEvent(TEXT("DISTANT TAIL"), ShotOrigin);
    }

    if (IsLocalWeaponOwner())
    {
        Play2D(Pick(ShotProfile->MechanicalShot, EventSeed + 31), ShotProfile->LocalMechanicalVolume);
    }

    if (ShotProfile->bSupersonicProjectile && bSupersonic && !IsLocalWeaponOwner())
    {
        const FVector Closest = FMath::ClosestPointOnSegment(Listener, ShotOrigin, TraceEnd);
        const float CrackDistance = FVector::Distance(Listener, Closest);
        const float FromMuzzle = FVector::Distance(Listener, ShotOrigin);
        if (FromMuzzle > 500.0f && CrackDistance <= ShotProfile->BulletCrackRadiusCm)
        {
            PlayAt(Pick(ShotProfile->BulletCracks, EventSeed + 47), Closest, 1.0f);
            EmitDebugEvent(TEXT("BULLET CRACK"), Closest);
        }
    }
}

void UOCWeaponAudioComponent::HandleStateEventLocal(EOCWeaponAudioEvent Event, const FVector& SourceLocation, int32 EventSeed)
{
    if (!GetWorld() || GetWorld()->GetNetMode() == NM_DedicatedServer)
    {
        return;
    }

    const AOCWeaponBase* Weapon = Cast<AOCWeaponBase>(GetOwner());
    auto ResolveSet = [Weapon, Event](UOCWeaponAudioProfile* Profile) -> const TArray<TObjectPtr<USoundBase>>*
    {
        if (!Profile) return nullptr;
        switch (Event)
        {
            case EOCWeaponAudioEvent::ReloadStart: return &Profile->ReloadStart;
            case EOCWeaponAudioEvent::ReloadEnd: return &Profile->ReloadEnd;
            case EOCWeaponAudioEvent::ReloadCancel: return &Profile->ReloadCancel;
            case EOCWeaponAudioEvent::DryFire: return &Profile->DryFire;
            case EOCWeaponAudioEvent::FireModeSwitch: return &Profile->FireModeSwitch;
            case EOCWeaponAudioEvent::ManualActionCycle:
            {
                if (!Weapon) return nullptr;
                switch (Weapon->GetWeaponActionType())
                {
                    case EOCWeaponActionType::BoltAction: return &Profile->BoltCycle;
                    case EOCWeaponActionType::PumpAction: return &Profile->PumpCycle;
                    case EOCWeaponActionType::LeverAction: return &Profile->LeverCycle;
                    default: return nullptr;
                }
            }
            case EOCWeaponAudioEvent::Equip: return &Profile->Equip;
            case EOCWeaponAudioEvent::Drop: return &Profile->Drop;
            default: return nullptr;
        }
    };

    UOCWeaponAudioProfile* StateProfile = AudioProfile;
    const TArray<TObjectPtr<USoundBase>>* Set = ResolveSet(StateProfile);
    if (!Set || Set->IsEmpty())
    {
        StateProfile = EnsureRepositoryFallbackProfile();
        Set = ResolveSet(StateProfile);
    }
    if (!Set || Set->IsEmpty())
    {
        if (Event == EOCWeaponAudioEvent::ManualActionCycle)
        {
            const FName WeaponId = Weapon ? Weapon->GetWeaponId() : NAME_None;
            UE_LOG(LogTemp, Warning,
                TEXT("PASS45_WEAPON_AUDIO_CONTENT_GAP weapon=%s event=manual_action action=%s runtime_acceptance=0"),
                *WeaponId.ToString(), Weapon ? *UEnum::GetValueAsString(Weapon->GetWeaponActionType()) : TEXT("None"));
            EmitDebugEvent(TEXT("MANUAL ACTION(content gap)"), SourceLocation);
        }
        return;
    }

    USoundBase* Sound = Pick(*Set, EventSeed);
    if (IsLocalWeaponOwner() && Event != EOCWeaponAudioEvent::Drop)
    {
        Play2D(Sound, StateProfile->LocalMechanicalVolume);
    }
    else
    {
        PlayAt(Sound, SourceLocation, 1.0f);
    }
    EmitDebugEvent(UEnum::GetValueAsString(Event), SourceLocation);
}

void UOCWeaponAudioComponent::HandleImpactLocal(const FVector& ImpactLocation, EOCImpactSurface Surface, int32 EventSeed)
{
    if (!GetWorld() || GetWorld()->GetNetMode() == NM_DedicatedServer)
    {
        return;
    }

    UOCWeaponAudioProfile* ImpactProfile = AudioProfile;
    if (!ImpactProfile || ImpactProfile->GetImpactSet(Surface).IsEmpty())
    {
        ImpactProfile = EnsureRepositoryFallbackProfile();
    }
    if (!ImpactProfile || ImpactProfile->GetImpactSet(Surface).IsEmpty())
    {
        return;
    }

    PlayAt(Pick(ImpactProfile->GetImpactSet(Surface), EventSeed), ImpactLocation, 1.0f);
    EmitDebugEvent(FString::Printf(TEXT("IMPACT %s"), *UEnum::GetValueAsString(Surface)), ImpactLocation);
}