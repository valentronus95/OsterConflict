#include "OCDenseGroundFoliageSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

namespace
{
    constexpr float SectorMin = -96000.0f;
    constexpr float SectorMax = 96000.0f;
    // Full profile compatibility. The normal START flow currently requests PerfProfile=LowCPU and therefore
    // uses the bounded museum-area constants below instead of slowly filling this entire 1.92 km sector.
    constexpr float GridStep = 4000.0f;
    constexpr int32 CellsPerBatch = 4;

    // Pass 36: the user's latest runtime started near 32 FPS and then collapsed to 7-4 while this subsystem
    // was still adding HISM instances across the whole sector for ~30 seconds. LowCPU is a recovery profile,
    // not a challenge to see how much vegetation a laptop can regret. Keep real imported grass around the
    // current museum playtest area, finish in well under the old population window, and stop there.
    constexpr float LowCPUHalfExtentCm = 7500.0f;
    constexpr float LowCPUGridStepCm = 1500.0f;
    constexpr int32 LowCPUCellsPerBatch = 8;

    bool IsLowCPUProfile(const UWorld& World)
    {
        const TCHAR* Value = World.URL.GetOption(TEXT("PerfProfile="), TEXT(""));
        return Value && FString(Value).Equals(TEXT("LowCPU"), ESearchCase::IgnoreCase);
    }

    UHierarchicalInstancedStaticMeshComponent* MakeFoliageHISM(
        AActor* Owner, USceneComponent* Root, UStaticMesh* Mesh, const FName Name, int32 CullEndCm)
    {
        if (!Owner || !Root || !Mesh) return nullptr;

        UHierarchicalInstancedStaticMeshComponent* Component =
            NewObject<UHierarchicalInstancedStaticMeshComponent>(Owner, Name);
        if (!Component) return nullptr;

        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(false);
        Component->SetCastShadow(false);
        Component->SetCullDistances(300, CullEndCm);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    bool IsBlockedSurface(const FHitResult& Hit)
    {
        const UPrimitiveComponent* Component = Hit.GetComponent();
        const AActor* Actor = Hit.GetActor();
        const FString ComponentName = Component ? Component->GetName() : FString();

        static const TCHAR* BlockedTerms[] =
        {
            TEXT("road"), TEXT("street"), TEXT("sidewalk"), TEXT("pavement"), TEXT("asphalt"),
            TEXT("concrete"), TEXT("path"), TEXT("bridge"), TEXT("floor"), TEXT("wall"),
            TEXT("roof"), TEXT("building"), TEXT("house"), TEXT("landmark"), TEXT("fence"),
            TEXT("plaza"), TEXT("court"), TEXT("stadium"), TEXT("parking"), TEXT("foundation")
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
                TEXT("Concrete"), TEXT("Asphalt"), TEXT("NoFoliage")
            };
            for (const FName Tag : BlockedTags)
            {
                if (Actor->ActorHasTag(Tag)) return true;
            }
        }

        return false;
    }

    UStaticMesh* LoadFirstMesh(const TArray<const TCHAR*>& Paths)
    {
        for (const TCHAR* Path : Paths)
        {
            if (UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, Path)) return Mesh;
        }
        return nullptr;
    }
}

bool UOCDenseGroundFoliageSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCDenseGroundFoliageSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    if (InWorld.GetNetMode() == NM_DedicatedServer) return;
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    bLowCPUProfile = IsLowCPUProfile(InWorld);
    if (bLowCPUProfile)
    {
        const FVector Museum = AOCWorldSectorOster::MuseumAnchor();
        PopulationMinX = Museum.X - LowCPUHalfExtentCm;
        PopulationMaxX = Museum.X + LowCPUHalfExtentCm;
        PopulationMinY = Museum.Y - LowCPUHalfExtentCm;
        PopulationMaxY = Museum.Y + LowCPUHalfExtentCm;
        ActiveGridStep = LowCPUGridStepCm;
        ActiveCellsPerBatch = LowCPUCellsPerBatch;

        UE_LOG(LogTemp, Display,
            TEXT("PASS36_LOWCPU_FOLIAGE_SCOPE_READY center=%s half_extent_m=%.1f grid_m=%.1f cells_per_batch=%d"),
            *Museum.ToCompactString(),
            LowCPUHalfExtentCm / 100.0f,
            LowCPUGridStepCm / 100.0f,
            LowCPUCellsPerBatch);
    }
    else
    {
        PopulationMinX = SectorMin;
        PopulationMaxX = SectorMax;
        PopulationMinY = SectorMin;
        PopulationMaxY = SectorMax;
        ActiveGridStep = GridStep;
        ActiveCellsPerBatch = CellsPerBatch;
    }

    InWorld.GetTimerManager().SetTimer(
        GameplayReadyTimer,
        this,
        &UOCDenseGroundFoliageSubsystem::TryPopulateWhenGameplayReady,
        0.20f,
        true,
        0.0f);
}

void UOCDenseGroundFoliageSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(GameplayReadyTimer);
        World->GetTimerManager().ClearTimer(PopulationBatchTimer);
    }
    GrassComponents.Reset();
    GroundPlants.Reset();
    Flowers.Reset();
    FoliageActor.Reset();
    Super::Deinitialize();
}

void UOCDenseGroundFoliageSubsystem::TryPopulateWhenGameplayReady()
{
    UWorld* World = GetWorld();
    if (!World || bPopulated || bPopulationStarted) return;

    if (const AOCGameMode* GameMode = World->GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    World->GetTimerManager().ClearTimer(GameplayReadyTimer);
    if (!BeginPopulation(*World))
    {
        bPopulated = true;
        return;
    }

    World->GetTimerManager().SetTimer(
        PopulationBatchTimer,
        this,
        &UOCDenseGroundFoliageSubsystem::PopulateBatch,
        0.050f,
        true,
        0.0f);
}

bool UOCDenseGroundFoliageSubsystem::BeginPopulation(UWorld& World)
{
    if (bPopulationStarted || bPopulated) return false;

    const TArray<const TCHAR*> GrassCandidates[] =
    {
        {
            TEXT("/Game/PN_FoliageCollection/Meshes/grassMesh/grass_01_01_mesh.grass_01_01_mesh"),
            TEXT("/Game/AdvancedVillagePack/Meshes/SM_GrassPatch_Var01.SM_GrassPatch_Var01")
        },
        {
            TEXT("/Game/PN_FoliageCollection/Meshes/grassMesh/grass_01_02_mesh.grass_01_02_mesh"),
            TEXT("/Game/AdvancedVillagePack/Meshes/SM_GrassPatch_Var02.SM_GrassPatch_Var02")
        },
        {
            TEXT("/Game/PN_FoliageCollection/Meshes/grassMesh/grass_01_04_mesh.grass_01_04_mesh"),
            TEXT("/Game/AdvancedVillagePack/Meshes/SM_GrassPatch_Var03.SM_GrassPatch_Var03")
        },
        {
            TEXT("/Game/PN_FoliageCollection/Meshes/grassMesh/grass_01_07_mesh.grass_01_07_mesh"),
            TEXT("/Game/AdvancedVillagePack/Meshes/SM_GrassPatch_Var01.SM_GrassPatch_Var01")
        }
    };

    UStaticMesh* GrassMeshes[UE_ARRAY_COUNT(GrassCandidates)] = {};
    bool bAnyGrass = false;
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(GrassCandidates); ++Index)
    {
        GrassMeshes[Index] = LoadFirstMesh(GrassCandidates[Index]);
        bAnyGrass |= GrassMeshes[Index] != nullptr;
    }

    UStaticMesh* GroundPlantMesh = LoadFirstMesh({
        TEXT("/Game/PN_FoliageCollection/Meshes/groundPlantMesh/ground_01_01.ground_01_01"),
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Plant.SM_Plant")
    });
    UStaticMesh* FlowerMesh = LoadFirstMesh({
        TEXT("/Game/PN_FoliageCollection/Meshes/flowerMesh/flower_01_01.flower_01_01"),
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Flower_Var01.SM_Flower_Var01")
    });

    if (!bAnyGrass)
    {
        UE_LOG(LogTemp, Error,
            TEXT("Dense foliage unavailable: grass assets are not loadable. Hydrate the PN/AdvancedVillage LFS content before playtest."));
        return false;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = TEXT("OC_DenseGroundFoliage");
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AActor* Owner = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, SpawnParams);
    if (!Owner) return false;
    Owner->Tags.Add(TEXT("OC_DenseGroundFoliage"));

    USceneComponent* Root = NewObject<USceneComponent>(Owner, TEXT("DenseFoliageRoot"));
    if (!Root)
    {
        Owner->Destroy();
        return false;
    }
    Owner->AddInstanceComponent(Root);
    Root->RegisterComponent();
    Owner->SetRootComponent(Root);

    GrassComponents.Reset();
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(GrassCandidates); ++Index)
    {
        GrassComponents.Add(MakeFoliageHISM(
            Owner, Root, GrassMeshes[Index], FName(*FString::Printf(TEXT("DenseGrass_%d"), Index)), 6000));
    }
    GroundPlants = MakeFoliageHISM(Owner, Root, GroundPlantMesh, TEXT("DenseGroundPlants"), 5000);
    Flowers = MakeFoliageHISM(Owner, Root, FlowerMesh, TEXT("DenseFlowers"), 3500);

    FoliageActor = Owner;
    RandomStream.Initialize(20260822);
    CursorX = PopulationMinX;
    CursorY = PopulationMinY;
    GrassInstances = 0;
    PlantInstances = 0;
    FlowerInstances = 0;
    bPopulationStarted = true;
    UE_LOG(LogTemp, Display,
        TEXT("PASS30_FOLIAGE_BUDGET_READY grid_cm=%.0f cells_per_batch=%d grass_cull_cm=6000 profile=%s"),
        ActiveGridStep, ActiveCellsPerBatch, bLowCPUProfile ? TEXT("LowCPU") : TEXT("Full"));
    return true;
}

