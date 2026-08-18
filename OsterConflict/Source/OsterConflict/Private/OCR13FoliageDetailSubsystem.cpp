#include "OCR13FoliageDetailSubsystem.h"

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

    UInstancedStaticMeshComponent* MakeISM(AActor* Owner, USceneComponent* Root, UStaticMesh* Mesh,
        const FName Name, const bool bCastShadow)
    {
        if (!Owner || !Root || !Mesh) return nullptr;
        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, Name);
        if (!Component) return nullptr;
        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionProfileName(TEXT("NoCollision"));
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetCastShadow(bCastShadow);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    bool IsReservedReferenceArea(const FVector& Location)
    {
        // Museum has its own photo-reference composition. Krushelnytska keeps its dedicated slice pass.
        if (Location.Size2D() <= 9500.0f) return true;
        return FMath::Abs(Location.X + 3400.0f) < 7000.0f &&
            Location.Y > -14500.0f && Location.Y < 17500.0f;
    }

    void AddGrounded(UInstancedStaticMeshComponent* Target, UStaticMesh* Mesh, FVector Location,
        const float DesiredHeightCm, const float Yaw)
    {
        if (!Target || !Mesh) return;
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector Size = Bounds.BoxExtent * 2.0f;
        if (Size.Z <= 1.0f) return;
        const float Scale = FMath::Clamp(DesiredHeightCm / Size.Z, 0.18f, 5.0f);
        const float LocalBottom = Bounds.Origin.Z - Bounds.BoxExtent.Z;
        Location.Z = -LocalBottom * Scale + 2.0f;
        Target->AddInstance(FTransform(FRotator(0.0f, Yaw, 0.0f), Location, FVector(Scale)), true);
    }

    void ScatterFromProxy(UInstancedStaticMeshComponent* Proxy,
        UInstancedStaticMeshComponent* Bushes, UStaticMesh* BushMesh,
        UInstancedStaticMeshComponent* Shrubs, UStaticMesh* ShrubMesh,
        UInstancedStaticMeshComponent* EdgeGrass, UStaticMesh* EdgeGrassMesh,
        const bool bRough, int32& OutBushes, int32& OutGrass)
    {
        if (!Proxy) return;

        for (int32 Index = 0; Index < Proxy->GetInstanceCount(); ++Index)
        {
            FTransform Transform;
            if (!Proxy->GetInstanceTransform(Index, Transform, true)) continue;
            const FVector Center = Transform.GetLocation();
            if (IsReservedReferenceArea(Center)) continue;

            const FVector ProxyScale = Transform.GetScale3D().GetAbs();
            const float Width = FMath::Max(500.0f, ProxyScale.X * 100.0f);
            const float Depth = FMath::Max(500.0f, ProxyScale.Y * 100.0f);
            const FQuat Rotation = FQuat(FRotator(0.0f, Transform.Rotator().Yaw, 0.0f));

            // One or two restrained shrub groups around the perimeter. No collision, so this cannot rewrite lanes.
            const int32 ShrubCount = bRough ? 2 : ((Index % 3) == 0 ? 1 : 0);
            for (int32 Local = 0; Local < ShrubCount; ++Local)
            {
                const float SX = Local == 0 ? 0.34f : -0.31f;
                const float SY = Local == 0 ? -0.29f : 0.33f;
                const float JitterX = static_cast<float>(((Index * 19 + Local * 11) % 9) - 4) * 24.0f;
                const float JitterY = static_cast<float>(((Index * 13 + Local * 17) % 9) - 4) * 24.0f;
                const FVector Offset(SX * Width + JitterX, SY * Depth + JitterY, 0.0f);
                const FVector Location = Center + Rotation.RotateVector(Offset);
                const bool bUseBush = ((Index + Local) % 2) == 0;
                UInstancedStaticMeshComponent* Target = bUseBush ? Bushes : Shrubs;
                UStaticMesh* Mesh = bUseBush ? BushMesh : ShrubMesh;
                const float Height = bRough ? (135.0f + 18.0f * static_cast<float>((Index + Local) % 4))
                    : (95.0f + 14.0f * static_cast<float>((Index + Local) % 3));
                AddGrounded(Target, Mesh, Location, Height,
                    static_cast<float>((Index * 43 + Local * 97) % 360));
                if (Target && Mesh) ++OutBushes;
            }

            if (!bRough || !EdgeGrass || !EdgeGrassMesh) continue;

            // Rough parcels get three taller edge clumps. The main PN grass grid remains the dominant ground layer.
            for (int32 Local = 0; Local < 3; ++Local)
            {
                const float Angle = static_cast<float>((Index * 61 + Local * 119) % 360);
                const float Rad = FMath::DegreesToRadians(Angle);
                const float RX = (0.31f + 0.045f * Local) * Width;
                const float RY = (0.29f + 0.04f * ((Local + 1) % 3)) * Depth;
                FVector Offset(FMath::Cos(Rad) * RX, FMath::Sin(Rad) * RY, 0.0f);
                const FVector Location = Center + Rotation.RotateVector(Offset);
                AddGrounded(EdgeGrass, EdgeGrassMesh, Location,
                    48.0f + 7.0f * static_cast<float>((Index + Local) % 4), Angle + 23.0f);
                ++OutGrass;
            }
        }
    }
}

