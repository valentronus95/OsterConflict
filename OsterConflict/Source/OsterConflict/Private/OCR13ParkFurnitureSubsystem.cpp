#include "OCR13ParkFurnitureSubsystem.h"

#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
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

    UInstancedStaticMeshComponent* MakeISM(AActor* Owner, USceneComponent* Root, UStaticMesh* Mesh,
        const FName Name, const bool bCollision)
    {
        if (!Owner || !Root || !Mesh) return nullptr;

        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, Name);
        if (!Component) return nullptr;

        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionProfileName(FName(bCollision ? TEXT("BlockAll") : TEXT("NoCollision")));
        Component->SetCastShadow(!bCollision);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    bool IsUsablePlank(UStaticMesh* Mesh)
    {
        if (!Mesh) return false;
        const FVector Size = Mesh->GetBounds().BoxExtent * 2.0f;
        const float LongAxis = FMath::Max(Size.X, Size.Y);
        const float ShortAxis = FMath::Max(1.0f, FMath::Min(Size.X, Size.Y));
        return LongAxis >= 70.0f && LongAxis >= ShortAxis * 2.0f && LongAxis >= FMath::Max(1.0f, Size.Z) * 2.0f;
    }

    bool IsBenchProxyTransform(const FTransform& Transform, const FVector& ParkAnchor)
    {
        const FVector Scale = Transform.GetScale3D().GetAbs();
        if (!FMath::IsNearlyEqual(Scale.X, 1.80f, 0.08f) ||
            !FMath::IsNearlyEqual(Scale.Y, 0.55f, 0.08f) ||
            !FMath::IsNearlyEqual(Scale.Z, 1.20f, 0.08f))
        {
            return false;
        }

        const FVector Delta = Transform.GetLocation() - ParkAnchor;
        return Delta.Size2D() <= 8000.0f && FMath::Abs(Delta.Z - 60.0f) <= 25.0f;
    }

    void AddFittedInstance(UInstancedStaticMeshComponent* Target, const FVector& Center,
        const FVector& DesiredLocalSize, const FQuat& Rotation)
    {
        if (!Target || !Target->GetStaticMesh()) return;

        const FBoxSphereBounds Bounds = Target->GetStaticMesh()->GetBounds();
        const FVector MeshSize = Bounds.BoxExtent * 2.0f;
        if (MeshSize.X <= 1.0f || MeshSize.Y <= 1.0f || MeshSize.Z <= 1.0f) return;

        const FVector Scale(
            DesiredLocalSize.X / MeshSize.X,
            DesiredLocalSize.Y / MeshSize.Y,
            DesiredLocalSize.Z / MeshSize.Z);
        const FVector Location = Center - Rotation.RotateVector(Bounds.Origin * Scale);
        Target->AddInstance(FTransform(Rotation, Location, Scale), true);
    }

    void ApplySupportTint(UInstancedStaticMeshComponent* Component, UMaterialInterface* BaseMaterial)
    {
        if (!Component || !BaseMaterial) return;
        UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMaterial, Component);
        if (!MID) return;
        MID->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.10f, 0.11f, 0.10f, 1.0f));
        Component->SetMaterial(0, MID);
    }

    int32 ReplaceBenchProxies(UInstancedStaticMeshComponent* Proxy, const FVector& ParkAnchor,
        const TArray<UInstancedStaticMeshComponent*>& PlankFamilies,
        UInstancedStaticMeshComponent* Supports,
        UInstancedStaticMeshComponent* CollisionProxy)
    {
        if (!Proxy || PlankFamilies.Num() == 0 || !Supports || !CollisionProxy) return 0;

        int32 Replaced = 0;
        for (int32 Index = 0; Index < Proxy->GetInstanceCount(); ++Index)
        {
            FTransform ProxyTransform;
            if (!Proxy->GetInstanceTransform(Index, ProxyTransform, true) ||
                !IsBenchProxyTransform(ProxyTransform, ParkAnchor))
            {
                continue;
            }

            UInstancedStaticMeshComponent* Wood = PlankFamilies[Replaced % PlankFamilies.Num()];
            if (!Wood || !Wood->GetStaticMesh()) continue;

            const float Yaw = ProxyTransform.Rotator().Yaw;
            const FQuat SeatRotation = FRotator(0.0f, Yaw, 0.0f).Quaternion();
            const FQuat BackRotation = FRotator(0.0f, Yaw, 90.0f).Quaternion();
            const FVector Base = ProxyTransform.GetLocation() - FVector(0.0f, 0.0f, 60.0f);
            const FVector Right = SeatRotation.RotateVector(FVector::RightVector);
            const FVector Forward = SeatRotation.RotateVector(FVector::ForwardVector);

            // Preserve one cheap collision body per bench before moving the source cube proxy out of sight.
            CollisionProxy->AddInstance(ProxyTransform, true);

            AddFittedInstance(Wood, Base + FVector(0.0f, 0.0f, 48.0f),
                FVector(180.0f, 42.0f, 7.0f), SeatRotation);
            AddFittedInstance(Wood, Base + Right * 22.0f + FVector(0.0f, 0.0f, 82.0f),
                FVector(180.0f, 45.0f, 7.0f), BackRotation);

            for (const float Along : { -61.0f, 61.0f })
            {
                const FVector LegCenter = Base + Forward * Along + FVector(0.0f, 0.0f, 23.0f);
                AddFittedInstance(Supports, LegCenter, FVector(8.0f, 34.0f, 46.0f), SeatRotation);
            }

            FTransform HiddenTransform = ProxyTransform;
            HiddenTransform.SetLocation(ProxyTransform.GetLocation() + FVector(0.0f, 0.0f, -100000.0f));
            HiddenTransform.SetScale3D(FVector(0.001f));
            Proxy->UpdateInstanceTransform(Index, HiddenTransform, true, true, true);
            ++Replaced;
        }
        return Replaced;
    }
}