void UOCDenseGroundFoliageSubsystem::PopulateBatch()
{
    UWorld* World = GetWorld();
    AActor* Owner = FoliageActor.Get();
    if (!World || !Owner || !bPopulationStarted || bPopulated)
    {
        if (World) World->GetTimerManager().ClearTimer(PopulationBatchTimer);
        return;
    }

    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(OCDenseGroundFoliage), false);
    QueryParams.AddIgnoredActor(Owner);

    int32 Processed = 0;
    while (Processed < ActiveCellsPerBatch && CursorX <= PopulationMaxX)
    {
        const float JitterExtent = bLowCPUProfile ? ActiveGridStep * 0.32f : 700.0f;
        const FVector2D Jitter(
            RandomStream.FRandRange(-JitterExtent, JitterExtent),
            RandomStream.FRandRange(-JitterExtent, JitterExtent));
        const FVector TraceStart(CursorX + Jitter.X, CursorY + Jitter.Y, 18000.0f);
        const FVector TraceEnd(CursorX + Jitter.X, CursorY + Jitter.Y, -3000.0f);

        FHitResult Hit;
        if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QueryParams) &&
            Hit.bBlockingHit && !IsBlockedSurface(Hit) && Hit.ImpactNormal.Z >= 0.72f)
        {
            const FVector BaseLocation = Hit.ImpactPoint + Hit.ImpactNormal * 2.0f;
            const int32 ClumpCount = RandomStream.RandRange(1, 2);
            for (int32 ClumpIndex = 0; ClumpIndex < ClumpCount; ++ClumpIndex)
            {
                const int32 Variant = RandomStream.RandRange(0, GrassComponents.Num() - 1);
                UHierarchicalInstancedStaticMeshComponent* Grass = GrassComponents.IsValidIndex(Variant)
                    ? GrassComponents[Variant].Get() : nullptr;
                if (!Grass) continue;

                const float Spread = bLowCPUProfile ? FMath::Min(520.0f, ActiveGridStep * 0.38f) : 700.0f;
                const FVector Offset(
                    RandomStream.FRandRange(-Spread, Spread),
                    RandomStream.FRandRange(-Spread, Spread),
                    RandomStream.FRandRange(-0.8f, 1.8f));
                const float Yaw = RandomStream.FRandRange(0.0f, 360.0f);
                const float Scale = RandomStream.FRandRange(0.80f, 1.16f);
                Grass->AddInstance(FTransform(
                    FRotator(0.0f, Yaw, 0.0f), BaseLocation + Offset, FVector(Scale)), true);
                ++GrassInstances;
            }

            if (UHierarchicalInstancedStaticMeshComponent* Plants = GroundPlants.Get();
                Plants && RandomStream.FRand() < 0.08f)
            {
                Plants->AddInstance(FTransform(
                    FRotator(0.0f, RandomStream.FRandRange(0.0f, 360.0f), 0.0f),
                    BaseLocation + FVector(RandomStream.FRandRange(-500.0f, 500.0f),
                        RandomStream.FRandRange(-500.0f, 500.0f), 1.0f),
                    FVector(RandomStream.FRandRange(0.70f, 1.02f))), true);
                ++PlantInstances;
            }

            if (UHierarchicalInstancedStaticMeshComponent* FlowerComponent = Flowers.Get();
                FlowerComponent && RandomStream.FRand() < 0.015f)
            {
                FlowerComponent->AddInstance(FTransform(
                    FRotator(0.0f, RandomStream.FRandRange(0.0f, 360.0f), 0.0f),
                    BaseLocation + FVector(RandomStream.FRandRange(-480.0f, 480.0f),
                        RandomStream.FRandRange(-480.0f, 480.0f), 1.0f),
                    FVector(RandomStream.FRandRange(0.64f, 0.88f))), true);
                ++FlowerInstances;
            }
        }

        ++Processed;
        CursorY += ActiveGridStep;
        if (CursorY > PopulationMaxY)
        {
            CursorY = PopulationMinY;
            CursorX += ActiveGridStep;
        }
    }

    if (CursorX > PopulationMaxX)
    {
        World->GetTimerManager().ClearTimer(PopulationBatchTimer);
        bPopulationStarted = false;
        bPopulated = true;
        UE_LOG(LogTemp, Display,
            TEXT("Dense Oster foliage batch population complete: %d grass, %d ground plants, %d flowers."),
            GrassInstances, PlantInstances, FlowerInstances);

        if (bLowCPUProfile)
        {
            UE_LOG(LogTemp, Display,
                TEXT("PASS36_LOWCPU_FOLIAGE_COMPLETE grass=%d plants=%d flowers=%d area_half_extent_m=%.1f"),
                GrassInstances, PlantInstances, FlowerInstances, LowCPUHalfExtentCm / 100.0f);
        }
    }
}
