#include "OCR13OsterPropArtSubsystem.h"

#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
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
        Component->SetCollisionProfileName(TEXT("BlockAll"));
        Component->SetCastShadow(true);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    int32 AddFenceModules(UInstancedStaticMeshComponent* Proxy,
        const TArray<UInstancedStaticMeshComponent*>& Families)
    {
        if (!Proxy || Families.Num() == 0) return 0;

        int32 Added = 0;
        for (int32 ProxyIndex = 0; ProxyIndex < Proxy->GetInstanceCount(); ++ProxyIndex)
        {
            FTransform ProxyTransform;
            if (!Proxy->GetInstanceTransform(ProxyIndex, ProxyTransform, true)) continue;

            const FVector ProxyScale = ProxyTransform.GetScale3D().GetAbs();
            const bool bDesiredLongX = ProxyScale.X >= ProxyScale.Y;
            const float DesiredLength = FMath::Max(100.0f,
                (bDesiredLongX ? ProxyScale.X : ProxyScale.Y) * 100.0f);
            const float DesiredHeight = FMath::Max(100.0f, ProxyScale.Z * 100.0f);

            UInstancedStaticMeshComponent* SampleFamily = Families[ProxyIndex % Families.Num()];
            UStaticMesh* SampleMesh = SampleFamily ? SampleFamily->GetStaticMesh() : nullptr;
            if (!SampleMesh) continue;

            const FVector SampleSize = SampleMesh->GetBounds().BoxExtent * 2.0f;
            const bool bSampleLongX = SampleSize.X >= SampleSize.Y;
            const float SampleLong = FMath::Max(1.0f, bSampleLongX ? SampleSize.X : SampleSize.Y);
            const float SampleHeight = FMath::Max(1.0f, SampleSize.Z);
            const float HeightScale = FMath::Clamp(DesiredHeight / SampleHeight, 0.55f, 2.20f);
            const float NaturalModuleLength = SampleLong * HeightScale;
            const int32 ModuleCount = FMath::Clamp(FMath::CeilToInt(DesiredLength / NaturalModuleLength), 1, 64);
            const float ModuleSpacing = DesiredLength / static_cast<float>(ModuleCount);

            const FVector DesiredAxisLocal = bDesiredLongX ? FVector::ForwardVector : FVector::RightVector;
            const FVector DesiredAxisWorld = ProxyTransform.GetRotation().RotateVector(DesiredAxisLocal).GetSafeNormal();

            for (int32 ModuleIndex = 0; ModuleIndex < ModuleCount; ++ModuleIndex)
            {
                UInstancedStaticMeshComponent* Target = Families[(ProxyIndex + ModuleIndex) % Families.Num()];
                UStaticMesh* Mesh = Target ? Target->GetStaticMesh() : nullptr;
                if (!Target || !Mesh) continue;

                const FBoxSphereBounds Bounds = Mesh->GetBounds();
                const FVector MeshSize = Bounds.BoxExtent * 2.0f;
                const bool bMeshLongX = MeshSize.X >= MeshSize.Y;
                const float MeshLong = FMath::Max(1.0f, bMeshLongX ? MeshSize.X : MeshSize.Y);
                const float MeshHeight = FMath::Max(1.0f, MeshSize.Z);
                const float ZScale = FMath::Clamp(DesiredHeight / MeshHeight, 0.55f, 2.20f);

                FVector Scale(ZScale);
                if (bMeshLongX) Scale.X = ModuleSpacing / MeshLong;
                else Scale.Y = ModuleSpacing / MeshLong;

                FQuat Rotation = ProxyTransform.GetRotation();
                if (bDesiredLongX != bMeshLongX)
                {
                    Rotation = Rotation * FQuat(FVector::UpVector, FMath::DegreesToRadians(90.0f));
                }

                const float Along = -0.5f * DesiredLength + (static_cast<float>(ModuleIndex) + 0.5f) * ModuleSpacing;
                FVector Location = ProxyTransform.GetLocation() + DesiredAxisWorld * Along;
                Location -= Rotation.RotateVector(Bounds.Origin * Scale);

                Target->AddInstance(FTransform(Rotation, Location, Scale), true);
                ++Added;
            }
        }
        return Added;
    }
}

bool UOCR13OsterPropArtSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13OsterPropArtSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) ApplyPropBridge(*World);
        }), 1.05f, false);
}

void UOCR13OsterPropArtSubsystem::ApplyPropBridge(UWorld& World)
{
    AOCWorldSectorOster* WorldSector = nullptr;
    for (TActorIterator<AOCWorldSectorOster> It(&World); It; ++It)
    {
        WorldSector = *It;
        if (WorldSector) break;
    }
    if (!WorldSector) return;

    UInstancedStaticMeshComponent* WoodProxy = FindISM(WorldSector, TEXT("WoodFences"));
    if (!WoodProxy || WoodProxy->GetInstanceCount() == 0) return;

    UStaticMesh* Fence01 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Fence_Old_1_2m.Fence_Old_1_2m"));
    UStaticMesh* Fence02 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Fence_Old_2_2m.Fence_Old_2_2m"));
    UStaticMesh* Fence03 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Fence_Old_3_2m.Fence_Old_3_2m"));

    if (!Fence01 && !Fence02 && !Fence03)
    {
        UE_LOG(LogTemp, Warning, TEXT("R13 Oster props: bundled rural-cabin fence meshes unavailable; keeping WoodFences proxies."));
        return;
    }

    AActor* ArtRoot = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!ArtRoot) return;
    ArtRoot->SetReplicates(false);

    USceneComponent* Root = NewObject<USceneComponent>(ArtRoot, TEXT("R13_OsterPropArtRoot"));
    if (!Root) return;
    ArtRoot->SetRootComponent(Root);
    ArtRoot->AddInstanceComponent(Root);
    Root->RegisterComponent();

    TArray<UInstancedStaticMeshComponent*> FenceFamilies;
    if (UInstancedStaticMeshComponent* Family = MakeISM(ArtRoot, Root, Fence01, TEXT("R13_WoodFence01"))) FenceFamilies.Add(Family);
    if (UInstancedStaticMeshComponent* Family = MakeISM(ArtRoot, Root, Fence02, TEXT("R13_WoodFence02"))) FenceFamilies.Add(Family);
    if (UInstancedStaticMeshComponent* Family = MakeISM(ArtRoot, Root, Fence03, TEXT("R13_WoodFence03"))) FenceFamilies.Add(Family);

    const int32 Added = AddFenceModules(WoodProxy, FenceFamilies);
    if (Added > 0)
    {
        WoodProxy->SetVisibility(false, true);
        WoodProxy->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    UE_LOG(LogTemp, Display, TEXT("R13 Oster props: wood fence modules=%d; source proxies %s."),
        Added, Added > 0 ? TEXT("hidden") : TEXT("preserved"));
}