bool UOCR13ParkFurnitureSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13ParkFurnitureSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) BuildParkFurnitureBridge(*World);
        }), 1.20f, false);
}

void UOCR13ParkFurnitureSubsystem::BuildParkFurnitureBridge(UWorld& World)
{
    AOCWorldSectorOster* WorldSector = nullptr;
    for (TActorIterator<AOCWorldSectorOster> It(&World); It; ++It)
    {
        WorldSector = *It;
        if (WorldSector) break;
    }
    if (!WorldSector) return;

    UInstancedStaticMeshComponent* ParkDetailsProxy = FindISM(WorldSector, TEXT("ParkDetails"));
    if (!ParkDetailsProxy || ParkDetailsProxy->GetInstanceCount() <= 0) return;

    UStaticMesh* Plank01 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Old_Planks_Plank_1.Old_Planks_Plank_1"));
    UStaticMesh* Plank02 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Old_Planks_Plank_2.Old_Planks_Plank_2"));
    UStaticMesh* Plank03 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Old_Planks_Plank_3.Old_Planks_Plank_3"));
    UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

    if (!IsUsablePlank(Plank01)) Plank01 = nullptr;
    if (!IsUsablePlank(Plank02)) Plank02 = nullptr;
    if (!IsUsablePlank(Plank03)) Plank03 = nullptr;
    if ((!Plank01 && !Plank02 && !Plank03) || !CubeMesh)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("R13 park furniture: bundled old-plank art unavailable/unsuitable; preserving bench proxies."));
        return;
    }

    AActor* ArtRoot = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!ArtRoot) return;
    ArtRoot->SetReplicates(false);

    USceneComponent* Root = NewObject<USceneComponent>(ArtRoot, TEXT("R13_ParkFurnitureRoot"));
    if (!Root) return;
    ArtRoot->SetRootComponent(Root);
    ArtRoot->AddInstanceComponent(Root);
    Root->RegisterComponent();

    TArray<UInstancedStaticMeshComponent*> PlankFamilies;
    if (UInstancedStaticMeshComponent* Family = MakeISM(ArtRoot, Root, Plank01, TEXT("R13_ParkBenchWood01"), false)) PlankFamilies.Add(Family);
    if (UInstancedStaticMeshComponent* Family = MakeISM(ArtRoot, Root, Plank02, TEXT("R13_ParkBenchWood02"), false)) PlankFamilies.Add(Family);
    if (UInstancedStaticMeshComponent* Family = MakeISM(ArtRoot, Root, Plank03, TEXT("R13_ParkBenchWood03"), false)) PlankFamilies.Add(Family);

    UInstancedStaticMeshComponent* Supports = MakeISM(ArtRoot, Root, CubeMesh, TEXT("R13_ParkBenchSupports"), false);
    UInstancedStaticMeshComponent* BenchCollision = MakeISM(ArtRoot, Root, CubeMesh, TEXT("R13_ParkBenchCollision"), true);
    if (BenchCollision) BenchCollision->SetVisibility(false, true);
    ApplySupportTint(Supports, BaseMaterial);

    const int32 Replaced = ReplaceBenchProxies(ParkDetailsProxy, AOCWorldSectorOster::ParkAnchor(),
        PlankFamilies, Supports, BenchCollision);

    UE_LOG(LogTemp, Display,
        TEXT("R13 park furniture: replaced %d semantic central-park bench proxies with bundled old-plank art; unrelated ParkDetails untouched."),
        Replaced);
}
