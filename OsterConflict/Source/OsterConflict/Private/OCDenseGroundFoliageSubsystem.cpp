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
        Component->SetCullDistances(450, CullEndCm);
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

    // Frontend and gameplay share the same runtime world. Returning permanently while the menu is
    // active meant this subsystem never got a second OnWorldBeginPlay after START, so the player saw
    // a bare green plane forever. Poll until the frontend flag drops, then build foliage once.
    InWorld.GetTimerManager().SetTimer(
        GameplayReadyTimer,
        this,
        &UOCDenseGroundFoliageSubsystem::TryPopulateWhenGameplayReady,
        0.35f,
        true,
        0.0f);
}

void UOCDenseGroundFoliageSubsystem::TryPopulateWhenGameplayReady()
{
    UWorld* World = GetWorld();
    if (!World || bPopulated) return;

    if (const AOCGameMode* GameMode = World->GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    World->GetTimerManager().ClearTimer(GameplayReadyTimer);
    Populate(*World);
}

void UOCDenseGroundFoliageSubsystem::Populate(UWorld& World)
{
    if (bPopulated) return;
    bPopulated = true;

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
        UE_LOG(LogTemp, Error, TEXT("Dense foliage unavailable: no real grass mesh was loadable from PN or AdvancedVillagePack."));
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

    UHierarchicalInstancedStaticMeshComponent* GrassComponents[UE_ARRAY_COUNT(GrassCandidates)] = {};
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(GrassCandidates); ++Index)
    {
        GrassComponents[Index] = MakeFoliageHISM(
            FoliageActor, Root, GrassMeshes[Index], FName(*FString::Printf(TEXT("DenseGrass_%d"), Index)), 36000);
    }
    UHierarchicalInstancedStaticMeshComponent* GroundPlants = MakeFoliageHISM(
        FoliageActor, Root, GroundPlantMesh, TEXT("DenseGroundPlants"), 30000);
    UHierarchicalInstancedStaticMeshComponent* Flowers = MakeFoliageHISM(
        FoliageActor, Root, FlowerMesh, TEXT("DenseFlowers"), 26000);

    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(OCDenseGroundFoliage), false);
    QueryParams.AddIgnoredActor(FoliageActor);

    FRandomStream Random(20260822);
    int32 GrassInstances = 0;
    int32 PlantInstances = 0;
    int32 FlowerInstances = 0;

    // Runtime playtest showed the old 13.5 m lattice still reading as isolated tufts. Use a denser
    // 10 m ground-traced lattice and 3-5 clumps per valid cell. HISM keeps this as instanced foliage,
    // while the hard-surface filter prevents grass from being sprayed across roads and buildings.
    constexpr float SectorMin = -96000.0f;
    constexpr float SectorMax = 96000.0f;
    constexpr float GridStep = 1000.0f;

    for (float X = SectorMin; X <= SectorMax; X += GridStep)
    {
        for (float Y = SectorMin; Y <= SectorMax; Y += GridStep)
        {
            const FVector2D Jitter(Random.FRandRange(-320.0f, 320.0f), Random.FRandRange(-320.0f, 320.0f));
            const FVector TraceStart(X + Jitter.X, Y + Jitter.Y, 18000.0f);
            const FVector TraceEnd(X + Jitter.X, Y + Jitter.Y, -3000.0f);

            FHitResult Hit;
            if (!World.LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QueryParams)) continue;
            if (!Hit.bBlockingHit || IsBlockedSurface(Hit)) continue;
            if (Hit.ImpactNormal.Z < 0.72f) continue;

            const FVector BaseLocation = Hit.ImpactPoint + Hit.ImpactNormal * 2.0f;
            const int32 ClumpCount = Random.RandRange(3, 5);
            for (int32 ClumpIndex = 0; ClumpIndex < ClumpCount; ++ClumpIndex)
            {
                const int32 Variant = Random.RandRange(0, UE_ARRAY_COUNT(GrassCandidates) - 1);
                UHierarchicalInstancedStaticMeshComponent* Grass = GrassComponents[Variant];
                if (!Grass) continue;

                const FVector Offset(
                    Random.FRandRange(-360.0f, 360.0f),
                    Random.FRandRange(-360.0f, 360.0f),
                    Random.FRandRange(-0.8f, 1.8f));
                const float Yaw = Random.FRandRange(0.0f, 360.0f);
                const float Scale = Random.FRandRange(0.74f, 1.14f);
                Grass->AddInstance(FTransform(
                    FRotator(0.0f, Yaw, 0.0f),
                    BaseLocation + Offset,
                    FVector(Scale)), true);
                ++GrassInstances;
            }

            if (GroundPlants && Random.FRand() < 0.19f)
            {
                GroundPlants->AddInstance(FTransform(
                    FRotator(0.0f, Random.FRandRange(0.0f, 360.0f), 0.0f),
                    BaseLocation + FVector(Random.FRandRange(-340.0f, 340.0f), Random.FRandRange(-340.0f, 340.0f), 1.0f),
                    FVector(Random.FRandRange(0.70f, 1.02f))), true);
                ++PlantInstances;
            }

            if (Flowers && Random.FRand() < 0.05f)
            {
                Flowers->AddInstance(FTransform(
                    FRotator(0.0f, Random.FRandRange(0.0f, 360.0f), 0.0f),
                    BaseLocation + FVector(Random.FRandRange(-320.0f, 320.0f), Random.FRandRange(-320.0f, 320.0f), 1.0f),
                    FVector(Random.FRandRange(0.64f, 0.90f))), true);
                ++FlowerInstances;
            }
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("Dense Oster foliage populated as sole ground-cover owner: %d grass, %d ground plants, %d flowers."),
        GrassInstances, PlantInstances, FlowerInstances);
}
