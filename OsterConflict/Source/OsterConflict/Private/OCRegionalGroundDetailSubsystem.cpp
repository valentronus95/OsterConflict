#include "OCRegionalGroundDetailSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "UObject/SoftObjectPath.h"

namespace
{
    const TCHAR* DeadLeavesMeshPath =
        TEXT("/Game/KiteDemo/Environments/Foliage/Leaves/SM_DeadLeaves.SM_DeadLeaves");

    constexpr int32 FullMaxLeafInstances = 240;
    constexpr int32 LowCPUMaxLeafInstances = 96;
    constexpr int32 FullLeavesPerTree = 2;
    constexpr int32 LowCPULeavesPerTree = 1;

    bool IsLowCPUProfile(const UWorld& World)
    {
        const TCHAR* Value = World.URL.GetOption(TEXT("PerfProfile="), TEXT(""));
        return Value && FString(Value).Equals(TEXT("LowCPU"), ESearchCase::IgnoreCase);
    }

    UInstancedStaticMeshComponent* FindISM(AActor* Actor, const FName Name)
    {
        if (!Actor) return nullptr;
        TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
        Actor->GetComponents(Components);
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            if (Component && Component->GetFName() == Name) return Component;
        }
        return nullptr;
    }

    bool IsBlockedGroundDetailSurface(const FHitResult& Hit)
    {
        const UPrimitiveComponent* Component = Hit.GetComponent();
        const AActor* Actor = Hit.GetActor();
        const FString ComponentName = Component ? Component->GetName() : FString();

        static const TCHAR* BlockedTerms[] =
        {
            TEXT("road"), TEXT("street"), TEXT("sidewalk"), TEXT("pavement"), TEXT("asphalt"),
            TEXT("concrete"), TEXT("path"), TEXT("bridge"), TEXT("floor"), TEXT("wall"),
            TEXT("roof"), TEXT("building"), TEXT("house"), TEXT("landmark"), TEXT("fence"),
            TEXT("plaza"), TEXT("court"), TEXT("stadium"), TEXT("parking"), TEXT("foundation"),
            TEXT("water"), TEXT("river"), TEXT("lake"), TEXT("pond"), TEXT("canal"), TEXT("reservoir")
        };
        for (const TCHAR* Term : BlockedTerms)
        {
            if (ComponentName.Contains(Term, ESearchCase::IgnoreCase)) return true;
        }

        if (Actor)
        {
            static const FName BlockedTags[] =
            {
                TEXT("Road"), TEXT("Street"), TEXT("Building"), TEXT("Bridge"),
                TEXT("Concrete"), TEXT("Asphalt"), TEXT("Water"), TEXT("River"),
                TEXT("Lake"), TEXT("Pond"), TEXT("Canal"), TEXT("Reservoir"), TEXT("NoFoliage")
            };
            for (const FName Tag : BlockedTags)
            {
                if (Actor->ActorHasTag(Tag)) return true;
            }
        }

        return false;
    }
}

bool UOCRegionalGroundDetailSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && World->GetNetMode() != NM_DedicatedServer &&
        (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCRegionalGroundDetailSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    bPreloadRequested = false;
    bPopulationFinished = false;
    PreloadHandle.Reset();

    if (!InWorld.IsGameWorld()) return;
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    RequestRegionalGroundDetailPreload();
}

void UOCRegionalGroundDetailSubsystem::RequestRegionalGroundDetailPreload()
{
    if (bPreloadRequested || bPopulationFinished) return;
    bPreloadRequested = true;

    TArray<FSoftObjectPath> Paths;
    Paths.Emplace(DeadLeavesMeshPath);
    PreloadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
        Paths,
        FStreamableDelegate::CreateUObject(this, &UOCRegionalGroundDetailSubsystem::HandleRegionalGroundDetailPreloadComplete),
        FStreamableManager::AsyncLoadHighPriority,
        false,
        false,
        TEXT("GameRecoveryRegionalGroundDetailPreload"));

    if (!PreloadHandle.IsValid())
    {
        bPopulationFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("GAME_RECOVERY_REGIONAL_GROUND_PRELOAD_FAIL handle=0 sync_load=0 resident_only=1 runtime_acceptance=0"));
        return;
    }

    UE_LOG(LogTemp, Display,
        TEXT("GAME_RECOVERY_REGIONAL_GROUND_PRELOAD_BEGIN assets=1 async=1 pre_spawn=1 sync_load=0"));
}

