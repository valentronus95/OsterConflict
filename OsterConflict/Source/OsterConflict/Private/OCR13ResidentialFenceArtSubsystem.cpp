#include "OCR13ResidentialFenceArtSubsystem.h"

#include "OCGameMode.h"
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
    constexpr float FenceArtDelaySeconds = 2.45f;

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
        Component->SetCollisionProfileName(TEXT("NoCollision"));
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(false);
        Component->SetCastShadow(true);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    bool IsMuseumRearValley(const FVector& Location)
    {
        return FMath::Abs(Location.X) <= 10000.0f &&
            Location.Y >= 2650.0f && Location.Y <= 34150.0f;
    }

    void AddFenceReplacement(UInstancedStaticMeshComponent* Target, UStaticMesh* Mesh,
        const FTransform& ProxyTransform)
    {
        if (!Target || !Mesh) return;

        const FVector ProxySize = ProxyTransform.GetScale3D().GetAbs() * 100.0f;
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        if (NativeSize.X <= 1.0f || NativeSize.Y <= 1.0f || NativeSize.Z <= 1.0f) return;

        const bool bProxyLongX = ProxySize.X >= ProxySize.Y;
        const bool bMeshLongX = NativeSize.X >= NativeSize.Y;
        const float DesiredSpan = FMath::Max(ProxySize.X, ProxySize.Y);
        const float DesiredThickness = FMath::Clamp(FMath::Min(ProxySize.X, ProxySize.Y), 22.0f, 95.0f);
        const float DesiredHeight = FMath::Clamp(ProxySize.Z, 120.0f, 260.0f);

        const float NativeLong = bMeshLongX ? NativeSize.X : NativeSize.Y;
        const float NativeShort = bMeshLongX ? NativeSize.Y : NativeSize.X;
        const float LongScale = FMath::Clamp(DesiredSpan / NativeLong, 0.18f, 12.0f);
        const float ShortScale = FMath::Clamp(DesiredThickness / FMath::Max(1.0f, NativeShort), 0.20f, 4.0f);
        const float ZScale = FMath::Clamp(DesiredHeight / NativeSize.Z, 0.20f, 4.0f);

        FVector Scale;
        if (bMeshLongX)
            Scale = FVector(LongScale, ShortScale, ZScale);
        else
            Scale = FVector(ShortScale, LongScale, ZScale);

        float Yaw = ProxyTransform.Rotator().Yaw;
        if (bProxyLongX != bMeshLongX) Yaw += 90.0f;
        const FRotator Rotation(0.0f, Yaw, 0.0f);

        const float ProxyGroundZ = ProxyTransform.GetLocation().Z - ProxySize.Z * 0.5f;
        const float LocalBottom = Bounds.Origin.Z - Bounds.BoxExtent.Z;
        FVector Location = ProxyTransform.GetLocation();
        const FVector PivotXY = Rotation.RotateVector(FVector(
            -Bounds.Origin.X * Scale.X, -Bounds.Origin.Y * Scale.Y, 0.0f));
        Location.X += PivotXY.X;
        Location.Y += PivotXY.Y;
        Location.Z = ProxyGroundZ - LocalBottom * Scale.Z;

        Target->AddInstance(FTransform(Rotation, Location, Scale), true);
    }
}

bool UOCR13ResidentialFenceArtSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13ResidentialFenceArtSubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
            if (UWorld* World = WeakWorld.Get()) ApplyFenceArt(*World);
        }), FenceArtDelaySeconds, false);
}

void UOCR13ResidentialFenceArtSubsystem::ApplyFenceArt(UWorld& World)
{
    AOCWorldSectorOster* Sector = nullptr;
    for (TActorIterator<AOCWorldSectorOster> It(&World); It; ++It)
    {
        Sector = *It;
        if (Sector) break;
    }
    if (!Sector) return;

    UStaticMesh* Fence01 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Fence_Var01.SM_Fence_Var01"));
    UStaticMesh* Fence02 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Fence_Var02.SM_Fence_Var02"));
    UStaticMesh* Fence03 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Fence_Var03.SM_Fence_Var03"));
    UStaticMesh* Fence04 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Fence_Var04.SM_Fence_Var04"));
    if (!Fence01 && !Fence02 && !Fence03 && !Fence04) return;

    AActor* ArtRoot = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!ArtRoot) return;
    ArtRoot->SetReplicates(false);
    ArtRoot->SetActorEnableCollision(false);

    USceneComponent* Root = NewObject<USceneComponent>(ArtRoot, TEXT("R13_ResidentialFenceArtRoot"));
    if (!Root)
    {
        ArtRoot->Destroy();
        return;
    }
    ArtRoot->SetRootComponent(Root);
    ArtRoot->AddInstanceComponent(Root);
    Root->RegisterComponent();

    UStaticMesh* Meshes[] = { Fence01, Fence02, Fence03, Fence04 };
    UInstancedStaticMeshComponent* Targets[] = {
        MakeVisualISM(ArtRoot, Root, Fence01, TEXT("R13_YardFence01")),
        MakeVisualISM(ArtRoot, Root, Fence02, TEXT("R13_YardFence02")),
        MakeVisualISM(ArtRoot, Root, Fence03, TEXT("R13_YardFence03")),
        MakeVisualISM(ArtRoot, Root, Fence04, TEXT("R13_YardFence04")),
    };

    struct FSourceFamily { FName Name; int32 VariantBase; };
    const FSourceFamily Families[] = {
        { TEXT("WoodFences"), 0 },
        { TEXT("MetalFences"), 2 },
        { TEXT("LightSheetFences"), 1 },
    };

    int32 Replaced = 0;
    int32 PreservedValley = 0;
    for (const FSourceFamily& Family : Families)
    {
        UInstancedStaticMeshComponent* Source = FindISM(Sector, Family.Name);
        if (!Source) continue;

        for (int32 Index = 0; Index < Source->GetInstanceCount(); ++Index)
        {
            FTransform Transform;
            if (!Source->GetInstanceTransform(Index, Transform, true)) continue;
            if (IsMuseumRearValley(Transform.GetLocation()))
            {
                ++PreservedValley;
                continue;
            }

            const int32 Variant = (Family.VariantBase + Index) % UE_ARRAY_COUNT(Targets);
            if (!Meshes[Variant] || !Targets[Variant]) continue;
            AddFenceReplacement(Targets[Variant], Meshes[Variant], Transform);
            ++Replaced;
        }

        // Keep the old invisible cube instances as the proven collision/navigation contract.
        Source->SetVisibility(false, true);
        Source->SetHiddenInGame(true, true);
    }

    UE_LOG(LogTemp, Display,
        TEXT("R13 residential fence art: real AdvancedVillage fence instances=%d; valley instances skipped for dedicated lower-district topology=%d; source collision retained invisibly."),
        Replaced, PreservedValley);
}