bool UOCR13FoliageDetailSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13FoliageDetailSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) BuildFoliageDetail(*World);
        }), 1.70f, false);
}

void UOCR13FoliageDetailSubsystem::BuildFoliageDetail(UWorld& World)
{
    AOCWorldSectorOster* WorldSector = nullptr;
    for (TActorIterator<AOCWorldSectorOster> It(&World); It; ++It)
    {
        WorldSector = *It;
        if (WorldSector) break;
    }
    if (!WorldSector) return;

    UInstancedStaticMeshComponent* RoughProxy = FindISM(WorldSector, TEXT("GrassRough"));
    UInstancedStaticMeshComponent* MownProxy = FindISM(WorldSector, TEXT("GrassMown"));
    if (!RoughProxy && !MownProxy) return;

    UStaticMesh* BushMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/Bush_1.Bush_1"));
    UStaticMesh* ShrubMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/Shrubs_1_Single.Shrubs_1_Single"));
    UStaticMesh* EdgeGrassMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/Grass_Patch_Long.Grass_Patch_Long"));
    if (!BushMesh && !ShrubMesh && !EdgeGrassMesh) return;

    AActor* ArtRoot = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!ArtRoot) return;
    ArtRoot->SetReplicates(false);
    ArtRoot->SetActorEnableCollision(false);

    USceneComponent* Root = NewObject<USceneComponent>(ArtRoot, TEXT("R13_FoliageDetailRoot"));
    if (!Root)
    {
        ArtRoot->Destroy();
        return;
    }
    Root->SetMobility(EComponentMobility::Static);
    ArtRoot->SetRootComponent(Root);
    ArtRoot->AddInstanceComponent(Root);
    Root->RegisterComponent();

    UInstancedStaticMeshComponent* Bushes = MakeISM(ArtRoot, Root, BushMesh, TEXT("R13_BushDetail"), true);
    UInstancedStaticMeshComponent* Shrubs = MakeISM(ArtRoot, Root, ShrubMesh, TEXT("R13_ShrubDetail"), true);
    UInstancedStaticMeshComponent* EdgeGrass = MakeISM(ArtRoot, Root, EdgeGrassMesh, TEXT("R13_RoughGrassEdge"), false);

    int32 BushCount = 0;
    int32 GrassCount = 0;
    ScatterFromProxy(RoughProxy, Bushes, BushMesh, Shrubs, ShrubMesh, EdgeGrass, EdgeGrassMesh,
        true, BushCount, GrassCount);
    ScatterFromProxy(MownProxy, Bushes, BushMesh, Shrubs, ShrubMesh, EdgeGrass, EdgeGrassMesh,
        false, BushCount, GrassCount);

    UE_LOG(LogTemp, Display,
        TEXT("R13.4 foliage detail: non-blocking shrubs=%d rough-edge grass clumps=%d."),
        BushCount, GrassCount);
}
