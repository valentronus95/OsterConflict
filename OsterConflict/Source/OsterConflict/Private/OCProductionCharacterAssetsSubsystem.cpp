#include "OCProductionCharacterAssetsSubsystem.h"

#include "OCCharacter.h"
#include "OCCharacterVisualComponent.h"
#include "OCCharacterVisualProfile.h"
#include "OCGameMode.h"

#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/StreamableManager.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "UObject/SoftObjectPath.h"

namespace
{
    constexpr const TCHAR* BodyPath = TEXT("/Game/QuantumCharacter/Mesh/SKM_QuantumCharacter.SKM_QuantumCharacter");
    constexpr const TCHAR* ArmsPath = TEXT("/Game/QuantumCharacter/Mesh/Modules/SKM_Arms.SKM_Arms");
    constexpr const TCHAR* VestPath = TEXT("/Game/QuantumCharacter/Mesh/Modules/SKM_Bulletproof_Bege.SKM_Bulletproof_Bege");
    constexpr const TCHAR* DropsPath = TEXT("/Game/QuantumCharacter/Mesh/Modules/SKM_Drops_1_Bege.SKM_Drops_1_Bege");
    constexpr const TCHAR* HolsterPath = TEXT("/Game/QuantumCharacter/Mesh/Modules/SKM_Holster_Hard_Bege.SKM_Holster_Hard_Bege");
    constexpr const TCHAR* CapPath = TEXT("/Game/QuantumCharacter/Mesh/Modules/SM_Cap_Bege.SM_Cap_Bege");
    constexpr const TCHAR* IdlePath = TEXT("/Game/QuantumCharacter/Demo/Animations/A_MM_Idle.A_MM_Idle");
    constexpr const TCHAR* WalkPath = TEXT("/Game/QuantumCharacter/Demo/Animations/A_MM_Walk_Fwd.A_MM_Walk_Fwd");
    constexpr const TCHAR* RunPath = TEXT("/Game/QuantumCharacter/Demo/Animations/A_MM_Run_Fwd.A_MM_Run_Fwd");
    constexpr const TCHAR* FallPath = TEXT("/Game/QuantumCharacter/Demo/Animations/A_MM_Fall_Loop.A_MM_Fall_Loop");
    constexpr const TCHAR* GrenadeThrowSoundPath = TEXT("/Game/R13/Audio/snd_throw1.snd_throw1");

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

    TArray<FSoftObjectPath> BuildCharacterPreloadPaths()
    {
        const TCHAR* Paths[] =
        {
            BodyPath,
            ArmsPath,
            VestPath,
            DropsPath,
            HolsterPath,
            CapPath,
            IdlePath,
            WalkPath,
            RunPath,
            FallPath,
            GrenadeThrowSoundPath
        };

        TArray<FSoftObjectPath> Result;
        Result.Reserve(UE_ARRAY_COUNT(Paths));
        for (const TCHAR* Path : Paths) Result.Emplace(Path);
        return Result;
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

    bInitialized = true;
    bEligible = false;
    bPreloadRequested = false;
    bPreloadComplete = false;

    if (InWorld.GetNetMode() == NM_DedicatedServer) return;
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    bEligible = true;
    BeginPreload();
}

void UOCProductionCharacterAssetsSubsystem::BeginPreload()
{
    if (!bEligible || bPreloadRequested) return;
    bPreloadRequested = true;

    const TArray<FSoftObjectPath> Paths = BuildCharacterPreloadPaths();
    PreloadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(Paths, FStreamableDelegate());
    if (!PreloadHandle.IsValid())
    {
        bPreloadComplete = true;
        UE_LOG(LogTemp, Error,
            TEXT("GAME_RECOVERY_CHARACTER_PRELOAD_FAIL handle=0 sync_spawn_loads=0"));
        return;
    }

    UE_LOG(LogTemp, Display,
        TEXT("GAME_RECOVERY_CHARACTER_PRELOAD_BEGIN assets=%d pre_spawn=1 async=1"), Paths.Num());
}

void UOCProductionCharacterAssetsSubsystem::Tick(float DeltaTime)
{
    (void)DeltaTime;
    if (!bEligible || bPreloadComplete) return;
    if (!bPreloadRequested)
    {
        BeginPreload();
        return;
    }
    if (!PreloadHandle.IsValid() || !PreloadHandle->HasLoadCompleted()) return;

    BuildProfiles();
    bPreloadComplete = true;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            RefreshTimer, this, &UOCProductionCharacterAssetsSubsystem::ApplyToCharacters,
            0.20f, true, 0.01f);
    }

    const bool bBodyReady = FSoftObjectPath(BodyPath).ResolveObject() != nullptr;
    const bool bArmsReady = FSoftObjectPath(ArmsPath).ResolveObject() != nullptr;
    UE_LOG(LogTemp, bBodyReady && bArmsReady ? Display : Error,
        TEXT("GAME_RECOVERY_CHARACTER_PRELOAD_FINISH body=%d arms=%d profiles=%d sync_spawn_loads=0 pre_spawn=1"),
        bBodyReady ? 1 : 0,
        bArmsReady ? 1 : 0,
        UAProfile ? 1 : 0);
}

TStatId UOCProductionCharacterAssetsSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCProductionCharacterAssetsSubsystem, STATGROUP_Tickables);
}

float UOCProductionCharacterAssetsSubsystem::GetCharacterAssetsProgress() const
{
    if (!bInitialized) return 0.0f;
    if (!bEligible || bPreloadComplete) return 1.0f;
    if (!bPreloadRequested || !PreloadHandle.IsValid()) return 0.05f;
    return PreloadHandle->HasLoadCompleted() ? 0.90f : 0.35f;
}

void UOCProductionCharacterAssetsSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(RefreshTimer);
    if (PreloadHandle.IsValid()) PreloadHandle->CancelHandle();
    PreloadHandle.Reset();
    AnimationStateByCharacter.Reset();
    bEligible = false;
    bPreloadComplete = true;
    Super::Deinitialize();
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

    // Packages are already resident through PreloadHandle. ResolveObject is deliberately non-blocking here.
    VestMesh = Cast<USkeletalMesh>(FSoftObjectPath(VestPath).ResolveObject());
    DropsMesh = Cast<USkeletalMesh>(FSoftObjectPath(DropsPath).ResolveObject());
    HolsterMesh = Cast<USkeletalMesh>(FSoftObjectPath(HolsterPath).ResolveObject());
    CapMesh = Cast<UStaticMesh>(FSoftObjectPath(CapPath).ResolveObject());

    IdleAnimation = Cast<UAnimSequence>(FSoftObjectPath(IdlePath).ResolveObject());
    WalkAnimation = Cast<UAnimSequence>(FSoftObjectPath(WalkPath).ResolveObject());
    RunAnimation = Cast<UAnimSequence>(FSoftObjectPath(RunPath).ResolveObject());
    FallAnimation = Cast<UAnimSequence>(FSoftObjectPath(FallPath).ResolveObject());
}

void UOCProductionCharacterAssetsSubsystem::ApplyToCharacters()
{
    UWorld* World = GetWorld();
    if (!World || !UAProfile || !bPreloadComplete) return;

    for (TActorIterator<AOCCharacter> It(World); It; ++It)
    {
        AOCCharacter& Character = **It;
        UOCCharacterVisualComponent* Visual = Character.GetCharacterVisualComponent();
        if (!Visual) continue;

        Visual->SetRuntimeProfiles(UAProfile, MaskedProfile, RangersProfile, InsurgentsProfile);
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
