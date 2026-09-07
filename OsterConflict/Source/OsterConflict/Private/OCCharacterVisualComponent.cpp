#include "OCCharacterVisualComponent.h"

#include "OCCharacter.h"
#include "OCCharacterVisualProfile.h"
#include "OCPlayerState.h"
#include "OCTeamTypes.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundBase.h"
#include "UObject/SoftObjectPath.h"

namespace
{
    const TCHAR* Pass45GrenadeThrowSoundPath = TEXT("/Game/R13/Audio/snd_throw1.snd_throw1");
}

UOCCharacterVisualComponent::UOCCharacterVisualComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.25f;
    SetIsReplicatedByDefault(true);
}

void UOCCharacterVisualComponent::BeginPlay()
{
    Super::BeginPlay();
    CharacterOwner = Cast<AOCCharacter>(GetOwner());
    if (GetWorld() && GetWorld()->GetNetMode() != NM_DedicatedServer)
    {
        // GAME RECOVERY: production character preparation preloads this package before possession.
        // Never let actor BeginPlay turn into a disk-backed package load.
        GrenadeThrowSound = Cast<USoundBase>(FSoftObjectPath(Pass45GrenadeThrowSoundPath).ResolveObject());
    }
    if (bEnableSourceOnlyProxy) BuildSourceOnlyProxy();
    RefreshPresentation(true);
}

void UOCCharacterVisualComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    RefreshPresentation(false);
}

void UOCCharacterVisualComponent::InitializeFirstPersonArms(USkeletalMeshComponent* InFirstPersonArms)
{
    FirstPersonArms = InFirstPersonArms;
}

UOCCharacterVisualProfile* UOCCharacterVisualComponent::GetProfile(EOCFactionArchetype Faction) const
{
    switch (Faction)
    {
    case EOCFactionArchetype::MaskedFighters: return MaskedFightersProfile;
    case EOCFactionArchetype::USRangers: return USRangersProfile;
    case EOCFactionArchetype::Insurgents: return InsurgentsProfile;
    default: return UASpecialUnitProfile;
    }
}

FOCCharacterAppearance UOCCharacterVisualComponent::BuildAppearance() const
{
    FOCCharacterAppearance Result;
    const AOCCharacter* Character = CharacterOwner.Get();
    const AOCPlayerState* State = Character ? Character->GetPlayerState<AOCPlayerState>() : nullptr;
    Result.Faction = State ? State->GetFactionArchetype() : EOCFactionArchetype::UASpecialUnit;
    Result.VariantSeed = State ? FMath::Max(1, State->GetAppearanceSeed()) : 1;

    FRandomStream Stream(Result.VariantSeed);
    Result.HeadVariant = Stream.RandRange(0, 5);
    Result.HelmetVariant = Stream.RandRange(0, 3);
    Result.VestVariant = Stream.RandRange(0, 3);
    Result.BackpackVariant = Stream.RandRange(0, 2);

    if (State)
    {
        switch (State->GetPlayerRole())
        {
        case EOCPlayerRole::Engineer:
        case EOCPlayerRole::Support: Result.GearClass = EOCCharacterGearClass::Heavy; break;
        case EOCPlayerRole::Medic: Result.GearClass = EOCCharacterGearClass::Standard; break;
        default: Result.GearClass = Stream.FRand() < 0.35f ? EOCCharacterGearClass::Light : EOCCharacterGearClass::Standard; break;
        }
    }
    return Result;
}

