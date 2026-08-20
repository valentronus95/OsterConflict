#include "OCR13MuseumChimneyArtSubsystem.h"

#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

namespace
{
    constexpr int32 ExpectedMuseumChimneyCount = 2;

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

    bool IsMuseumChimneyProxy(const FTransform& Transform, const FVector& MuseumAnchor)
    {
        const FVector SizeCm = Transform.GetScale3D().GetAbs() * 100.0f;
        const FVector Delta = Transform.GetLocation() - MuseumAnchor;
        return Delta.Size2D() <= 3500.0f &&
            SizeCm.X >= 160.0f && SizeCm.X <= 220.0f &&
            SizeCm.Y >= 160.0f && SizeCm.Y <= 220.0f &&
            SizeCm.Z >= 420.0f && SizeCm.Z <= 560.0f;
    }

    bool IsUsableChimneyMesh(UStaticMesh* Mesh)
    {
        if (!Mesh) return false;
        const FVector Size = Mesh->GetBounds().BoxExtent * 2.0f;
        return Size.X > 10.0f && Size.Y > 10.0f && Size.Z > 50.0f &&
            Size.Z >= FMath::Max(Size.X, Size.Y) * 1.15f;
    }
}

bool UOCR13MuseumChimneyArtSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13MuseumChimneyArtSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) BuildMuseumChimneyBridge(*World);
        }), 1.34f, false);
}

void UOCR13MuseumChimneyArtSubsystem::BuildMuseumChimneyBridge(UWorld& World)
{
    AOCWorldSectorOster* WorldSector = nullptr;
    for (TActorIterator<AOCWorldSectorOster> It(&World); It; ++It)
    {
        WorldSector = *It;
        if (WorldSector) break;
    }
    if (!WorldSector) return;

    UInstancedStaticMeshComponent* Proxy = FindISM(WorldSector, TEXT("LandmarkDetails"));
    if (!Proxy || Proxy->GetInstanceCount() <= 0) return;

    const FVector MuseumAnchor = AOCWorldSectorOster::MuseumAnchor();
    TArray<int32> ChimneyIndices;
    TArray<FTransform> ChimneyTransforms;
    for (int32 Index = 0; Index < Proxy->GetInstanceCount(); ++Index)
    {
        FTransform Transform;
        if (!Proxy->GetInstanceTransform(Index, Transform, true)) continue;
        if (!IsMuseumChimneyProxy(Transform, MuseumAnchor)) continue;
        ChimneyIndices.Add(Index);
        ChimneyTransforms.Add(Transform);
    }

    if (ChimneyTransforms.Num() != ExpectedMuseumChimneyCount)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("R13 museum chimneys: expected %d source proxies, found %d; preserving LandmarkDetails."),
            ExpectedMuseumChimneyCount, ChimneyTransforms.Num());
        return;
    }

    UStaticMesh* ChimneyMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Stone_Chimney.Stone_Chimney"));
    if (!IsUsableChimneyMesh(ChimneyMesh))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("R13 museum chimneys: bundled Stone_Chimney unavailable/unsuitable; preserving source proxies."));
        return;
    }

    AActor* ArtRoot = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!ArtRoot) return;
    ArtRoot->SetReplicates(false);

    USceneComponent* Root = NewObject<USceneComponent>(ArtRoot, TEXT("R13_MuseumChimneyArtRoot"));
    if (!Root)
    {
        ArtRoot->Destroy();
        return;
    }
    Root->SetMobility(EComponentMobility::Static);
    ArtRoot->SetRootComponent(Root);
    ArtRoot->AddInstanceComponent(Root);
    Root->RegisterComponent();

    UInstancedStaticMeshComponent* ChimneyArt = NewObject<UInstancedStaticMeshComponent>(ArtRoot,
        TEXT("R13_MuseumStoneChimneys"));
    if (!ChimneyArt)
    {
        ArtRoot->Destroy();
        return;
    }
    ChimneyArt->SetupAttachment(Root);
    ChimneyArt->SetStaticMesh(ChimneyMesh);
    ChimneyArt->SetMobility(EComponentMobility::Static);
    ChimneyArt->SetCollisionProfileName(TEXT("BlockAll"));
    ChimneyArt->SetCastShadow(true);
    ArtRoot->AddInstanceComponent(ChimneyArt);
    ChimneyArt->RegisterComponent();

    const FBoxSphereBounds Bounds = ChimneyMesh->GetBounds();
    const FVector MeshSize = Bounds.BoxExtent * 2.0f;
    for (const FTransform& SourceTransform : ChimneyTransforms)
    {
        const FVector DesiredSize = SourceTransform.GetScale3D().GetAbs() * 100.0f;
        const FVector Scale(DesiredSize.X / MeshSize.X, DesiredSize.Y / MeshSize.Y, DesiredSize.Z / MeshSize.Z);
        const FQuat Rotation = SourceTransform.GetRotation();
        const FVector Location = SourceTransform.GetLocation() - Rotation.RotateVector(Bounds.Origin * Scale);
        ChimneyArt->AddInstance(FTransform(Rotation, Location, Scale), true);
    }

    if (ChimneyArt->GetInstanceCount() != ExpectedMuseumChimneyCount)
    {
        ArtRoot->Destroy();
        return;
    }

    int32 HiddenCount = 0;
    for (int32 ArrayIndex = 0; ArrayIndex < ChimneyIndices.Num(); ++ArrayIndex)
    {
        FTransform HiddenTransform = ChimneyTransforms[ArrayIndex];
        HiddenTransform.SetScale3D(FVector(0.001f));
        if (!Proxy->UpdateInstanceTransform(ChimneyIndices[ArrayIndex], HiddenTransform, true, true, true))
        {
            for (int32 RestoreIndex = 0; RestoreIndex < HiddenCount; ++RestoreIndex)
            {
                Proxy->UpdateInstanceTransform(ChimneyIndices[RestoreIndex],
                    ChimneyTransforms[RestoreIndex], true, true, true);
            }
            ArtRoot->Destroy();
            UE_LOG(LogTemp, Warning,
                TEXT("R13 museum chimneys: source suppression failed; rolled back %d modified proxies."),
                HiddenCount);
            return;
        }
        ++HiddenCount;
    }

    UE_LOG(LogTemp, Display,
        TEXT("R13 museum chimneys: replaced %d source-authored chimney proxies with bundled Stone_Chimney art; other LandmarkDetails untouched."),
        ExpectedMuseumChimneyCount);
}
