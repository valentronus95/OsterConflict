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
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

namespace
{
    bool ApplyBundledMannequinFallback(USkeletalMeshComponent* Mesh, int32 VariantSeed)
    {
        if (!Mesh) return false;

        const bool bUseQuinn = (VariantSeed & 1) != 0;
        const TCHAR* MeshPath = bUseQuinn
            ? TEXT("/Game/SampleAnimationPack/Demo/Characters/Mannequins/Meshes/SKM_Quinn.SKM_Quinn")
            : TEXT("/Game/SampleAnimationPack/Demo/Characters/Mannequins/Meshes/SKM_Manny.SKM_Manny");
        const TCHAR* AnimPath = bUseQuinn
            ? TEXT("/Game/SampleAnimationPack/Demo/Characters/Mannequins/Animations/ABP_Quinn.ABP_Quinn_C")
            : TEXT("/Game/SampleAnimationPack/Demo/Characters/Mannequins/Animations/ABP_Manny.ABP_Manny_C");

        USkeletalMesh* LoadedMesh = LoadObject<USkeletalMesh>(nullptr, MeshPath);
        if (!LoadedMesh) return false;

        Mesh->SetSkeletalMeshAsset(LoadedMesh);
        if (UClass* AnimClass = LoadClass<UAnimInstance>(nullptr, AnimPath))
        {
            Mesh->SetAnimInstanceClass(AnimClass);
        }
        Mesh->SetOwnerNoSee(true);
        Mesh->SetVisibility(true, true);
        return true;
    }
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
        if (USkeletalMesh* Loaded = Profile->ThirdPersonBodyMesh.LoadSynchronous())
        {
            ThirdPersonMesh->SetSkeletalMeshAsset(Loaded);
            if (Profile->ThirdPersonAnimClass) ThirdPersonMesh->SetAnimInstanceClass(Profile->ThirdPersonAnimClass);
            ThirdPersonMesh->SetOwnerNoSee(true);
            ThirdPersonMesh->SetVisibility(true, true);
            bHasProductionBody = true;
        }
    }

    // R13 already ships Manny/Quinn and their locomotion animation blueprints. Use them as the visual bridge
    // before ever exposing the source-only cube/cylinder soldier. Authored faction soldiers still override this.
    if (!bHasProductionBody && ThirdPersonMesh)
    {
        bHasProductionBody = ApplyBundledMannequinFallback(ThirdPersonMesh, CurrentAppearance.VariantSeed);
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
            if (USkeletalMesh* Loaded = Profile->FirstPersonArmsMesh.LoadSynchronous())
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
            Comp->SetVisibility(false, true);
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

    for (UStaticMeshComponent* Part : FirstPersonProxyParts)
    {
        if (Part) Part->SetVisibility(false, true);
    }
}

void UOCCharacterVisualComponent::ApplyProxyTint(EOCFactionArchetype Faction)
{
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
    BP_OnCharacterAction(Event, EventSeed);
}
