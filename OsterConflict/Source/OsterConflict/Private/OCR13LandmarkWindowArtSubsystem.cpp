#include "OCR13LandmarkWindowArtSubsystem.h"

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
        UMaterialInterface* Material, const FName Name, const bool bCastShadow)
    {
        if (!Owner || !Root || !Mesh || !Material) return nullptr;

        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, Name);
        if (!Component) return nullptr;

        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        Component->SetMaterial(0, Material);
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetCastShadow(bCastShadow);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    bool DecodeWindowProxy(const FTransform& ProxyTransform, FVector& OutSizeCm)
    {
        // LandmarkWindows are authored from the engine 100 cm cube. Their local X is facade width,
        // Y is the shallow facade-normal thickness and Z is window height.
        OutSizeCm = ProxyTransform.GetScale3D().GetAbs() * 100.0f;
        return OutSizeCm.X >= 120.0f && OutSizeCm.X <= 900.0f &&
            OutSizeCm.Y >= 5.0f && OutSizeCm.Y <= 80.0f &&
            OutSizeCm.Z >= 100.0f && OutSizeCm.Z <= 500.0f;
    }

    void AddCubeInstance(UInstancedStaticMeshComponent* Component, const FVector& LocalCenterOffsetCm,
        const FVector& LocalSizeCm, const FTransform& ProxyTransform)
    {
        if (!Component) return;

        const FQuat Rotation = ProxyTransform.GetRotation();
        const FVector WorldCenter = ProxyTransform.GetLocation() + Rotation.RotateVector(LocalCenterOffsetCm);
        Component->AddInstance(FTransform(Rotation, WorldCenter, LocalSizeCm / 100.0f), true);
    }

    int32 BuildFramedWindows(UInstancedStaticMeshComponent* Proxy,
        UInstancedStaticMeshComponent* Glass,
        UInstancedStaticMeshComponent* Frames)
    {
        if (!Proxy || !Glass || !Frames) return 0;

        int32 Replaced = 0;
        for (int32 Index = 0; Index < Proxy->GetInstanceCount(); ++Index)
        {
            FTransform ProxyTransform;
            if (!Proxy->GetInstanceTransform(Index, ProxyTransform, true)) return -1;

            FVector SizeCm;
            if (!DecodeWindowProxy(ProxyTransform, SizeCm)) return -1;

            const float FrameWidth = FMath::Clamp(FMath::Min(SizeCm.X, SizeCm.Z) * 0.055f, 5.0f, 11.0f);
            const float FrameDepth = FMath::Clamp(SizeCm.Y + 4.0f, 10.0f, 34.0f);
            const float PaneDepth = FMath::Clamp(SizeCm.Y * 0.16f, 2.0f, 5.0f);
            const float InnerWidth = FMath::Max(20.0f, SizeCm.X - FrameWidth * 2.0f);
            const float InnerHeight = FMath::Max(20.0f, SizeCm.Z - FrameWidth * 2.0f);

            AddCubeInstance(Glass, FVector::ZeroVector,
                FVector(InnerWidth, PaneDepth, InnerHeight), ProxyTransform);

            const float HalfInnerX = InnerWidth * 0.5f;
            const float HalfInnerZ = InnerHeight * 0.5f;
            AddCubeInstance(Frames, FVector(-(HalfInnerX + FrameWidth * 0.5f), 0.0f, 0.0f),
                FVector(FrameWidth, FrameDepth, SizeCm.Z), ProxyTransform);
            AddCubeInstance(Frames, FVector(HalfInnerX + FrameWidth * 0.5f, 0.0f, 0.0f),
                FVector(FrameWidth, FrameDepth, SizeCm.Z), ProxyTransform);
            AddCubeInstance(Frames, FVector(0.0f, 0.0f, -(HalfInnerZ + FrameWidth * 0.5f)),
                FVector(InnerWidth, FrameDepth, FrameWidth), ProxyTransform);
            AddCubeInstance(Frames, FVector(0.0f, 0.0f, HalfInnerZ + FrameWidth * 0.5f),
                FVector(InnerWidth, FrameDepth, FrameWidth), ProxyTransform);

            // The college and museum references both show divided window frames. Add one restrained center mullion
            // only on sufficiently wide openings rather than inventing a dense grid on every small museum window.
            if (SizeCm.X >= 300.0f)
            {
                AddCubeInstance(Frames, FVector::ZeroVector,
                    FVector(FrameWidth * 0.82f, FrameDepth, InnerHeight), ProxyTransform);
            }

            ++Replaced;
        }
        return Replaced;
    }
}

bool UOCR13LandmarkWindowArtSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13LandmarkWindowArtSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) BuildLandmarkWindowBridge(*World);
        }), 1.25f, false);
}

void UOCR13LandmarkWindowArtSubsystem::BuildLandmarkWindowBridge(UWorld& World)
{
    AOCWorldSectorOster* WorldSector = nullptr;
    for (TActorIterator<AOCWorldSectorOster> It(&World); It; ++It)
    {
        WorldSector = *It;
        if (WorldSector) break;
    }
    if (!WorldSector) return;

    UInstancedStaticMeshComponent* Proxy = FindISM(WorldSector, TEXT("LandmarkWindows"));
    if (!Proxy || Proxy->GetInstanceCount() <= 0) return;

    UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UMaterialInterface* GlassMaterial = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Materials/Instances/Glass_Window.Glass_Window"));
    UMaterialInterface* FrameMaterial = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Materials/Instances/Window_Frame.Window_Frame"));
    if (!CubeMesh || !GlassMaterial || !FrameMaterial)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("R13 landmark windows: bundled glass/frame materials unavailable; preserving semantic proxy windows."));
        return;
    }

    AActor* ArtRoot = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!ArtRoot) return;
    ArtRoot->SetReplicates(false);

    USceneComponent* Root = NewObject<USceneComponent>(ArtRoot, TEXT("R13_LandmarkWindowArtRoot"));
    if (!Root)
    {
        ArtRoot->Destroy();
        return;
    }
    Root->SetMobility(EComponentMobility::Static);
    ArtRoot->SetRootComponent(Root);
    ArtRoot->AddInstanceComponent(Root);
    Root->RegisterComponent();

    UInstancedStaticMeshComponent* Glass = MakeISM(ArtRoot, Root, CubeMesh, GlassMaterial,
        TEXT("R13_LandmarkWindowGlass"), false);
    UInstancedStaticMeshComponent* Frames = MakeISM(ArtRoot, Root, CubeMesh, FrameMaterial,
        TEXT("R13_LandmarkWindowFrames"), true);
    if (!Glass || !Frames)
    {
        ArtRoot->Destroy();
        return;
    }

    const int32 SourceCount = Proxy->GetInstanceCount();
    const int32 Replaced = BuildFramedWindows(Proxy, Glass, Frames);
    if (Replaced != SourceCount)
    {
        // All-or-nothing: partial replacement would leave a mixture of framed art and cyan cube proxies.
        ArtRoot->Destroy();
        UE_LOG(LogTemp, Warning,
            TEXT("R13 landmark windows: validation/replacement incomplete (%d/%d); preserving all source proxies."),
            FMath::Max(0, Replaced), SourceCount);
        return;
    }

    Proxy->SetVisibility(false, true);
    Proxy->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    UE_LOG(LogTemp, Display,
        TEXT("R13 landmark windows: replaced %d source-authored museum/college window proxies with framed bundled glass; landmark massing untouched."),
        Replaced);
}
