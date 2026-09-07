#include "OCWeaponAudioComponent.h"

#include "OCCharacter.h"
#include "OCAudioUserSettings.h"
#include "OCWeaponBase.h"
#include "OCWeaponAudioProfile.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/StreamableManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    constexpr const TCHAR* AKFirePath = TEXT("/Game/AK-47/Sound/AK-47/Cues/AK47_Fire_Cue.AK47_Fire_Cue");
    constexpr const TCHAR* AKReloadPath = TEXT("/Game/AK-47/Sound/AK-47/Cues/Reload_Cue.Reload_Cue");
    constexpr const TCHAR* AKDryFirePath = TEXT("/Game/AK-47/Sound/AK-47/Cues/AK47_Empty_Cue.AK47_Empty_Cue");
    constexpr const TCHAR* GenericShotPath = TEXT("/Game/R13/Audio/gunfire_sfx.gunfire_sfx");
    constexpr const TCHAR* GenericReloadPath = TEXT("/Game/R13/Audio/gunreload1.gunreload1");
    constexpr const TCHAR* AssaultReloadPath = TEXT("/Game/R13/Audio/assaultriflereload1.assaultriflereload1");
    constexpr const TCHAR* PumpPath = TEXT("/Game/R13/Audio/shotguncock.shotguncock");
    constexpr const TCHAR* ImpactPath = TEXT("/Game/R13/Audio/snd_bullethit.snd_bullethit");
    constexpr const TCHAR* BoltPath = TEXT("/Game/PASS45/Audio/ManualAction/SW_PASS45_BoltAction_CC0_Donor.SW_PASS45_BoltAction_CC0_Donor");
    constexpr const TCHAR* LeverPath = TEXT("/Game/PASS45/Audio/ManualAction/SW_PASS45_LeverAction_CC0_Donor.SW_PASS45_LeverAction_CC0_Donor");

    TAutoConsoleVariable<int32> CVarOCAudioDebug(
        TEXT("oc.Audio.Debug"),
        0,
        TEXT("Weapon audio debug labels. 0=off, 1=events."),
        ECVF_Default);

    void BuildRepositoryFallbackAudioPaths(TArray<FSoftObjectPath>& OutPaths)
    {
        const TCHAR* Paths[] =
        {
            AKFirePath,
            AKReloadPath,
            AKDryFirePath,
            GenericShotPath,
            GenericReloadPath,
            AssaultReloadPath,
            PumpPath,
            ImpactPath,
            BoltPath,
            LeverPath
        };

        OutPaths.Reset();
        OutPaths.Reserve(UE_ARRAY_COUNT(Paths));
        for (const TCHAR* Path : Paths)
        {
            OutPaths.Emplace(Path);
        }
    }

    USoundBase* ResolveResidentSound(const TCHAR* AssetPath)
    {
        return Cast<USoundBase>(FSoftObjectPath(AssetPath).ResolveObject());
    }
}

UOCWeaponAudioComponent::UOCWeaponAudioComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(false);
}

void UOCWeaponAudioComponent::BeginPlay()
{
    Super::BeginPlay();

    UWorld* World = GetWorld();
    if (!World || World->GetNetMode() == NM_DedicatedServer)
    {
        return;
    }

    BeginRepositoryFallbackPreload();
}

void UOCWeaponAudioComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (RepositoryFallbackPreloadHandle.IsValid())
    {
        RepositoryFallbackPreloadHandle->CancelHandle();
        RepositoryFallbackPreloadHandle.Reset();
    }

    Super::EndPlay(EndPlayReason);
}

void UOCWeaponAudioComponent::SetAudioProfile(UOCWeaponAudioProfile* NewProfile)
{
    AudioProfile = NewProfile;
}

void UOCWeaponAudioComponent::BeginRepositoryFallbackPreload()
{
    if (bRepositoryFallbackPreloadRequested)
    {
        return;
    }
    bRepositoryFallbackPreloadRequested = true;

    TArray<FSoftObjectPath> Paths;
    BuildRepositoryFallbackAudioPaths(Paths);
    RepositoryFallbackPreloadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
        Paths,
        FStreamableDelegate());

    if (!RepositoryFallbackPreloadHandle.IsValid())
    {
        bRepositoryFallbackPreloadGapLogged = true;
        UE_LOG(LogTemp, Error,
            TEXT("GAME_RECOVERY_WEAPON_AUDIO_PRELOAD_GAP reason=invalid_handle assets=%d first_use_sync_load=0 runtime_acceptance=0"),
            Paths.Num());
        return;
    }

    UE_LOG(LogTemp, Display,
        TEXT("GAME_RECOVERY_WEAPON_AUDIO_PRELOAD_BEGIN assets=%d async=1 first_use_sync_load=0 resident_until_end_play=1"),
        Paths.Num());
}