void UOCRegionalGroundDetailSubsystem::HandleRegionalGroundDetailPreloadComplete()
{
    if (bPopulationFinished) return;
    if (!PreloadHandle.IsValid() || !PreloadHandle->HasLoadCompleted())
    {
        bPopulationFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("GAME_RECOVERY_REGIONAL_GROUND_PRELOAD_FAIL handle_or_completion=0 sync_load=0 resident_only=1 runtime_acceptance=0"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World || !World->IsGameWorld())
    {
        bPopulationFinished = true;
        return;
    }

    // One short deferred pass gives the authored Oster sector its BeginPlay work without polling after spawn.
    World->GetTimerManager().SetTimer(
        PopulateTimerHandle,
        this,
        &UOCRegionalGroundDetailSubsystem::PopulateRegionalGroundDetail,
        0.30f,
        false);

    UE_LOG(LogTemp, Display,
        TEXT("GAME_RECOVERY_REGIONAL_GROUND_PRELOAD_READY async=1 resident_only=1 sync_load=0 one_shot=1 pre_spawn=1"));
}

void UOCRegionalGroundDetailSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PopulateTimerHandle);
    }
    if (PreloadHandle.IsValid()) PreloadHandle->CancelHandle();
    PreloadHandle.Reset();
    bPreloadRequested = false;
    bPopulationFinished = true;
    Super::Deinitialize();
}

