#include "OCR13LandmarkRoofArtSubsystem.h"

#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"

namespace
{
    constexpr int32 ExpectedMuseumRoofPanelCount = 8;

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

    bool IsMuseumPitchedRoofPanel(const FTransform& Transform, const FVector& MuseumAnchor)
    {
        const FRotator Rotation = Transform.Rotator();
        const float Slope = FMath::Max(FMath::Abs(Rotation.Roll), FMath::Abs(Rotation.Pitch));
        const FVector Delta = Transform.GetLocation() - MuseumAnchor;
        const FVector Scale = Transform.GetScale3D().GetAbs();

        // Museum AddGableRoof panels are thin source-cube slabs with 24-30 degree slope and sit close to
        // MuseumAnchor. College roofs are flat and must never be captured by this bridge.
        return Delta.Size2D() <= 6000.0f &&
            Slope >= 18.0f && Slope <= 36.0f &&
            Scale.Z <= 0.40f && FMath::Max(Scale.X, Scale.Y) >= 5.0f;
    }
}

bool UOCR13LandmarkRoofArtSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13LandmarkRoofArtSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) BuildMuseumRoofBridge(*World);
        }), 1.30f, false);
}

void UOCR13LandmarkRoofArtSubsystem::BuildMuseumRoofBridge(UWorld& World)
{
    AOCWorldSectorOster* WorldSector = nullptr;
    for (TActorIterator<AOCWorldSectorOster> It(&World); It; ++It)
    {
        WorldSector = *It;
        if (WorldSector) break;
    }
    if (!WorldSector) return;

    UInstancedStaticMeshComponent* Proxy = FindISM(WorldSector, TEXT("LandmarkRoofs"));
    if (!Proxy || Proxy->GetInstanceCount() <= 0) return;

    const FVector MuseumAnchor = AOCWorldSectorOster::MuseumAnchor();
    TArray<int32> MuseumRoofIndices;
    TArray<FTransform> MuseumRoofTransforms;
    for (int32 Index = 0; Index < Proxy->GetInstanceCount(); ++Index)
    {
        FTransform Transform;
        if (!Proxy->GetInstanceTransform(Index, Transform, true)) continue;
        if (!IsMuseumPitchedRoofPanel(Transform, MuseumAnchor)) continue;
        MuseumRoofIndices.Add(Index);
        MuseumRoofTransforms.Add(Transform);
    }

    // The source currently defines four gable groups, two panels each. Treat any count drift as a topology
    // change and leave all roofs alone rather than guessing which future landmark panels belong to the museum.
    if (MuseumRoofTransforms.Num() != ExpectedMuseumRoofPanelCount)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("R13 museum roof: expected %d pitched source panels, found %d; preserving all LandmarkRoofs proxies."),
            ExpectedMuseumRoofPanelCount, MuseumRoofTransforms.Num());
        return;
    }

    UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UMaterialInterface* RoofMaterial = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Materials/Instances/Metal_Roof.Metal_Roof"));
    if (!CubeMesh || !RoofMaterial)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("R13 museum roof: bundled Metal_Roof material unavailable; preserving source roof proxies."));
        return;
    }

    AActor* ArtRoot = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!ArtRoot) return;
    ArtRoot->SetReplicates(false);

    USceneComponent* Root = NewObject<USceneComponent>(ArtRoot, TEXT("R13_MuseumRoofArtRoot"));
    if (!Root)
    {
        ArtRoot->Destroy();
        return;
    }
    ArtRoot->SetRootComponent(Root);
    ArtRoot->AddInstanceComponent(Root);
    Root->RegisterComponent();

    UInstancedStaticMeshComponent* RoofArt = NewObject<UInstancedStaticMeshComponent>(ArtRoot, TEXT("R13_MuseumMetalRoof"));
    if (!RoofArt)
    {
        ArtRoot->Destroy();
        return;
    }
    RoofArt->SetupAttachment(Root);
    RoofArt->SetStaticMesh(CubeMesh);
    RoofArt->SetMaterial(0, RoofMaterial);
    RoofArt->SetMobility(EComponentMobility::Static);
    RoofArt->SetCollisionProfileName(TEXT("BlockAll"));
    RoofArt->SetCastShadow(true);
    ArtRoot->AddInstanceComponent(RoofArt);
    RoofArt->RegisterComponent();

    for (const FTransform& Transform : MuseumRoofTransforms)
    {
        RoofArt->AddInstance(Transform, true);
    }
    if (RoofArt->GetInstanceCount() != ExpectedMuseumRoofPanelCount)
    {
        ArtRoot->Destroy();
        return;
    }

    // Suppress only the eight matched source instances. Keeping them in-place at negligible scale avoids
    // disturbing the flat college roof instances that share this ISM and avoids expanding bounds below the map.
    for (const int32 Index : MuseumRoofIndices)
    {
        FTransform HiddenTransform;
        if (!Proxy->GetInstanceTransform(Index, HiddenTransform, true))
        {
            ArtRoot->Destroy();
            return;
        }
        HiddenTransform.SetScale3D(FVector(0.001f));
        Proxy->UpdateInstanceTransform(Index, HiddenTransform, true, true, true);
    }

    UE_LOG(LogTemp, Display,
        TEXT("R13 museum roof: applied bundled Metal_Roof material to %d pitched museum panels; flat college LandmarkRoofs untouched."),
        ExpectedMuseumRoofPanelCount);
}
