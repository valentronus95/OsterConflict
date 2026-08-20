#include "OCDenseGroundFoliageSubsystem.h"

#include "OCGameMode.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

namespace
{
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
        Component->SetCullDistances(900, CullEndCm);
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

    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    InWorld.GetTimerManager().SetTimerForNextTick(
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) Populate(*World);
        }));
}

void UOCDenseGroundFoliageSubsystem::Populate(UWorld& World)
{
    if (bPopulated) return;
    bPopulated = true;

    const TCHAR* GrassPaths[] =
    {
        TEXT("/Game/PN_FoliageCollection/Meshes/grassMesh/grass_01_01_mesh.grass_01_01_mesh"),
        TEXT("/Game/PN_FoliageCollection/Meshes/grassMesh/grass_01_02_mesh.grass_01_02_mesh"),
        TEXT("/Game/PN_FoliageCollection/Meshes/grassMesh/grass_01_04_mesh.grass_01_04_mesh"),
        TEXT("/Game/PN_FoliageCollection/Meshes/grassMesh/grass_01_07_mesh.grass_01_07_mesh")
    };

    UStaticMesh* GrassMeshes[UE_ARRAY_COUNT(GrassPaths)] = {};
    bool bAnyGrass = false;
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(GrassPaths); ++Index)
    {
        GrassMeshes[Index] = LoadObject<UStaticMesh>(nullptr, GrassPaths[Index]);
        bAnyGrass |= GrassMeshes[Index] != nullptr;
    }

    UStaticMesh* GroundPlantMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/PN_FoliageCollection/Meshes/groundPlantMesh/ground_01_01.ground_01_01"));
    UStaticMesh* FlowerMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/PN_FoliageCollection/Meshes/flowerMesh/flower_01_01.flower_01_01"));

    if (!bAnyGrass)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dense foliage skipped: PN grass meshes were not loadable."));
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = TEXT("OC_DenseGroundFoliage");
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AActor* FoliageActor = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, SpawnParams);
    if (!FoliageActor) return;
    FoliageActor->Tags.Add(TEXT("OC_DenseGroundFoliage"));

    USceneComponent* Root = NewObject<USceneComponent>(FoliageActor, TEXT("DenseFoliageRoot"));
    FoliageActor->AddInstanceComponent(Root);
    Root->RegisterComponent();
    FoliageActor->SetRootComponent(Root);

    UHierarchicalInstancedStaticMeshComponent* GrassComponents[UE_ARRAY_COUNT(GrassPaths)] = {};
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(GrassPaths); ++Index)
    {
        GrassComponents[Index] = MakeFoliageHISM(
            FoliageActor, Root, GrassMeshes[Index], FName(*FString::Printf(TEXT("DenseGrass_%d"), Index)), 28000);
    }
    UHierarchicalInstancedStaticMeshComponent* GroundPlants = MakeFoliageHISM(
        FoliageActor, Root, GroundPlantMesh, TEXT("DenseGroundPlants"), 25000);
    UHierarchicalInstancedStaticMeshComponent* Flowers = MakeFoliageHISM(
        FoliageActor, Root, FlowerMesh, TEXT("DenseFlowers"), 22000);

    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(OCDenseGroundFoliage), false);
    QueryParams.AddIgnoredActor(FoliageActor);

    FRandomStream Random(20260820);
    int32 GrassInstances = 0;
    int32 PlantInstances = 0;
    int32 FlowerInstances = 0;

    // Cover the playable sector instead of decorating only one park. Ground traces keep foliage
    // on actual collision surfaces, while component/tag filtering rejects roads, buildings,
    // pavement, bridges and other hard surfaces.
    constexpr float SectorMin = -96000.0f;
    constexpr float SectorMax = 96000.0f;
    constexpr float GridStep = 2200.0f;

    for (float X = SectorMin; X <= SectorMax; X += GridStep)
    {
        for (float Y = SectorMin; Y <= SectorMax; Y += GridStep)
        {
            const FVector2D Jitter(Random.FRandRange(-720.0f, 720.0f), Random.FRandRange(-720.0f, 720.0f));
            const FVector TraceStart(X + Jitter.X, Y + Jitter.Y, 18000.0f);
            const FVector TraceEnd(X + Jitter.X, Y + Jitter.Y, -3000.0f);

            FHitResult Hit;
            if (!World.LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QueryParams)) continue;
            if (!Hit.bBlockingHit || IsBlockedSurface(Hit)) continue;
            if (Hit.ImpactNormal.Z < 0.72f) continue;

            const int32 Variant = Random.RandRange(0, UE_ARRAY_COUNT(GrassPaths) - 1);
            UHierarchicalInstancedStaticMeshComponent* Grass = GrassComponents[Variant];
            if (!Grass) continue;

            const FVector BaseLocation = Hit.ImpactPoint + Hit.ImpactNormal * 2.0f;
            const float Yaw = Random.FRandRange(0.0f, 360.0f);
            const float Scale = Random.FRandRange(0.72f, 1.08f);
            Grass->AddInstance(FTransform(FRotator(0.0f, Yaw, 0.0f), BaseLocation, FVector(Scale)), true);
            ++GrassInstances;

            // Most valid cells receive a second clump so parks, yards and verges read as ground
            // cover rather than isolated decorative tufts.
            if (Random.FRand() < 0.68f)
            {
                const FVector Offset(Random.FRandRange(-620.0f, 620.0f), Random.FRandRange(-620.0f, 620.0f), 0.0f);
                Grass->AddInstance(FTransform(
                    FRotator(0.0f, Random.FRandRange(0.0f, 360.0f), 0.0f),
                    BaseLocation + Offset,
                    FVector(Random.FRandRange(0.68f, 1.02f))), true);
                ++GrassInstances;
            }

            if (GroundPlants && Random.FRand() < 0.13f)
            {
                GroundPlants->AddInstance(FTransform(
                    FRotator(0.0f, Random.FRandRange(0.0f, 360.0f), 0.0f),
                    BaseLocation + FVector(Random.FRandRange(-500.0f, 500.0f), Random.FRandRange(-500.0f, 500.0f), 1.0f),
                    FVector(Random.FRandRange(0.70f, 1.00f))), true);
                ++PlantInstances;
            }

            if (Flowers && Random.FRand() < 0.055f)
            {
                Flowers->AddInstance(FTransform(
                    FRotator(0.0f, Random.FRandRange(0.0f, 360.0f), 0.0f),
                    BaseLocation + FVector(Random.FRandRange(-420.0f, 420.0f), Random.FRandRange(-420.0f, 420.0f), 1.0f),
                    FVector(Random.FRandRange(0.65f, 0.90f))), true);
                ++FlowerInstances;
            }
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("Dense Oster foliage populated: %d grass, %d ground plants, %d flowers."),
        GrassInstances, PlantInstances, FlowerInstances);
}