UOCWeaponAudioProfile* UOCWeaponAudioComponent::EnsureRepositoryFallbackProfile()
{
    if (RepositoryFallbackProfile)
    {
        return RepositoryFallbackProfile;
    }

    if (!bRepositoryFallbackPreloadRequested)
    {
        BeginRepositoryFallbackPreload();
    }

    if (!RepositoryFallbackPreloadHandle.IsValid())
    {
        if (!bRepositoryFallbackPreloadGapLogged)
        {
            bRepositoryFallbackPreloadGapLogged = true;
            UE_LOG(LogTemp, Error,
                TEXT("GAME_RECOVERY_WEAPON_AUDIO_PRELOAD_GAP reason=missing_handle first_use_sync_load=0 runtime_acceptance=0"));
        }
        return nullptr;
    }

    if (!RepositoryFallbackPreloadHandle->HasLoadCompleted())
    {
        if (!bRepositoryFallbackPreloadPendingLogged)
        {
            bRepositoryFallbackPreloadPendingLogged = true;
            const AOCWeaponBase* Weapon = Cast<AOCWeaponBase>(GetOwner());
            UE_LOG(LogTemp, Verbose,
                TEXT("GAME_RECOVERY_WEAPON_AUDIO_PRELOAD_PENDING weapon=%s first_use_sync_load=0 fallback_audio_deferred=1"),
                Weapon ? *Weapon->GetWeaponId().ToString() : TEXT("None"));
        }
        return nullptr;
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

    USoundBase* Shot = nullptr;
    USoundBase* Reload = nullptr;
    USoundBase* DryFire = nullptr;

    // The preload handle owns package residency. First-use paths only resolve already-resident objects and never issue disk loads.
    if (WeaponId == FName(TEXT("OC_AR1")))
    {
        Shot = ResolveResidentSound(AKFirePath);
        Reload = ResolveResidentSound(AKReloadPath);
        DryFire = ResolveResidentSound(AKDryFirePath);
    }

    if (!Shot)
    {
        Shot = ResolveResidentSound(GenericShotPath);
    }
    if (!Reload)
    {
        Reload = Weapon && Weapon->GetWeaponClass() == EOCWeaponClass::AssaultRifle
            ? ResolveResidentSound(AssaultReloadPath)
            : ResolveResidentSound(GenericReloadPath);
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

    if (ActionType == EOCWeaponActionType::BoltAction)
    {
        if (USoundBase* Bolt = ResolveResidentSound(BoltPath))
        {
            RepositoryFallbackProfile->BoltCycle.Add(Bolt);
        }
    }
    if (ActionType == EOCWeaponActionType::PumpAction)
    {
        if (USoundBase* Pump = ResolveResidentSound(PumpPath))
        {
            RepositoryFallbackProfile->PumpCycle.Add(Pump);
        }
    }
    if (ActionType == EOCWeaponActionType::LeverAction)
    {
        if (USoundBase* Lever = ResolveResidentSound(LeverPath))
        {
            RepositoryFallbackProfile->LeverCycle.Add(Lever);
        }
    }

    if (USoundBase* Impact = ResolveResidentSound(ImpactPath))
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
            TEXT("PASS45_WEAPON_AUDIO_CONTENT_GAP weapon=%s event=shot repository_fallback_load=0 async_preloaded=1 resident_only=1 first_use_sync_load=0 runtime_acceptance=0"),
            *WeaponId.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_WEAPON_AUDIO_FALLBACK_READY weapon=%s shot=1 reload=%d bolt_cycle=%d pump_cycle=%d lever_cycle=%d exact_profile_override=0 authoritative_mutation=0 async_preloaded=1 resident_only=1 first_use_sync_load=0 runtime_acceptance=0"),
            *WeaponId.ToString(),
            RepositoryFallbackProfile->ReloadStart.IsEmpty() ? 0 : 1,
            RepositoryFallbackProfile->BoltCycle.IsEmpty() ? 0 : 1,
            RepositoryFallbackProfile->PumpCycle.IsEmpty() ? 0 : 1,
            RepositoryFallbackProfile->LeverCycle.IsEmpty() ? 0 : 1);
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
    const bool bLocalPlayback = IsLocalWeaponOwner() && Event != EOCWeaponAudioEvent::Drop;
    const float PlaybackVolume = bLocalPlayback ? StateProfile->LocalMechanicalVolume : 1.0f;
    const float WeaponBusVolume = UOCAudioUserSettings::Get()->GetBusVolume(EOCAudioBus::Weapons);
    const bool bPlaybackDispatchable = Sound && PlaybackVolume > 0.0f && WeaponBusVolume > 0.0f;
    if (bLocalPlayback)
    {
        Play2D(Sound, PlaybackVolume);
    }
    else
    {
        PlayAt(Sound, SourceLocation, PlaybackVolume);
    }

    if (Event == EOCWeaponAudioEvent::ManualActionCycle)
    {
        const FName WeaponId = Weapon ? Weapon->GetWeaponId() : NAME_None;
        const EOCWeaponActionType ActionType = Weapon ? Weapon->GetWeaponActionType() : EOCWeaponActionType::GasOperated;
        if (bPlaybackDispatchable)
        {
            UE_LOG(LogTemp, Display,
                TEXT("PASS45_MANUAL_ACTION_AUDIO_PLAYBACK_DISPATCHED weapon=%s action=%s sound=%s route=%s bus_gt_zero=1 effective_volume_gt_zero=1 second_gameplay_timer=0 runtime_acceptance=0"),
                *WeaponId.ToString(), *UEnum::GetValueAsString(ActionType), *GetPathNameSafe(Sound),
                bLocalPlayback ? TEXT("local2d") : TEXT("world3d"));
        }
        else
        {
            UE_LOG(LogTemp, Warning,
                TEXT("PASS45_MANUAL_ACTION_AUDIO_PLAYBACK_FAIL weapon=%s action=%s sound_present=%d bus_gt_zero=%d effective_volume_gt_zero=%d runtime_acceptance=0"),
                *WeaponId.ToString(), *UEnum::GetValueAsString(ActionType), Sound ? 1 : 0,
                WeaponBusVolume > 0.0f ? 1 : 0, PlaybackVolume > 0.0f ? 1 : 0);
        }
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
