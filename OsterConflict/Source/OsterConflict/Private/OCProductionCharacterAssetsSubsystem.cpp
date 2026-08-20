#include "OCProductionCharacterAssetsSubsystem.h"

#include "OCCharacter.h"
#include "OCCharacterVisualComponent.h"
#include "OCCharacterVisualProfile.h"
#include "OCGameMode.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "UObject/UObjectGlobals.h"
#include "TimerManager.h"

namespace
{
    constexpr const TCHAR* BodyPath = TEXT("/Game/QuantumCharacter/Mesh/SKM_QuantumCharacter.SKM_QuantumCharacter");
    constexpr const TCHAR* ArmsPath = TEXT("/Game/QuantumCharacter/Mesh/Modules/SKM_Arms.SKM_Arms");

    void AddSkeletalGear(AOCCharacter& Character, USkeletalMeshComponent* Body,
        USkeletalMesh* Mesh, const FName BaseComponentName)
    {
        if (!Body || !Mesh) return;

        const FName UniqueName = MakeUniqueObjectName(&Character, USkeletalMeshComponent::StaticClass(), BaseComponentName);
        USkeletalMeshComponent* Gear = NewObject<USkeletalMeshComponent>(&Character, UniqueName);
        if (!Gear) return;

        Gear->SetupAttachment(Body);
        Gear->SetSkeletalMeshAsset(Mesh);
        Gear->SetLeaderPoseComponent(Body);
        Gear->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Gear->SetGenerateOverlapEvents(false);
        Gear->SetCanEverAffectNavigation(false);
        Gear->SetOwnerNoSee(true);
        Gear->SetCastShadow(true);
        Gear->ComponentTags.Add(FName(TEXT("OC_ProductionGear")));
        Character.AddInstanceComponent(Gear);
        Gear->RegisterComponent();
    }

    void AddStaticGear(AOCCharacter& Character, USkeletalMeshComponent* Body,
        UStaticMesh* Mesh, const FName BaseComponentName, const FName SocketName)
    {
        if (!Body || !Mesh) return;

        const FName UniqueName = MakeUniqueObjectName(&Character, UStaticMeshComponent::StaticClass(), BaseComponentName);
        UStaticMeshComponent* Gear = NewObject<UStaticMeshComponent>(&Character, UniqueName);
        if (!Gear) return;

        Gear->SetupAttachment(Body, SocketName);
        Gear->SetStaticMesh(Mesh);
        Gear->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Gear->SetGenerateOverlapEvents(false);
        Gear->SetCanEverAffectNavigation(false);
        Gear->SetOwnerNoSee(true);
        Gear->SetCastShadow(true);
        Gear->ComponentTags.Add(FName(TEXT("OC_ProductionGear")));
        Character.AddInstanceComponent(Gear);
        Gear->RegisterComponent();
    }

    FName GearTag(EOCCharacterGearClass GearClass)
    {
        switch (GearClass)
        {
        case EOCCharacterGearClass::Light: return FName(TEXT("OC_Gear_Light"));
        case EOCCharacterGearClass::Heavy: return FName(TEXT("OC_Gear_Heavy"));
        default: return FName(TEXT("OC_Gear_Standard"));
        }
    }
}

bool UOCProductionCharacterAssetsSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCProductionCharacterAssetsSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    if (InWorld.GetNetMode() == NM_DedicatedServer) return;
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    BuildProfiles();
    InWorld.GetTimerManager().SetTimer(
        RefreshTimer, this, &UOCProductionCharacterAssetsSubsystem::ApplyToCharacters,
        1.0f, true, 0.05f);
}

