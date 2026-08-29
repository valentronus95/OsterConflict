#include "OCRegionalGroundDetailSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

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

    if (!InWorld.IsGameWorld()) return;
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    // Tree content upgrade also executes at world begin-play. Defer this one-shot detail pass slightly so it reads
    // the final imported tree family transforms without adding a permanent tick owner.
    InWorld.GetTimerManager().SetTimer(
        PopulateTimerHandle,
        this,
        &UOCRegionalGroundDetailSubsystem::PopulateRegionalGroundDetail,
        0.30f,
        false);
}

void UOCRegionalGroundDetailSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PopulateTimerHandle);
    }
    Super::Deinitialize();
}

void UOCRegionalGroundDetailSubsystem::PopulateRegionalGroundDetail()
{
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
            TEXT("PASS45_BLOCK0_REGIONAL_GROUND_DETAIL_FAIL reason=oster_sector_count_%d runtime_acceptance=0"),
            SectorCount);
        return;
    }

    UInstancedStaticMeshComponent* DeciduousTrees = FindISM(Sector, TEXT("AuthoredDeciduousTrees"));
    UStaticMesh* DeadLeavesMesh = LoadObject<UStaticMesh>(nullptr, DeadLeavesMeshPath);
    if (!DeciduousTrees || !DeadLeavesMesh || DeciduousTrees->GetInstanceCount() <= 0)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_BLOCK0_REGIONAL_GROUND_DETAIL_FAIL reason=source_or_asset_missing deciduous_component=%d deciduous_instances=%d dead_leaves_loaded=%d runtime_acceptance=0"),
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
    FRandomStream RandomStream(0x4F535445); // "OSTE", deterministic across runs.

    int32 AddedInstances = 0;
    int32 TraceRejected = 0;
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
            TEXT("PASS45_BLOCK0_REGIONAL_GROUND_DETAIL_FAIL reason=no_grounded_leaf_instances traces_rejected=%d runtime_acceptance=0"),
            TraceRejected);
        return;
    }

    DetailOwner->Tags.AddUnique(TEXT("OC_Block0RegionalGroundDetail"));
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_BLOCK0_REGIONAL_GROUND_DETAIL_WIRED asset=SM_DeadLeaves source_tree_family=AuthoredDeciduousTrees tree_instances=%d leaf_instances=%d max_instances=%d profile=%s collision=0 navigation=0 shadow=0 cull_end_cm=6500 deterministic=1 permanent_tick=0 runtime_acceptance=0"),
        TreeCount,
        AddedInstances,
        MaxLeafInstances,
        bLowCPU ? TEXT("LowCPU") : TEXT("Full"));
}
