#include "OCR13UtilityPoleSubsystem.h"

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

    UInstancedStaticMeshComponent* MakeISM(AActor* Owner, USceneComponent* Root, UStaticMesh* Mesh, const FName Name)
    {
        if (!Owner || !Root || !Mesh) return nullptr;
        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, Name);
        if (!Component) return nullptr;
        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionProfileName(TEXT("NoCollision"));
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetCastShadow(true);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    void AddGroundedPole(UInstancedStaticMeshComponent* Component, UStaticMesh* Mesh,
        FVector Location, const float DesiredHeightCm, const float Yaw)
    {
        if (!Component || !Mesh) return;
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector Size = Bounds.BoxExtent * 2.0f;
        if (Size.Z <= 10.0f) return;
        const float Scale = FMath::Clamp(DesiredHeightCm / Size.Z, 0.35f, 3.0f);
        const float LocalBottom = Bounds.Origin.Z - Bounds.BoxExtent.Z;
        Location.Z = -LocalBottom * Scale;
        Component->AddInstance(FTransform(FRotator(0.0f, Yaw, 0.0f), Location, FVector(Scale)), true);
    }

    bool IsInsideCompactBounds(const FVector& P)
    {
        return P.X >= -69000.0f && P.X <= 24000.0f && P.Y >= -24000.0f && P.Y <= 49000.0f;
    }
}

bool UOCR13UtilityPoleSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13UtilityPoleSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) BuildUtilityPoles(*World);
        }), 1.95f, false);
}

void UOCR13UtilityPoleSubsystem::BuildUtilityPoles(UWorld& World)
{
    AOCWorldSectorOster* WorldSector = nullptr;
    for (TActorIterator<AOCWorldSectorOster> It(&World); It; ++It)
    {
        WorldSector = *It;
        if (WorldSector) break;
    }
    if (!WorldSector) return;

    UInstancedStaticMeshComponent* Roads = FindISM(WorldSector, TEXT("Roads"));
    if (!Roads || Roads->GetInstanceCount() == 0) return;

    UStaticMesh* PoleMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Power_Pole_1.Power_Pole_1"));
    if (!PoleMesh) return;

    AActor* ArtRoot = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!ArtRoot) return;
    ArtRoot->SetReplicates(false);
    ArtRoot->SetActorEnableCollision(false);

    USceneComponent* Root = NewObject<USceneComponent>(ArtRoot, TEXT("R13_UtilityPoleRoot"));
    if (!Root)
    {
        ArtRoot->Destroy();
        return;
    }
    Root->SetMobility(EComponentMobility::Static);
    ArtRoot->SetRootComponent(Root);
    ArtRoot->AddInstanceComponent(Root);
    Root->RegisterComponent();

    UInstancedStaticMeshComponent* Poles = MakeISM(ArtRoot, Root, PoleMesh, TEXT("R13_UtilityPoles"));
    if (!Poles)
    {
        ArtRoot->Destroy();
        return;
    }

    int32 PoleCount = 0;
    for (int32 Index = 0; Index < Roads->GetInstanceCount(); ++Index)
    {
        // Road proxies include long connected pieces. Sampling one in three keeps poles sparse and avoids a picket fence.
        if ((Index % 3) != 0) continue;

        FTransform RoadTransform;
        if (!Roads->GetInstanceTransform(Index, RoadTransform, true)) continue;

        const FVector RoadScale = RoadTransform.GetScale3D().GetAbs();
        const float RoadWidth = FMath::Max(350.0f, FMath::Min(RoadScale.X, RoadScale.Y) * 100.0f);
        const float SideOffset = FMath::Clamp(RoadWidth * 0.5f + 260.0f, 520.0f, 1250.0f);
        const float Yaw = RoadTransform.Rotator().Yaw;
        const FVector LocalSide(0.0f, ((Index / 3) % 2 == 0 ? SideOffset : -SideOffset), 0.0f);
        const FVector Location = RoadTransform.GetLocation() +
            FQuat(FRotator(0.0f, Yaw, 0.0f)).RotateVector(LocalSide);

        if (!IsInsideCompactBounds(Location)) continue;
        if (Location.Size2D() < 4800.0f) continue; // leave the museum front garden to its photo-reference composition

        AddGroundedPole(Poles, PoleMesh, Location,
            760.0f + 25.0f * static_cast<float>(Index % 5), Yaw + 90.0f);
        ++PoleCount;
    }

    // Two explicit museum-area poles are visible in supplied reference angles, but keep them outside the entrance axis.
    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();
    AddGroundedPole(Poles, PoleMesh, Museum + FVector(-5200.0f, 2300.0f, 0.0f), 790.0f, 18.0f);
    AddGroundedPole(Poles, PoleMesh, Museum + FVector(5600.0f, 3300.0f, 0.0f), 805.0f, 194.0f);
    PoleCount += 2;

    UE_LOG(LogTemp, Display,
        TEXT("R13.4 utility poles: %d non-blocking poles placed from authored road topology + museum reference accents."),
        PoleCount);
}