void UOCCharacterVisualComponent::RefreshPresentation(bool bForce)
{
    AOCCharacter* Character = CharacterOwner.Get();
    if (!Character) return;
    const AOCPlayerState* State = Character->GetPlayerState<AOCPlayerState>();
    if (!State && !bForce) return;

    const FOCCharacterAppearance NewAppearance = BuildAppearance();
    if (!bForce && bHasAppliedPresentation && NewAppearance.Faction == LastAppliedFaction &&
        NewAppearance.VariantSeed == LastAppliedSeed && NewAppearance.GearClass == CurrentAppearance.GearClass)
    {
        return;
    }

    CurrentAppearance = NewAppearance;
    LastAppliedFaction = NewAppearance.Faction;
    LastAppliedSeed = NewAppearance.VariantSeed;
    UOCCharacterVisualProfile* Profile = GetProfile(NewAppearance.Faction);
    ApplyProfile(Profile);
    ApplyProxyTint(NewAppearance.Faction);
    BP_OnAppearanceApplied(CurrentAppearance, Profile);
    bHasAppliedPresentation = true;
}

void UOCCharacterVisualComponent::ApplyProfile(UOCCharacterVisualProfile* Profile)
{
    AOCCharacter* Character = CharacterOwner.Get();
    if (!Character) return;

    USkeletalMeshComponent* ThirdPersonMesh = Character->GetMesh();
    bool bHasProductionBody = false;
    if (Profile && ThirdPersonMesh && !Profile->ThirdPersonBodyMesh.IsNull())
    {
        // Preload is owned by UOCProductionCharacterAssetsSubsystem. Get() is intentionally non-blocking.
        if (USkeletalMesh* Loaded = Profile->ThirdPersonBodyMesh.Get())
        {
            ThirdPersonMesh->SetSkeletalMeshAsset(Loaded);
            if (Profile->ThirdPersonAnimClass) ThirdPersonMesh->SetAnimInstanceClass(Profile->ThirdPersonAnimClass);
            ThirdPersonMesh->SetOwnerNoSee(true);
            ThirdPersonMesh->SetVisibility(true, true);
            bHasProductionBody = true;
        }
    }

    if (!bHasProductionBody && ThirdPersonMesh)
    {
        ThirdPersonMesh->SetVisibility(false, true);
    }

    if (USkeletalMeshComponent* Arms = FirstPersonArms.Get())
    {
        bool bHasProductionArms = false;
        if (Profile && !Profile->FirstPersonArmsMesh.IsNull())
        {
            if (USkeletalMesh* Loaded = Profile->FirstPersonArmsMesh.Get())
            {
                Arms->SetSkeletalMeshAsset(Loaded);
                if (Profile->FirstPersonAnimClass) Arms->SetAnimInstanceClass(Profile->FirstPersonAnimClass);
                bHasProductionArms = true;
            }
        }
        Arms->SetOnlyOwnerSee(true);
        Arms->SetCastShadow(false);
        Arms->SetVisibility(bHasProductionArms, true);
    }

    UpdateSourceOnlyProxy(!bHasProductionBody);
}

