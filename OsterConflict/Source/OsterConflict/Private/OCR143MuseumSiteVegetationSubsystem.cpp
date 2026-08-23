#include "OCR143MuseumSiteVegetationSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    constexpr float R143SiteVegetationDelaySeconds = 6.18f;

    UInstancedStaticMeshComponent* MakeISM(AActor* Owner, USceneComponent* Root, UStaticMesh* Mesh,
        const TCHAR* Name, const int32 CullEndCm)
    {
        if (!Owner || !Root || !Mesh) return nullptr;
        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(
            Owner, MakeUniqueObjectName(Owner, UInstancedStaticMeshComponent::StaticClass(), FName(Name)));
        if (!Component) return nullptr;
        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(false);
        Component->SetCastShadow(false);
        Component->SetCullDistances(0, CullEndCm);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    void AddGrounded(UInstancedStaticMeshComponent* Component, UStaticMesh* Mesh,
        const FVector& Ground, const float DesiredHeightCm, const float Yaw, const float WidthScale = 1.0f)
    {
        if (!Component || !Mesh) return;
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        if (NativeSize.Z <= 1.0f) return;

        const float Uniform = DesiredHeightCm / NativeSize.Z;
        const FVector Scale(Uniform * WidthScale, Uniform * WidthScale, Uniform);
        const float LocalBottom = Bounds.Origin.Z - Bounds.BoxExtent.Z;
        FVector Location = Ground;
        Location.Z = -LocalBottom * Uniform;
        Component->AddInstance(FTransform(FRotator(0.0f, Yaw, 0.0f), Location, Scale), true);
    }

    bool IsClearApproach(const FVector2D& Offset)
    {
        // REF-04/05/07/10/14: central concrete approach stays visually open.
        if (Offset.Y < -700.0f && FMath::Abs(Offset.X) < 250.0f) return true;
        // Keep the stairs/vestibule and the immediate wall perimeter free of random plants.
        if (Offset.Y < 650.0f && Offset.Y > -950.0f && FMath::Abs(Offset.X) < 1250.0f) return true;
        return false;
    }
}

bool UOCR143MuseumSiteVegetationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR143MuseumSiteVegetationSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) BuildSiteVegetation(*World);
        }), R143SiteVegetationDelaySeconds, false);
}

