#include "OCR13MetalFenceBridgeSubsystem.h"

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

    UInstancedStaticMeshComponent* MakeVisualISM(AActor* Owner, USceneComponent* Root,
        UStaticMesh* Mesh, const FName Name)
    {
        if (!Owner || !Root || !Mesh) return nullptr;

        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, Name);
        if (!Component) return nullptr;

        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetCastShadow(true);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    void ApplyMetalTint(UInstancedStaticMeshComponent* Component, UMaterialInterface* BaseMaterial)
    {
        if (!Component || !BaseMaterial) return;
        UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMaterial, Component);
        if (!MID) return;
        MID->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.12f, 0.14f, 0.15f, 1.0f));
        Component->SetMaterial(0, MID);
    }

    bool BuildOpenMetalFence(UInstancedStaticMeshComponent* Proxy,
        UInstancedStaticMeshComponent* Pickets,
        UInstancedStaticMeshComponent* Rails,
        int32& OutAdded)
    {
        OutAdded = 0;
        if (!Proxy || Proxy->GetInstanceCount() <= 0 || !Pickets || !Rails) return false;

        for (int32 ProxyIndex = 0; ProxyIndex < Proxy->GetInstanceCount(); ++ProxyIndex)
        {
            FTransform ProxyTransform;
            if (!Proxy->GetInstanceTransform(ProxyIndex, ProxyTransform, true))
            {
                Pickets->ClearInstances();
                Rails->ClearInstances();
                OutAdded = 0;
                return false;
            }

            const FVector ProxyScale = ProxyTransform.GetScale3D().GetAbs();
            const bool bLongX = ProxyScale.X >= ProxyScale.Y;
            const float DesiredLength = FMath::Max(100.0f, (bLongX ? ProxyScale.X : ProxyScale.Y) * 100.0f);
            const float DesiredHeight = FMath::Clamp(ProxyScale.Z * 100.0f, 120.0f, 260.0f);
            const float Yaw = ProxyTransform.Rotator().Yaw;
            const FQuat Rotation = FQuat(FRotator(0.0f, Yaw, 0.0f));
            const FVector LocalAxis = bLongX ? FVector::ForwardVector : FVector::RightVector;
            const FVector WorldAxis = Rotation.RotateVector(LocalAxis).GetSafeNormal();
            const FVector Center = ProxyTransform.GetLocation();
            const float BaseZ = Center.Z - DesiredHeight * 0.5f;

            // Keep instance count bounded on long base-perimeter runs while still reading as an open iron fence.
            const int32 PicketCount = FMath::Clamp(FMath::CeilToInt(DesiredLength / 95.0f) + 1, 2, 96);
            const float Step = DesiredLength / static_cast<float>(FMath::Max(1, PicketCount - 1));
            for (int32 PicketIndex = 0; PicketIndex < PicketCount; ++PicketIndex)
            {
                const float Along = -DesiredLength * 0.5f + Step * static_cast<float>(PicketIndex);
                FVector Location = Center + WorldAxis * Along;
                Location.Z = BaseZ + DesiredHeight * 0.5f;

                const FVector Scale(0.055f, 0.055f, DesiredHeight / 100.0f);
                Pickets->AddInstance(FTransform(Rotation, Location, Scale), true);
                ++OutAdded;
            }

            constexpr float RailFractions[] = { 0.24f, 0.55f, 0.82f };
            for (const float Fraction : RailFractions)
            {
                FVector Location = Center;
                Location.Z = BaseZ + DesiredHeight * Fraction;

                FVector Scale;
                if (bLongX) Scale = FVector(DesiredLength / 100.0f, 0.065f, 0.065f);
                else Scale = FVector(0.065f, DesiredLength / 100.0f, 0.065f);
                Rails->AddInstance(FTransform(Rotation, Location, Scale), true);
                ++OutAdded;
            }
        }
        return OutAdded > 0;
    }
}

bool UOCR13MetalFenceBridgeSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13MetalFenceBridgeSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) BuildMetalFenceBridge(*World);
        }), 1.10f, false);
}

void UOCR13MetalFenceBridgeSubsystem::BuildMetalFenceBridge(UWorld& World)
{
    AOCWorldSectorOster* WorldSector = nullptr;
    for (TActorIterator<AOCWorldSectorOster> It(&World); It; ++It)
    {
        WorldSector = *It;
        if (WorldSector) break;
    }
    if (!WorldSector) return;

    UInstancedStaticMeshComponent* Proxy = FindISM(WorldSector, TEXT("MetalFences"));
    if (!Proxy || Proxy->GetInstanceCount() <= 0) return;

    UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!CubeMesh) return;

    AActor* ArtRoot = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!ArtRoot) return;
    ArtRoot->SetReplicates(false);

    USceneComponent* Root = NewObject<USceneComponent>(ArtRoot, TEXT("R13_MetalFenceBridgeRoot"));
    if (!Root)
    {
        ArtRoot->Destroy();
        return;
    }
    Root->SetMobility(EComponentMobility::Static);
    ArtRoot->SetRootComponent(Root);
    ArtRoot->AddInstanceComponent(Root);
    Root->RegisterComponent();

    UInstancedStaticMeshComponent* Pickets = MakeVisualISM(ArtRoot, Root, CubeMesh, TEXT("R13_MetalFencePickets"));
    UInstancedStaticMeshComponent* Rails = MakeVisualISM(ArtRoot, Root, CubeMesh, TEXT("R13_MetalFenceRails"));
    if (!Pickets || !Rails)
    {
        ArtRoot->Destroy();
        return;
    }

    ApplyMetalTint(Pickets, BaseMaterial);
    ApplyMetalTint(Rails, BaseMaterial);

    int32 Added = 0;
    const bool bComplete = BuildOpenMetalFence(Proxy, Pickets, Rails, Added);
    if (!bComplete || Added <= 0)
    {
        ArtRoot->Destroy();
        UE_LOG(LogTemp, Warning,
            TEXT("R13 metal-fence bridge: incomplete source traversal; preserving visible semantic MetalFences proxy."));
        return;
    }

    // Keep the original invisible proxy as one cheap continuous collision volume. The visible pickets/rails stay
    // collision-free, avoiding thousands of tiny collision bodies on long residential and base perimeter fences.
    Proxy->SetVisibility(false, true);

    UE_LOG(LogTemp, Display,
        TEXT("R13 metal-fence bridge: built %d visible picket/rail instances after complete source traversal; retained hidden semantic proxy collision."),
        Added);
}
