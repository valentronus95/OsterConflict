#include "OCRecoveredFoliageSubsystem.h"

#include "OCGameMode.h"
#include "OCGeoReference.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

namespace
{
    UInstancedStaticMeshComponent* MakeFoliageISM(AActor* Owner, USceneComponent* Root,
        UStaticMesh* Mesh, const FName Name, const int32 CullEndCm)
    {
        if (!Owner || !Root || !Mesh) return nullptr;

        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, Name);
        if (!Component) return nullptr;

        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionProfileName(TEXT("NoCollision"));
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(false);
        Component->SetCastShadow(false);
        Component->SetCullDistances(1200, CullEndCm);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    FVector GeoPoint(const FOCGeoReferencePoint& Ref)
    {
        return FOCGeoReference::ToLocalCm(Ref.Latitude, Ref.Longitude, 0.0f);
    }

    void AddInstance(UInstancedStaticMeshComponent* Component, const FVector& Location,
        const float YawDegrees, const float UniformScale)
    {
        if (!Component || !Component->GetStaticMesh()) return;
        Component->AddInstance(FTransform(FRotator(0.0f, YawDegrees, 0.0f),
            Location, FVector(UniformScale)), true);
    }
}

bool UOCRecoveredFoliageSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCRecoveredFoliageSubsystem::OnWorldBeginPlay(UWorld& InWorld)
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

void UOCRecoveredFoliageSubsystem::Populate(UWorld& World)
{
    if (bPopulated) return;
    bPopulated = true;

    UStaticMesh* GrassMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/PN_FoliageCollection/Meshes/grassMesh/grass_01_01_mesh.grass_01_01_mesh"));
    UStaticMesh* FlowerMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/PN_FoliageCollection/Meshes/flowerMesh/flower_01_01.flower_01_01"));
    UStaticMesh* GroundPlantMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/PN_FoliageCollection/Meshes/groundPlantMesh/ground_01_01.ground_01_01"));

    if (!GrassMesh && !FlowerMesh && !GroundPlantMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("Recovered foliage pass skipped: PN foliage meshes were not loadable."));
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = TEXT("OC_RecoveredFoliage");
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AActor* FoliageActor = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, SpawnParams);
    if (!FoliageActor) return;
    FoliageActor->Tags.Add(TEXT("OC_RecoveredFoliage"));

    USceneComponent* Root = NewObject<USceneComponent>(FoliageActor, TEXT("RecoveredFoliageRoot"));
    FoliageActor->AddInstanceComponent(Root);
    Root->RegisterComponent();
    FoliageActor->SetRootComponent(Root);

    UInstancedStaticMeshComponent* Grass = MakeFoliageISM(
        FoliageActor, Root, GrassMesh, TEXT("RecoveredGrass"), 32000);
    UInstancedStaticMeshComponent* Flowers = MakeFoliageISM(
        FoliageActor, Root, FlowerMesh, TEXT("RecoveredFlowers"), 24000);
    UInstancedStaticMeshComponent* GroundPlants = MakeFoliageISM(
        FoliageActor, Root, GroundPlantMesh, TEXT("RecoveredGroundPlants"), 28000);

    const FVector Park = GeoPoint(FOCGeoReference::CentralPark());

    // Park ground layer. Keep the central pedestrian opening clear instead of carpet-bombing
    // the whole park with foliage instances.
    int32 Seed = 1;
    for (int32 Row = -5; Row <= 5; ++Row)
    {
        for (int32 Col = -6; Col <= 6; ++Col)
        {
            if (FMath::Abs(Row) <= 1 && FMath::Abs(Col) <= 1) continue;
            if (((Row * 13 + Col * 7) & 3) == 0) continue;

            const float JitterX = static_cast<float>(((Seed * 97) % 700) - 350);
            const float JitterY = static_cast<float>(((Seed * 53) % 620) - 310);
            const FVector Location = Park + FVector(
                static_cast<float>(Col) * 1150.0f + JitterX,
                static_cast<float>(Row) * 1050.0f + JitterY,
                2.0f);
            const float Scale = 0.72f + static_cast<float>(Seed % 5) * 0.08f;
            AddInstance(Grass, Location, static_cast<float>((Seed * 41) % 360), Scale);

            if ((Seed % 4) == 0)
            {
                AddInstance(GroundPlants, Location + FVector(170.0f, -120.0f, 1.0f),
                    static_cast<float>((Seed * 67) % 360), 0.72f + 0.06f * static_cast<float>(Seed % 3));
            }
            if ((Seed % 9) == 0)
            {
                AddInstance(Flowers, Location + FVector(-140.0f, 95.0f, 1.0f),
                    static_cast<float>((Seed * 83) % 360), 0.65f + 0.08f * static_cast<float>(Seed % 2));
            }
            ++Seed;
        }
    }

    // Rural verge clusters near the restored forest-road pieces. These are intentionally sparse
    // and far from the museum replacement footprint.
    const FVector VergeCenters[] =
    {
        FVector(-92000.0f, -69000.0f, 2.0f),
        FVector(-80500.0f, -61000.0f, 2.0f),
        FVector(79000.0f, 61000.0f, 2.0f)
    };

    for (int32 Cluster = 0; Cluster < UE_ARRAY_COUNT(VergeCenters); ++Cluster)
    {
        for (int32 Index = 0; Index < 24; ++Index)
        {
            const float Angle = static_cast<float>((Cluster * 37 + Index * 29) % 360);
            const float Radius = 1900.0f + static_cast<float>((Index * 173) % 2200);
            const FVector Offset = FRotator(0.0f, Angle, 0.0f).RotateVector(FVector(Radius, 0.0f, 0.0f));
            const FVector Location = VergeCenters[Cluster] + Offset;
            AddInstance(Grass, Location, Angle + 17.0f, 0.78f + 0.05f * static_cast<float>(Index % 4));

            if ((Index % 5) == 0)
            {
                AddInstance(GroundPlants, Location + FVector(120.0f, 80.0f, 1.0f),
                    Angle + 71.0f, 0.75f);
            }
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("Recovered PN foliage placed with instancing and short cull distances."));
}