void UOCRegionalGroundDetailSubsystem::PopulateRegionalGroundDetail()
{
    if (bPopulationFinished) return;
    bPopulationFinished = true;

    UWorld* World = GetWorld();
    if (!World || !World->IsGameWorld()) return;

    AOCWorldSectorOster* Sector = nullptr;
    int32 SectorCount = 0;
    for (TActorIterator<AOCWorldSectorOster> It(World); It; ++It)
    {
        Sector = *It;
        ++SectorCount;
    }
    if (SectorCount != 1 || !Sector)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_BLOCK0_REGIONAL_GROUND_DETAIL_FAIL reason=oster_sector_count_%d runtime_acceptance=0 sync_load=0"),
            SectorCount);
        return;
    }

    UInstancedStaticMeshComponent* DeciduousTrees = FindISM(Sector, TEXT("AuthoredDeciduousTrees"));
    UStaticMesh* DeadLeavesMesh = Cast<UStaticMesh>(FSoftObjectPath(DeadLeavesMeshPath).ResolveObject());
    if (!DeciduousTrees || !DeadLeavesMesh || DeciduousTrees->GetInstanceCount() <= 0)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_BLOCK0_REGIONAL_GROUND_DETAIL_FAIL reason=source_or_preload_gap deciduous_component=%d deciduous_instances=%d dead_leaves_resident=%d runtime_acceptance=0 sync_load=0 resident_only=1"),
            DeciduousTrees ? 1 : 0,
            DeciduousTrees ? DeciduousTrees->GetInstanceCount() : 0,
            DeadLeavesMesh ? 1 : 0);
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = TEXT("OC_Block0RegionalGroundDetail");
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AActor* DetailOwner = World->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, SpawnParams);
    if (!DetailOwner)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_BLOCK0_REGIONAL_GROUND_DETAIL_FAIL reason=detail_owner_spawn_failed runtime_acceptance=0"));
        return;
    }

    UHierarchicalInstancedStaticMeshComponent* Leaves =
        NewObject<UHierarchicalInstancedStaticMeshComponent>(DetailOwner, TEXT("OC_RegionalDeadLeaves"));
    if (!Leaves)
    {
        DetailOwner->Destroy();
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_BLOCK0_REGIONAL_GROUND_DETAIL_FAIL reason=hism_create_failed runtime_acceptance=0"));
        return;
    }

    Leaves->SetStaticMesh(DeadLeavesMesh);
    Leaves->SetMobility(EComponentMobility::Static);
    Leaves->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Leaves->SetGenerateOverlapEvents(false);
    Leaves->SetCanEverAffectNavigation(false);
    Leaves->SetCastShadow(false);
    Leaves->SetCullDistances(250, 6500);
    DetailOwner->AddInstanceComponent(Leaves);
    DetailOwner->SetRootComponent(Leaves);
    Leaves->RegisterComponent();

    const bool bLowCPU = IsLowCPUProfile(*World);
    const int32 LeavesPerTree = bLowCPU ? LowCPULeavesPerTree : FullLeavesPerTree;
    const int32 MaxLeafInstances = bLowCPU ? LowCPUMaxLeafInstances : FullMaxLeafInstances;
    FRandomStream RandomStream(0x4F535445);

    int32 AddedInstances = 0;
    int32 TraceRejected = 0;
    int32 BlockedSurfaceRejected = 0;
    const int32 TreeCount = DeciduousTrees->GetInstanceCount();
    for (int32 TreeIndex = 0; TreeIndex < TreeCount && AddedInstances < MaxLeafInstances; ++TreeIndex)
    {
        FTransform TreeTransform;
        if (!DeciduousTrees->GetInstanceTransform(TreeIndex, TreeTransform, true)) continue;

        const FVector TreeLocation = TreeTransform.GetLocation();
        for (int32 LocalIndex = 0; LocalIndex < LeavesPerTree && AddedInstances < MaxLeafInstances; ++LocalIndex)
        {
            const float Radius = RandomStream.FRandRange(75.0f, 230.0f);
            const float Angle = RandomStream.FRandRange(0.0f, 2.0f * PI);
            const FVector CandidateXY = TreeLocation + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.0f);
            const FVector TraceStart(CandidateXY.X, CandidateXY.Y, TreeLocation.Z + 180.0f);
            const FVector TraceEnd(CandidateXY.X, CandidateXY.Y, TreeLocation.Z - 260.0f);

            FHitResult Hit;
            FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(OCBlock0RegionalGroundDetail), false, DetailOwner);
            QueryParams.AddIgnoredActor(DetailOwner);
            if (!World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QueryParams) ||
                Hit.ImpactNormal.Z < 0.82f)
            {
                ++TraceRejected;
                continue;
            }
            if (IsBlockedGroundDetailSurface(Hit))
            {
                ++BlockedSurfaceRejected;
                continue;
            }

            const float UniformScale = RandomStream.FRandRange(0.72f, 1.12f);
            const FRotator Rotation(0.0f, RandomStream.FRandRange(0.0f, 360.0f), 0.0f);
            const FVector Location = Hit.ImpactPoint + Hit.ImpactNormal * 1.2f;
            Leaves->AddInstance(FTransform(Rotation, Location, FVector(UniformScale)), true);
            ++AddedInstances;
        }
    }

    if (AddedInstances <= 0)
    {
        DetailOwner->Destroy();
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_BLOCK0_REGIONAL_GROUND_DETAIL_FAIL reason=no_grounded_leaf_instances traces_rejected=%d blocked_surfaces=%d runtime_acceptance=0"),
            TraceRejected,
            BlockedSurfaceRejected);
        return;
    }

    DetailOwner->Tags.AddUnique(TEXT("OC_Block0RegionalGroundDetail"));
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_BLOCK0_REGIONAL_GROUND_DETAIL_WIRED asset=SM_DeadLeaves source_tree_family=AuthoredDeciduousTrees tree_instances=%d leaf_instances=%d max_instances=%d profile=%s collision=0 navigation=0 shadow=0 cull_end_cm=6500 deterministic=1 permanent_tick=0 candidate_surface_guard=1 water_surface_guard=1 trace_rejected=%d blocked_surface_rejected=%d runtime_acceptance=0 async_preloaded=1 sync_load=0 resident_only=1 one_shot=1"),
        TreeCount,
        AddedInstances,
        MaxLeafInstances,
        bLowCPU ? TEXT("LowCPU") : TEXT("Full"),
        TraceRejected,
        BlockedSurfaceRejected);
}