void UOCR143MuseumSiteVegetationSubsystem::BuildSiteVegetation(UWorld& World) const
{
    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        if (AActor* Actor = *It; Actor && Actor->ActorHasTag(TEXT("R143_MuseumSiteVegetation"))) return;
    }

    UStaticMesh* GrassA = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/PN_FoliageCollection/Meshes/grassMesh/grass_01_03_mesh.grass_01_03_mesh"));
    UStaticMesh* GrassB = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/PN_FoliageCollection/Meshes/grassMesh/grass_02_01_mesh.grass_02_01_mesh"));
    UStaticMesh* GroundA = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/PN_FoliageCollection/Meshes/groundPlantMesh/ground_01_02.ground_01_02"));
    UStaticMesh* GroundB = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/PN_FoliageCollection/Meshes/groundPlantMesh/ground_02_03.ground_02_03"));
    if (!GrassA && !GrassB && !GroundA && !GroundB) return;

    AActor* SiteActor = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!SiteActor) return;
    SiteActor->SetReplicates(false);
    SiteActor->Tags.Add(TEXT("R143_MuseumSiteVegetation"));

    USceneComponent* Root = NewObject<USceneComponent>(SiteActor,
        MakeUniqueObjectName(SiteActor, USceneComponent::StaticClass(), FName(TEXT("R143MuseumVegetationRoot"))));
    if (!Root)
    {
        SiteActor->Destroy();
        return;
    }
    SiteActor->SetRootComponent(Root);
    SiteActor->AddInstanceComponent(Root);
    Root->RegisterComponent();

    UInstancedStaticMeshComponent* GrassAISM = MakeISM(SiteActor, Root, GrassA,
        TEXT("R143Museum_GrassA"), 15000);
    UInstancedStaticMeshComponent* GrassBISM = MakeISM(SiteActor, Root, GrassB,
        TEXT("R143Museum_GrassB"), 15000);
    UInstancedStaticMeshComponent* GroundAISM = MakeISM(SiteActor, Root, GroundA,
        TEXT("R143Museum_GroundPlantA"), 12000);
    UInstancedStaticMeshComponent* GroundBISM = MakeISM(SiteActor, Root, GroundB,
        TEXT("R143Museum_GroundPlantB"), 12000);

    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();
    FRandomStream Random(14032026);
    int32 GrassCount = 0;
    int32 PlantCount = 0;

    // Open front lawn around the slab approach. Density is intentionally low because the references show mown grass,
    // not waist-high procedural foliage.
    for (int32 Index = 0; Index < 110; ++Index)
    {
        const FVector2D Offset(Random.FRandRange(-1900.0f, 1900.0f), Random.FRandRange(-4550.0f, -850.0f));
        if (IsClearApproach(Offset)) continue;

        UStaticMesh* Mesh = (Index % 2 == 0) ? GrassA : GrassB;
        UInstancedStaticMeshComponent* ISM = (Index % 2 == 0) ? GrassAISM : GrassBISM;
        if (!Mesh || !ISM) continue;
        AddGrounded(ISM, Mesh, Museum + FVector(Offset.X, Offset.Y, 0.0f),
            Random.FRandRange(22.0f, 46.0f), Random.FRandRange(0.0f, 360.0f), Random.FRandRange(0.8f, 1.25f));
        ++GrassCount;
    }

    // Side/rear low plants and the less manicured patches visible in REF-13.
    for (int32 Index = 0; Index < 48; ++Index)
    {
        FVector2D Offset;
        if (Index < 28)
        {
            Offset = FVector2D(Random.FRandRange(-1750.0f, 1750.0f), Random.FRandRange(650.0f, 1800.0f));
        }
        else
        {
            const float Side = (Index % 2 == 0) ? -1.0f : 1.0f;
            Offset = FVector2D(Side * Random.FRandRange(1250.0f, 2050.0f), Random.FRandRange(-700.0f, 900.0f));
        }
        if (IsClearApproach(Offset)) continue;

        UStaticMesh* Mesh = (Index % 2 == 0) ? GroundA : GroundB;
        UInstancedStaticMeshComponent* ISM = (Index % 2 == 0) ? GroundAISM : GroundBISM;
        if (!Mesh || !ISM) continue;
        AddGrounded(ISM, Mesh, Museum + FVector(Offset.X, Offset.Y, 0.0f),
            Random.FRandRange(28.0f, 62.0f), Random.FRandRange(0.0f, 360.0f), Random.FRandRange(0.75f, 1.2f));
        ++PlantCount;
    }

    // Small irregular bed near the rear/side annex, based on REF-13 rather than a symmetric landscaping pattern.
    for (int32 Index = 0; Index < 14; ++Index)
    {
        UStaticMesh* Mesh = (Index % 2 == 0) ? GroundA : GroundB;
        UInstancedStaticMeshComponent* ISM = (Index % 2 == 0) ? GroundAISM : GroundBISM;
        if (!Mesh || !ISM) continue;
        const FVector Offset(920.0f + Random.FRandRange(-230.0f, 260.0f),
            760.0f + Random.FRandRange(-240.0f, 260.0f), 0.0f);
        AddGrounded(ISM, Mesh, Museum + Offset, Random.FRandRange(34.0f, 68.0f),
            Random.FRandRange(0.0f, 360.0f), Random.FRandRange(0.8f, 1.25f));
        ++PlantCount;
    }

    UE_LOG(LogTemp, Display,
        TEXT("R14.3 museum site vegetation: low photo-oriented grass=%d groundPlants=%d; central slab approach kept clear."),
        GrassCount, PlantCount);
}