void UOCCharacterVisualComponent::BuildSourceOnlyProxy()
{
    AOCCharacter* Character = CharacterOwner.Get();
    if (!Character || ThirdPersonProxyParts.Num() > 0) return;

    // Explicit developer-only diagnostics. Production runtime defaults bEnableSourceOnlyProxy=false.
    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (!Cube || !Sphere || !Cylinder) return;

    struct FPart { const TCHAR* Name; UStaticMesh* Mesh; FVector Location; FVector Scale; FRotator Rotation; };
    const FPart Parts[] = {
        { TEXT("ProxyTorso"), Cube, FVector(0,0,10), FVector(0.42f,0.28f,0.58f), FRotator::ZeroRotator },
        { TEXT("ProxyVest"), Cube, FVector(4,0,12), FVector(0.46f,0.31f,0.38f), FRotator::ZeroRotator },
        { TEXT("ProxyHead"), Sphere, FVector(0,0,67), FVector(0.20f), FRotator::ZeroRotator },
        { TEXT("ProxyHelmet"), Sphere, FVector(0,0,76), FVector(0.23f,0.23f,0.14f), FRotator::ZeroRotator },
        { TEXT("ProxyLegL"), Cylinder, FVector(0,-14,-49), FVector(0.13f,0.13f,0.46f), FRotator::ZeroRotator },
        { TEXT("ProxyLegR"), Cylinder, FVector(0,14,-49), FVector(0.13f,0.13f,0.46f), FRotator::ZeroRotator },
        { TEXT("ProxyArmL"), Cylinder, FVector(0,-38,13), FVector(0.12f,0.12f,0.42f), FRotator(0,0,8) },
        { TEXT("ProxyArmR"), Cylinder, FVector(0,38,13), FVector(0.12f,0.12f,0.42f), FRotator(0,0,-8) },
        { TEXT("ProxyBackpack"), Cube, FVector(-22,0,15), FVector(0.16f,0.28f,0.34f), FRotator::ZeroRotator },
    };
    for (const FPart& Part : Parts)
    {
        UStaticMeshComponent* Comp = NewObject<UStaticMeshComponent>(Character, FName(Part.Name));
        if (!Comp) continue;
        Comp->SetStaticMesh(Part.Mesh);
        Comp->SetupAttachment(Character->GetCapsuleComponent());
        Comp->SetRelativeLocation(Part.Location);
        Comp->SetRelativeRotation(Part.Rotation);
        Comp->SetRelativeScale3D(Part.Scale);
        Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Comp->SetGenerateOverlapEvents(false);
        Comp->SetOwnerNoSee(true);
        Comp->RegisterComponent();
        ThirdPersonProxyParts.Add(Comp);
    }

    if (UCameraComponent* Camera = Character->GetFirstPersonCamera())
    {
        const FPart Arms[] = {
            { TEXT("FPProxyArmL"), Cylinder, FVector(22,-17,-22), FVector(0.065f,0.065f,0.36f), FRotator(0,90,-10) },
            { TEXT("FPProxyArmR"), Cylinder, FVector(22,17,-22), FVector(0.065f,0.065f,0.36f), FRotator(0,90,10) },
            { TEXT("FPProxyHandL"), Sphere, FVector(54,-10,-17), FVector(0.075f,0.065f,0.065f), FRotator::ZeroRotator },
            { TEXT("FPProxyHandR"), Sphere, FVector(54,10,-17), FVector(0.075f,0.065f,0.065f), FRotator::ZeroRotator },
        };
        for (const FPart& Part : Arms)
        {
            UStaticMeshComponent* Comp = NewObject<UStaticMeshComponent>(Character, FName(Part.Name));
            if (!Comp) continue;
            Comp->SetStaticMesh(Part.Mesh);
            Comp->SetupAttachment(Camera);
            Comp->SetRelativeLocation(Part.Location);
            Comp->SetRelativeRotation(Part.Rotation);
            Comp->SetRelativeScale3D(Part.Scale);
            Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            Comp->SetGenerateOverlapEvents(false);
            Comp->SetOnlyOwnerSee(true);
            Comp->SetCastShadow(false);
            Comp->RegisterComponent();
            FirstPersonProxyParts.Add(Comp);
        }
    }
}

void UOCCharacterVisualComponent::UpdateSourceOnlyProxy(bool bShowProxy)
{
    for (UStaticMeshComponent* Part : ThirdPersonProxyParts)
    {
        if (Part) Part->SetVisibility(bShowProxy, true);
    }
    const bool bShowFPProxy = bShowProxy && CharacterOwner.IsValid() && CharacterOwner->IsLocallyControlled();
    for (UStaticMeshComponent* Part : FirstPersonProxyParts)
    {
        if (Part) Part->SetVisibility(bShowFPProxy, true);
    }
}

