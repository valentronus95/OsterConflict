#include "OCWeaponAudioComponent.h"

#include "OCCharacter.h"
#include "OCAudioUserSettings.h"
#include "OCWeaponAudioProfile.h"
#include "OCWeaponBase.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"

namespace
{
    TAutoConsoleVariable<int32> CVarOCAudioDebug(
        TEXT("oc.Audio.Debug"),
        0,
        TEXT("Weapon audio debug labels. 0=off, 1=events."),
        ECVF_Default);

    USoundBase* LoadR13Audio(const TCHAR* AssetName)
    {
        const FString ObjectPath = FString::Printf(TEXT("/Game/R13/Audio/%s.%s"), AssetName, AssetName);
        return LoadObject<USoundBase>(nullptr, *ObjectPath);
    }
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

    // R13 practical fallback: the old source build had a complete audio routing system but no assigned profile assets,
    // which meant technically valid shots were completely silent. Imported CC0 audio makes combat audible immediately.
    if (!AudioProfile)
    {
        if (USoundBase* Shot = LoadR13Audio(TEXT("gunfire_sfx")))
        {
            if (IsLocalWeaponOwner()) Play2D(Shot, bSuppressed ? 0.42f : 0.82f);
            else PlayAt(Shot, ShotOrigin, bSuppressed ? 0.34f : 0.68f);
            EmitDebugEvent(TEXT("R13 SHOT"), ShotOrigin);
        }
        else
        {
            EmitDebugEvent(TEXT("SHOT(no profile/assets)"), ShotOrigin);
        }
        return;
    }

    bool bHasListener = false;
    const FVector Listener = GetListenerLocation(bHasListener);
    if (!bHasListener)
    {
        return;
    }

    const float Distance = FVector::Distance(Listener, ShotOrigin);
    const bool bIndoor = Environment == EOCAcousticEnvironment::Indoor;

    const TArray<TObjectPtr<USoundBase>>* NearSet = nullptr;
    float ReportVolume = 1.0f;
    if (bSuppressed)
    {
        NearSet = bIndoor ? &AudioProfile->ShotSuppressedIndoor : &AudioProfile->ShotSuppressedOutdoor;
        if (NearSet->IsEmpty())
        {
            NearSet = bIndoor ? &AudioProfile->ShotNearIndoor : &AudioProfile->ShotNearOutdoor;
            ReportVolume = AudioProfile->SuppressedFallbackVolume;
        }
    }
    else
    {
        NearSet = bIndoor ? &AudioProfile->ShotNearIndoor : &AudioProfile->ShotNearOutdoor;
    }

    if (Distance <= AudioProfile->NearShotMaxDistanceCm)
    {
        PlayAt(Pick(*NearSet, EventSeed), ShotOrigin, ReportVolume);
        EmitDebugEvent(bIndoor ? TEXT("SHOT INDOOR") : TEXT("SHOT OUTDOOR"), ShotOrigin);
    }
    else if (Distance <= AudioProfile->DistantTailMaxDistanceCm)
    {
        PlayAt(Pick(AudioProfile->DistantTails, EventSeed + 17), ShotOrigin, bSuppressed ? 0.30f : 1.0f);
        EmitDebugEvent(TEXT("DISTANT TAIL"), ShotOrigin);
    }

    if (IsLocalWeaponOwner())
    {
        Play2D(Pick(AudioProfile->MechanicalShot, EventSeed + 31), AudioProfile->LocalMechanicalVolume);
    }

    if (AudioProfile->bSupersonicProjectile && bSupersonic && !IsLocalWeaponOwner())
    {
        const FVector Closest = FMath::ClosestPointOnSegment(Listener, ShotOrigin, TraceEnd);
        const float CrackDistance = FVector::Distance(Listener, Closest);
        const float FromMuzzle = FVector::Distance(Listener, ShotOrigin);
        if (FromMuzzle > 500.0f && CrackDistance <= AudioProfile->BulletCrackRadiusCm)
        {
            PlayAt(Pick(AudioProfile->BulletCracks, EventSeed + 47), Closest, 1.0f);
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

    if (!AudioProfile)
    {
        USoundBase* Sound = nullptr;
        if (Event == EOCWeaponAudioEvent::ReloadStart)
        {
            const AOCWeaponBase* Weapon = Cast<AOCWeaponBase>(GetOwner());
            if (Weapon && Weapon->GetWeaponClass() == EOCWeaponClass::Pistol)
                Sound = LoadR13Audio(TEXT("gunreload1"));
            else if (Weapon && Weapon->GetWeaponClass() == EOCWeaponClass::Shotgun)
                Sound = LoadR13Audio(TEXT("shotguncock"));
            else
                Sound = LoadR13Audio(TEXT("assaultriflereload1"));
        }
        if (Sound)
        {
            if (IsLocalWeaponOwner() && Event != EOCWeaponAudioEvent::Drop) Play2D(Sound, 0.90f);
            else PlayAt(Sound, SourceLocation, 0.80f);
            EmitDebugEvent(TEXT("R13 WEAPON STATE"), SourceLocation);
        }
        return;
    }

    const TArray<TObjectPtr<USoundBase>>* Set = nullptr;
    switch (Event)
    {
        case EOCWeaponAudioEvent::ReloadStart: Set = &AudioProfile->ReloadStart; break;
        case EOCWeaponAudioEvent::ReloadEnd: Set = &AudioProfile->ReloadEnd; break;
        case EOCWeaponAudioEvent::ReloadCancel: Set = &AudioProfile->ReloadCancel; break;
        case EOCWeaponAudioEvent::DryFire: Set = &AudioProfile->DryFire; break;
        case EOCWeaponAudioEvent::FireModeSwitch: Set = &AudioProfile->FireModeSwitch; break;
        case EOCWeaponAudioEvent::Equip: Set = &AudioProfile->Equip; break;
        case EOCWeaponAudioEvent::Drop: Set = &AudioProfile->Drop; break;
        default: break;
    }
    if (!Set)
    {
        return;
    }

    USoundBase* Sound = Pick(*Set, EventSeed);
    if (IsLocalWeaponOwner() && Event != EOCWeaponAudioEvent::Drop)
    {
        Play2D(Sound, AudioProfile->LocalMechanicalVolume);
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
    if (!AudioProfile)
    {
        PlayAt(LoadR13Audio(TEXT("snd_bullethit")), ImpactLocation, 0.55f);
        return;
    }
    PlayAt(Pick(AudioProfile->GetImpactSet(Surface), EventSeed), ImpactLocation, 1.0f);
    EmitDebugEvent(FString::Printf(TEXT("IMPACT %s"), *UEnum::GetValueAsString(Surface)), ImpactLocation);
}