void UOCProductionCharacterAssetsSubsystem::BuildProfiles()
{
    if (UAProfile) return;

    auto MakeProfile = [this](EOCFactionArchetype Faction, const TCHAR* DisplayName)
    {
        UOCCharacterVisualProfile* Profile = NewObject<UOCCharacterVisualProfile>(this);
        if (!Profile) return static_cast<UOCCharacterVisualProfile*>(nullptr);
        Profile->Faction = Faction;
        Profile->DisplayName = FText::FromString(FString(DisplayName));
        Profile->ThirdPersonBodyMesh = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(BodyPath));
        Profile->FirstPersonArmsMesh = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(ArmsPath));
        return Profile;
    };

    UAProfile = MakeProfile(EOCFactionArchetype::UASpecialUnit, TEXT("UA Special Unit"));
    MaskedProfile = MakeProfile(EOCFactionArchetype::MaskedFighters, TEXT("Masked Fighters"));
    RangersProfile = MakeProfile(EOCFactionArchetype::USRangers, TEXT("US Rangers Style"));
    InsurgentsProfile = MakeProfile(EOCFactionArchetype::Insurgents, TEXT("Insurgents"));

    VestMesh = LoadObject<USkeletalMesh>(nullptr,
        TEXT("/Game/QuantumCharacter/Mesh/Modules/SKM_Bulletproof_Bege.SKM_Bulletproof_Bege"));
    DropsMesh = LoadObject<USkeletalMesh>(nullptr,
        TEXT("/Game/QuantumCharacter/Mesh/Modules/SKM_Drops_1_Bege.SKM_Drops_1_Bege"));
    HolsterMesh = LoadObject<USkeletalMesh>(nullptr,
        TEXT("/Game/QuantumCharacter/Mesh/Modules/SKM_Holster_Hard_Bege.SKM_Holster_Hard_Bege"));
    CapMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/QuantumCharacter/Mesh/Modules/SM_Cap_Bege.SM_Cap_Bege"));
}

void UOCProductionCharacterAssetsSubsystem::ApplyToCharacters()
{
    UWorld* World = GetWorld();
    if (!World || !UAProfile) return;

    for (TActorIterator<AOCCharacter> It(World); It; ++It)
    {
        AOCCharacter& Character = **It;
        UOCCharacterVisualComponent* Visual = Character.GetCharacterVisualComponent();
        if (!Visual) continue;

        Visual->SetRuntimeProfiles(UAProfile, MaskedProfile, RangersProfile, InsurgentsProfile);
        ApplyGear(Character);
    }
}

void UOCProductionCharacterAssetsSubsystem::ApplyGear(AOCCharacter& Character)
{
    UOCCharacterVisualComponent* Visual = Character.GetCharacterVisualComponent();
    USkeletalMeshComponent* Body = Character.GetMesh();
    if (!Visual || !Body || !Body->GetSkeletalMeshAsset()) return;

    const EOCCharacterGearClass GearClass = Visual->GetAppearance().GearClass;
    const FName DesiredTag = GearTag(GearClass);
    if (Character.ActorHasTag(DesiredTag)) return;

    TArray<UActorComponent*> Components;
    Character.GetComponents(Components);
    for (UActorComponent* Component : Components)
    {
        if (Component && Component->ComponentHasTag(FName(TEXT("OC_ProductionGear"))))
        {
            Component->DestroyComponent();
        }
    }

    Character.Tags.Remove(FName(TEXT("OC_Gear_Light")));
    Character.Tags.Remove(FName(TEXT("OC_Gear_Standard")));
    Character.Tags.Remove(FName(TEXT("OC_Gear_Heavy")));
    Character.Tags.Add(DesiredTag);

    switch (GearClass)
    {
    case EOCCharacterGearClass::Light:
        AddStaticGear(Character, Body, CapMesh, FName(TEXT("OC_ProductionCap")), FName(TEXT("head")));
        AddSkeletalGear(Character, Body, HolsterMesh, FName(TEXT("OC_ProductionHolster")));
        break;

    case EOCCharacterGearClass::Heavy:
        AddSkeletalGear(Character, Body, VestMesh, FName(TEXT("OC_ProductionVest")));
        AddSkeletalGear(Character, Body, DropsMesh, FName(TEXT("OC_ProductionDrops")));
        AddSkeletalGear(Character, Body, HolsterMesh, FName(TEXT("OC_ProductionHolster")));
        break;

    default:
        AddSkeletalGear(Character, Body, VestMesh, FName(TEXT("OC_ProductionVest")));
        AddSkeletalGear(Character, Body, HolsterMesh, FName(TEXT("OC_ProductionHolster")));
        break;
    }
}