void UOCCharacterVisualComponent::ApplyProxyTint(EOCFactionArchetype Faction)
{
    // No production character should pay for Engine BasicShape material loading when proxies are disabled.
    if (ThirdPersonProxyParts.IsEmpty() && FirstPersonProxyParts.IsEmpty()) return;

    FLinearColor Color;
    switch (Faction)
    {
    case EOCFactionArchetype::MaskedFighters: Color = FLinearColor(0.12f,0.12f,0.13f); break;
    case EOCFactionArchetype::USRangers: Color = FLinearColor(0.40f,0.34f,0.22f); break;
    case EOCFactionArchetype::Insurgents: Color = FLinearColor(0.31f,0.25f,0.18f); break;
    default: Color = FLinearColor(0.23f,0.33f,0.20f); break;
    }

    UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!BaseMaterial) return;
    auto TintParts = [&](const TArray<TObjectPtr<UStaticMeshComponent>>& Parts)
    {
        for (UStaticMeshComponent* Part : Parts)
        {
            if (!Part) continue;
            UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMaterial, Part);
            if (MID)
            {
                MID->SetVectorParameterValue(TEXT("Color"), Color);
                Part->SetMaterial(0, MID);
            }
        }
    };
    TintParts(ThirdPersonProxyParts);
    TintParts(FirstPersonProxyParts);
}

void UOCCharacterVisualComponent::BroadcastActionServer(EOCCharacterActionEvent Event)
{
    AActor* Owner = GetOwner();
    if (!Owner || !Owner->HasAuthority()) return;
    MulticastCharacterAction(Event, ++ActionSequence);
}

void UOCCharacterVisualComponent::MulticastCharacterAction_Implementation(EOCCharacterActionEvent Event, int32 EventSeed)
{
    if (Event == EOCCharacterActionEvent::GrenadeThrow)
    {
        AActor* Owner = GetOwner();
        if (!GrenadeThrowSound && GetWorld() && GetWorld()->GetNetMode() != NM_DedicatedServer)
        {
            // Non-blocking retry only. GAME RECOVERY preloads this package before player spawn.
            GrenadeThrowSound = Cast<USoundBase>(FSoftObjectPath(Pass45GrenadeThrowSoundPath).ResolveObject());
        }
        if (GetWorld() && GetWorld()->GetNetMode() != NM_DedicatedServer && GrenadeThrowSound && Owner)
        {
            const bool bLocalFirstPerson = CharacterOwner.IsValid() && CharacterOwner->IsLocallyControlled();
            if (bLocalFirstPerson)
            {
                UGameplayStatics::PlaySound2D(this, GrenadeThrowSound);
            }
            else
            {
                UGameplayStatics::PlaySoundAtLocation(this, GrenadeThrowSound, Owner->GetActorLocation());
            }
            UE_LOG(LogTemp, Display,
                TEXT("PASS45_GRENADE_THROW_AUDIO_RUNTIME_READY asset=%s authored_sound=1 local_first_person=%d replicated_event=1 gameplay_authority=0 runtime_visual_acceptance=0"),
                Pass45GrenadeThrowSoundPath, bLocalFirstPerson ? 1 : 0);
        }
        else if (GetWorld() && GetWorld()->GetNetMode() != NM_DedicatedServer)
        {
            UE_LOG(LogTemp, Error,
                TEXT("PASS45_GRENADE_THROW_AUDIO_CONTENT_GAP asset=%s authored_sound=0 replicated_event=1 gameplay_authority=0 runtime_acceptance=0"),
                Pass45GrenadeThrowSoundPath);
        }

        // Pass45 item 24: the replicated cosmetic bridge exists, but the repository has no accepted authored
        // first-person hand/throw/recover sequence wired here yet. Keep gameplay flowing while making the visual
        // content gap fatal to final runtime acceptance instead of allowing a Blueprint hook to impersonate proof.
        UE_LOG(LogTemp, Warning,
            TEXT("PASS45_GRENADE_THROW_AUTHORED_ANIMATION_CONTENT_GAP event=GrenadeThrow native_authored_sequence=0 blueprint_hook_dispatched=1 second_gameplay_timer=0 runtime_visual_acceptance=0"));
    }
    BP_OnCharacterAction(Event, EventSeed);
}
