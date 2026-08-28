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
    // PASS45 Block 0 uses the same compact central-Oster authoring bounds as AOCWorldSectorOster.
    // LowCPU is a density/render-budget policy, never a museum-only spatial crop.
    constexpr float CompactMinX = -78000.0f;
    constexpr float CompactMaxX =  18000.0f;
    constexpr float CompactMinY = -12000.0f;
    constexpr float CompactMaxY =  82000.0f;
    constexpr float CompactWidthCm = CompactMaxX - CompactMinX;
    constexpr float CompactHeightCm = CompactMaxY - CompactMinY;
    static_assert(CompactWidthCm == 96000.0f, "Block0 compact foliage width must remain 960 m");
    static_assert(CompactHeightCm == 94000.0f, "Block0 compact foliage height must remain 940 m");

    constexpr float FullGridStepCm = 1000.0f;
    constexpr int32 FullCellsPerBatch = 32;
    constexpr int32 FullGrassCullEndCm = 18000;
    constexpr int32 FullPlantCullEndCm = 12000;
    constexpr int32 FullFlowerCullEndCm = 8000;

    constexpr float LowCPUGridStepCm = 1500.0f;
    constexpr int32 LowCPUCellsPerBatch = 48;
    constexpr int32 LowCPUGrassCullEndCm = 14000;
    constexpr int32 LowCPUPlantCullEndCm = 9000;
    constexpr int32 LowCPUFlowerCullEndCm = 6000;

    const FName Block0PopulationCompleteTag(TEXT("OC_Block0FullMapGrassComplete"));

    bool IsLowCPUProfile(const UWorld& World)
    {
        const TCHAR* Value = World.URL.GetOption(TEXT("PerfProfile="), TEXT(""));
        return Value && FString(Value).Equals(TEXT("LowCPU"), ESearchCase::IgnoreCase);
    }

    bool IsInside2DBox(const FVector& Point, const FVector& Center, const float HalfX, const float HalfY)
    {
        return FMath::Abs(Point.X - Center.X) <= HalfX && FMath::Abs(Point.Y - Center.Y) <= HalfY;
    }

    bool IsMaintainedCivicZone(const FVector& Point)
    {
        static const FVector Park = AOCWorldSectorOster::ParkAnchor();
        static const FVector College = AOCWorldSectorOster::CollegeAnchor();
        static const FVector Stadium = AOCWorldSectorOster::StadiumAnchor();
        return IsInside2DBox(Point, Park, 10500.0f, 8000.0f) ||
            IsInside2DBox(Point, College + FVector(0.0f, 4200.0f, 0.0f), 7000.0f, 6500.0f) ||
            IsInside2DBox(Point, Stadium, 7600.0f, 5600.0f);
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
    PopulationMinX = CompactMinX;
    PopulationMaxX = CompactMaxX;
    PopulationMinY = CompactMinY;
    PopulationMaxY = CompactMaxY;
    ActiveGridStep = bLowCPUProfile ? LowCPUGridStepCm : FullGridStepCm;
    ActiveCellsPerBatch = bLowCPUProfile ? LowCPUCellsPerBatch : FullCellsPerBatch;

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_BLOCK0_FULL_MAP_GRASS_SCOPE_READY bounds_m=960x940 x_m=[-780,180] y_m=[-120,820] profile=%s grid_m=%.1f cells_per_batch=%d museum_only=0 full_playable_bounds=1 content_intake=KiteDemo"),
        bLowCPUProfile ? TEXT("LowCPU") : TEXT("Full"),
        ActiveGridStep / 100.0f,
        ActiveCellsPerBatch);

    // Allow the existing foliage runtime guard to retire source ground-cover proxies before planting traces begin.
    InWorld.GetTimerManager().SetTimer(
        GameplayReadyTimer,
        this,
        &UOCDenseGroundFoliageSubsystem::TryPopulateWhenGameplayReady,
        0.20f,
        true,
        0.75f);
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

    // Intake assets are selected first; already-accepted historical packs remain explicit fallbacks.
    const TArray<const TCHAR*> GrassCandidates[] =
    {
        {
            TEXT("/Game/KiteDemo/Environments/Foliage/Grass/FieldGrass/SM_FieldGrass_01.SM_FieldGrass_01"),
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
        TEXT("/Game/KiteDemo/Environments/Foliage/Ferns/SM_Fern_01.SM_Fern_01"),
        TEXT("/Game/PN_FoliageCollection/Meshes/groundPlantMesh/ground_01_01.ground_01_01"),
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Plant.SM_Plant")
    });
    UStaticMesh* FlowerMesh = LoadFirstMesh({
        TEXT("/Game/KiteDemo/Environments/Foliage/Flowers/FieldScabious/SM_FieldScabious_01.SM_FieldScabious_01"),
        TEXT("/Game/KiteDemo/Environments/Foliage/Flowers/Buttercup/SM_Buttercup_Patch_01.SM_Buttercup_Patch_01"),
        TEXT("/Game/PN_FoliageCollection/Meshes/flowerMesh/flower_01_01.flower_01_01"),
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Flower_Var01.SM_Flower_Var01")
    });

    if (!bAnyGrass)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_BLOCK0_FULL_MAP_GRASS_FAIL reason=grass_assets_not_loadable full_playable_bounds=0 content_intake=KiteDemo"));
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

    const int32 GrassCullEnd = bLowCPUProfile ? LowCPUGrassCullEndCm : FullGrassCullEndCm;
    const int32 PlantCullEnd = bLowCPUProfile ? LowCPUPlantCullEndCm : FullPlantCullEndCm;
    const int32 FlowerCullEnd = bLowCPUProfile ? LowCPUFlowerCullEndCm : FullFlowerCullEndCm;

    GrassComponents.Reset();
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(GrassCandidates); ++Index)
    {
        GrassComponents.Add(MakeFoliageHISM(
            Owner, Root, GrassMeshes[Index], FName(*FString::Printf(TEXT("DenseGrass_%d"), Index)), GrassCullEnd));
    }
    GroundPlants = MakeFoliageHISM(Owner, Root, GroundPlantMesh, TEXT("DenseGroundPlants"), PlantCullEnd);
    Flowers = MakeFoliageHISM(Owner, Root, FlowerMesh, TEXT("DenseFlowers"), FlowerCullEnd);

    FoliageActor = Owner;
    RandomStream.Initialize(20260828);
    CursorX = PopulationMinX;
    CursorY = PopulationMinY;
    GrassInstances = 0;
    PlantInstances = 0;
    FlowerInstances = 0;
    ProcessedCells = 0;
    CandidateTraceAttempts = 0;
    CandidateAccepted = 0;
    CandidateRejectedBlocked = 0;
    CandidateRejectedTrace = 0;
    CandidateRejectedBounds = 0;
    bPopulationStarted = true;
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_BLOCK0_FOLIAGE_BUDGET_READY grid_cm=%.0f cells_per_batch=%d grass_cull_cm=%d plant_cull_cm=%d flower_cull_cm=%d profile=%s full_playable_bounds=1 candidate_surface_guard=1 water_surface_guard=1 content_intake=KiteDemo"),
        ActiveGridStep,
        ActiveCellsPerBatch,
        GrassCullEnd,
        PlantCullEnd,
        FlowerCullEnd,
        bLowCPUProfile ? TEXT("LowCPU") : TEXT("Full"));
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

    auto ResolveGrassComponent = [this](const int32 PreferredVariant)
    {
        for (int32 Offset = 0; Offset < GrassComponents.Num(); ++Offset)
        {
            const int32 Variant = (PreferredVariant + Offset) % GrassComponents.Num();
            if (GrassComponents.IsValidIndex(Variant))
            {
                if (UHierarchicalInstancedStaticMeshComponent* Component = GrassComponents[Variant].Get())
                    return Component;
            }
        }
        return static_cast<UHierarchicalInstancedStaticMeshComponent*>(nullptr);
    };

    // Cell traces are a cheap preflight. Every randomized final XY is independently traced before AddInstance.
    auto ResolveCandidateSurface = [this, World, &QueryParams](const FVector2D& XY, FVector& OutLocation)
    {
        if (XY.X < PopulationMinX || XY.X > PopulationMaxX ||
            XY.Y < PopulationMinY || XY.Y > PopulationMaxY)
        {
            ++CandidateRejectedBounds;
            return false;
        }

        ++CandidateTraceAttempts;
        const FVector TraceStart(XY.X, XY.Y, 18000.0f);
        const FVector TraceEnd(XY.X, XY.Y, -3000.0f);
        FHitResult CandidateHit;
        if (!World->LineTraceSingleByChannel(
                CandidateHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams) ||
            !CandidateHit.bBlockingHit)
        {
            ++CandidateRejectedTrace;
            return false;
        }

        if (CandidateHit.ImpactNormal.Z < 0.72f || IsBlockedSurface(CandidateHit))
        {
            ++CandidateRejectedBlocked;
            return false;
        }

        OutLocation = CandidateHit.ImpactPoint + CandidateHit.ImpactNormal * 2.0f;
        ++CandidateAccepted;
        return true;
    };

    int32 ProcessedThisBatch = 0;
    while (ProcessedThisBatch < ActiveCellsPerBatch && CursorX <= PopulationMaxX)
    {
        const float JitterExtent = ActiveGridStep * 0.30f;
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
            const bool bMaintained = IsMaintainedCivicZone(BaseLocation);
            const int32 ClumpCount = bMaintained
                ? RandomStream.RandRange(bLowCPUProfile ? 3 : 4, bLowCPUProfile ? 4 : 5)
                : RandomStream.RandRange(bLowCPUProfile ? 2 : 3, bLowCPUProfile ? 4 : 5);
            const float Spread = FMath::Min(620.0f, ActiveGridStep * 0.42f);

            for (int32 ClumpIndex = 0; ClumpIndex < ClumpCount; ++ClumpIndex)
            {
                const int32 VariantCount = GrassComponents.Num();
                if (VariantCount <= 0) break;
                const int32 PreferredVariant = bMaintained
                    ? RandomStream.RandRange(0, FMath::Min(1, VariantCount - 1))
                    : RandomStream.RandRange(0, VariantCount - 1);
                UHierarchicalInstancedStaticMeshComponent* Grass = ResolveGrassComponent(PreferredVariant);
                if (!Grass) continue;

                const FVector2D CandidateXY(
                    BaseLocation.X + RandomStream.FRandRange(-Spread, Spread),
                    BaseLocation.Y + RandomStream.FRandRange(-Spread, Spread));
                FVector CandidateLocation;
                if (!ResolveCandidateSurface(CandidateXY, CandidateLocation)) continue;

                const float Yaw = RandomStream.FRandRange(0.0f, 360.0f);
                const float Scale = bMaintained
                    ? RandomStream.FRandRange(0.68f, 0.94f)
                    : RandomStream.FRandRange(0.82f, 1.18f);
                Grass->AddInstance(FTransform(
                    FRotator(0.0f, Yaw, 0.0f), CandidateLocation, FVector(Scale)), true);
                ++GrassInstances;
            }

            const float PlantChance = bMaintained ? 0.03f : 0.12f;
            if (UHierarchicalInstancedStaticMeshComponent* Plants = GroundPlants.Get();
                Plants && RandomStream.FRand() < PlantChance)
            {
                const FVector2D CandidateXY(
                    BaseLocation.X + RandomStream.FRandRange(-500.0f, 500.0f),
                    BaseLocation.Y + RandomStream.FRandRange(-500.0f, 500.0f));
                FVector CandidateLocation;
                if (ResolveCandidateSurface(CandidateXY, CandidateLocation))
                {
                    Plants->AddInstance(FTransform(
                        FRotator(0.0f, RandomStream.FRandRange(0.0f, 360.0f), 0.0f),
                        CandidateLocation,
                        FVector(RandomStream.FRandRange(0.70f, 1.02f))), true);
                    ++PlantInstances;
                }
            }

            const float FlowerChance = bMaintained ? 0.008f : 0.025f;
            if (UHierarchicalInstancedStaticMeshComponent* FlowerComponent = Flowers.Get();
                FlowerComponent && RandomStream.FRand() < FlowerChance)
            {
                const FVector2D CandidateXY(
                    BaseLocation.X + RandomStream.FRandRange(-480.0f, 480.0f),
                    BaseLocation.Y + RandomStream.FRandRange(-480.0f, 480.0f));
                FVector CandidateLocation;
                if (ResolveCandidateSurface(CandidateXY, CandidateLocation))
                {
                    FlowerComponent->AddInstance(FTransform(
                        FRotator(0.0f, RandomStream.FRandRange(0.0f, 360.0f), 0.0f),
                        CandidateLocation,
                        FVector(RandomStream.FRandRange(0.64f, 0.88f))), true);
                    ++FlowerInstances;
                }
            }
        }

        ++ProcessedThisBatch;
        ++ProcessedCells;
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
        Owner->Tags.Add(Block0PopulationCompleteTag);

        UE_LOG(LogTemp, Display,
            TEXT("PASS45_BLOCK0_FULL_MAP_GRASS_READY bounds_m=960x940 grass=%d plants=%d flowers=%d processed_cells=%d profile=%s population_complete=1 full_playable_bounds=1 museum_only=0 candidate_surface_guard=1 water_surface_guard=1 candidate_traces=%d candidate_accepted=%d candidate_rejected_blocked=%d candidate_rejected_trace=%d candidate_rejected_bounds=%d content_intake=KiteDemo runtime_acceptance=0"),
            GrassInstances,
            PlantInstances,
            FlowerInstances,
            ProcessedCells,
            bLowCPUProfile ? TEXT("LowCPU") : TEXT("Full"),
            CandidateTraceAttempts,
            CandidateAccepted,
            CandidateRejectedBlocked,
            CandidateRejectedTrace,
            CandidateRejectedBounds);
    }
}
