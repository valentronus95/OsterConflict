#include "OCProductionCharacterAssetsSubsystem.h"

#include "OCCharacter.h"
#include "OCCharacterVisualComponent.h"
#include "OCCharacterVisualProfile.h"
#include "OCGameMode.h"
#include "OCLocalInboxRuntimeSubsystem.h"

#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
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

    void ApplyLocalSkinIfAvailable(UOCCharacterVisualProfile* Profile, int32 SkinIndex)
    {
        if (!Profile) return;
        if (USkeletalMesh* LocalSkin = UOCLocalInboxRuntimeSubsystem::LoadCompatibleCharacterSkin(SkinIndex))
        {
            Profile->ThirdPersonBodyMesh = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(LocalSkin->GetPathName()));
            UE_LOG(LogTemp, Display,
                TEXT("PASS45_LOCAL_CHARACTER_SKIN_BOUND faction=%s index=%d mesh=%s skeleton=%s"),
                *Profile->DisplayName.ToString(), SkinIndex, *LocalSkin->GetPathName(),
                LocalSkin->GetSkeleton() ? *LocalSkin->GetSkeleton()->GetPathName() : TEXT("<none>"));
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
        0.20f, true, 0.05f);
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

    // User-added human models stop being passive files. Every skin that the intake proved compatible with
    // the active Quantum skeleton is assigned to live faction profiles. When fewer than four are supplied,
    // cycle the verified skins rather than falling back to a cube/proxy for the remaining factions.
    const int32 LocalSkinCount = UOCLocalInboxRuntimeSubsystem::GetCompatibleCharacterSkinCount();
    if (LocalSkinCount > 0)
    {
        ApplyLocalSkinIfAvailable(UAProfile, 0 % LocalSkinCount);
        ApplyLocalSkinIfAvailable(MaskedProfile, 1 % LocalSkinCount);
        ApplyLocalSkinIfAvailable(RangersProfile, 2 % LocalSkinCount);
        ApplyLocalSkinIfAvailable(InsurgentsProfile, 3 % LocalSkinCount);
        UE_LOG(LogTemp, Display, TEXT("PASS45_LOCAL_CHARACTER_SKIN_POOL_READY compatible_skins=%d live_factions=4"),
            LocalSkinCount);
    }

    VestMesh = LoadObject<USkeletalMesh>(nullptr,
        TEXT("/Game/QuantumCharacter/Mesh/Modules/SKM_Bulletproof_Bege.SKM_Bulletproof_Bege"));
    DropsMesh = LoadObject<USkeletalMesh>(nullptr,
        TEXT("/Game/QuantumCharacter/Mesh/Modules/SKM_Drops_1_Bege.SKM_Drops_1_Bege"));
    HolsterMesh = LoadObject<USkeletalMesh>(nullptr,
        TEXT("/Game/QuantumCharacter/Mesh/Modules/SKM_Holster_Hard_Bege.SKM_Holster_Hard_Bege"));
    CapMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/QuantumCharacter/Mesh/Modules/SM_Cap_Bege.SM_Cap_Bege"));

    // These sequences ship with QuantumCharacter itself, so unlike the separate sample animation
    // pack they can be compatibility-checked against the production character skeleton at runtime.
    IdleAnimation = LoadObject<UAnimSequence>(nullptr,
        TEXT("/Game/QuantumCharacter/Demo/Animations/A_MM_Idle.A_MM_Idle"));
    WalkAnimation = LoadObject<UAnimSequence>(nullptr,
        TEXT("/Game/QuantumCharacter/Demo/Animations/A_MM_Walk_Fwd.A_MM_Walk_Fwd"));
    RunAnimation = LoadObject<UAnimSequence>(nullptr,
        TEXT("/Game/QuantumCharacter/Demo/Animations/A_MM_Run_Fwd.A_MM_Run_Fwd"));
    FallAnimation = LoadObject<UAnimSequence>(nullptr,
        TEXT("/Game/QuantumCharacter/Demo/Animations/A_MM_Fall_Loop.A_MM_Fall_Loop"));
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

        const int32 LocalSkinCount = UOCLocalInboxRuntimeSubsystem::GetCompatibleCharacterSkinCount();
        if (LocalSkinCount > 0)
        {
            const int32 Seed = FMath::Max(1, Visual->GetAppearance().VariantSeed);
            const int32 SkinIndex = (Seed - 1) % LocalSkinCount;
            if (USkeletalMesh* LocalSkin = UOCLocalInboxRuntimeSubsystem::LoadCompatibleCharacterSkin(SkinIndex))
            {
                if (USkeletalMeshComponent* Body = Character.GetMesh())
                {
                    if (Body->GetSkeletalMeshAsset() != LocalSkin) Body->SetSkeletalMeshAsset(LocalSkin);
                }
            }
        }

        ApplyGear(Character);
        ApplyAnimation(Character);
    }

    for (auto It = AnimationStateByCharacter.CreateIterator(); It; ++It)
    {
        if (!It.Key().IsValid()) It.RemoveCurrent();
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

void UOCProductionCharacterAssetsSubsystem::ApplyAnimation(AOCCharacter& Character)
{
    USkeletalMeshComponent* Body = Character.GetMesh();
    USkeletalMesh* BodyMesh = Body ? Body->GetSkeletalMeshAsset() : nullptr;
    if (!Body || !BodyMesh) return;

    UAnimSequence* DesiredAnimation = IdleAnimation;
    uint8 DesiredState = 0;

    const UCharacterMovementComponent* Movement = Character.GetCharacterMovement();
    if (Movement && Movement->IsFalling())
    {
        DesiredAnimation = FallAnimation;
        DesiredState = 3;
    }
    else
    {
        const float Speed2D = Character.GetVelocity().Size2D();
        if (Speed2D > 420.0f)
        {
            DesiredAnimation = RunAnimation;
            DesiredState = 2;
        }
        else if (Speed2D > 10.0f)
        {
            DesiredAnimation = WalkAnimation;
            DesiredState = 1;
        }
    }

    if (!DesiredAnimation || !DesiredAnimation->GetSkeleton() ||
        DesiredAnimation->GetSkeleton() != BodyMesh->GetSkeleton())
    {
        return;
    }

    const TWeakObjectPtr<AOCCharacter> Key(&Character);
    if (const uint8* CurrentState = AnimationStateByCharacter.Find(Key))
    {
        if (*CurrentState == DesiredState) return;
    }

    Body->PlayAnimation(DesiredAnimation, true);
    AnimationStateByCharacter.Add(Key, DesiredState);
}
