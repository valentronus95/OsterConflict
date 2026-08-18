#include "OCR13ResidentialYardSubsystem.h"

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
    struct FYardFamily
    {
        UInstancedStaticMeshComponent* Component = nullptr;
        UStaticMesh* Mesh = nullptr;
    };

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
        const FName Name)
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

    bool IsReservedArea(const FVector& Location)
    {
        if (Location.Size2D() <= 10500.0f) return true; // museum reference garden
        return FMath::Abs(Location.X + 3400.0f) < 7200.0f &&
            Location.Y > -15000.0f && Location.Y < 18000.0f; // dedicated Krushelnytska slice
    }

    void AddGrounded(const FYardFamily& Family, FVector Location, const float Scale, const float Yaw)
    {
        if (!Family.Component || !Family.Mesh) return;
        const FBoxSphereBounds Bounds = Family.Mesh->GetBounds();
        const float LocalBottom = Bounds.Origin.Z - Bounds.BoxExtent.Z;
        Location.Z = -LocalBottom * Scale + 2.0f;
        Family.Component->AddInstance(
            FTransform(FRotator(0.0f, Yaw, 0.0f), Location, FVector(Scale)), true);
    }

    FYardFamily MakeFamily(AActor* Owner, USceneComponent* Root, const TCHAR* AssetPath, const FName Name)
    {
        FYardFamily Family;
        Family.Mesh = LoadObject<UStaticMesh>(nullptr, AssetPath);
        Family.Component = MakeISM(Owner, Root, Family.Mesh, Name);
        return Family;
    }
}

bool UOCR13ResidentialYardSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13ResidentialYardSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) BuildResidentialYards(*World);
        }), 1.85f, false);
}

void UOCR13ResidentialYardSubsystem::BuildResidentialYards(UWorld& World)
{
    AOCWorldSectorOster* WorldSector = nullptr;
    for (TActorIterator<AOCWorldSectorOster> It(&World); It; ++It)
    {
        WorldSector = *It;
        if (WorldSector) break;
    }
    if (!WorldSector) return;

    UInstancedStaticMeshComponent* Buildings = FindISM(WorldSector, TEXT("Buildings"));
    if (!Buildings || Buildings->GetInstanceCount() == 0) return;

    AActor* ArtRoot = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!ArtRoot) return;
    ArtRoot->SetReplicates(false);
    ArtRoot->SetActorEnableCollision(false);

    USceneComponent* Root = NewObject<USceneComponent>(ArtRoot, TEXT("R13_ResidentialYardRoot"));
    if (!Root)
    {
        ArtRoot->Destroy();
        return;
    }
    Root->SetMobility(EComponentMobility::Static);
    ArtRoot->SetRootComponent(Root);
    ArtRoot->AddInstanceComponent(Root);
    Root->RegisterComponent();

    const FYardFamily SideShed = MakeFamily(ArtRoot, Root,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Side_Shed.Side_Shed"), TEXT("R13_YardSideShed"));
    const FYardFamily Outhouse = MakeFamily(ArtRoot, Root,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Outhouse_House.Outhouse_House"), TEXT("R13_YardOuthouse"));
    const FYardFamily LogPile = MakeFamily(ArtRoot, Root,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Log_Pile_1.Log_Pile_1"), TEXT("R13_YardLogPile"));
    const FYardFamily Wheelbarrow = MakeFamily(ArtRoot, Root,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Wheel_Barrow.Wheel_Barrow"), TEXT("R13_YardWheelbarrow"));
    const FYardFamily Barrel = MakeFamily(ArtRoot, Root,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Metal_Barrel.Metal_Barrel"), TEXT("R13_YardBarrel"));
    const FYardFamily Pallet = MakeFamily(ArtRoot, Root,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Pallet.Pallet"), TEXT("R13_YardPallet"));
    const FYardFamily Tire = MakeFamily(ArtRoot, Root,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Tire.Tire"), TEXT("R13_YardTire"));
    const FYardFamily UtilityBox = MakeFamily(ArtRoot, Root,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Utility_Box_1a.Utility_Box_1a"), TEXT("R13_YardUtilityBox"));

    int32 PropCount = 0;
    for (int32 Index = 0; Index < Buildings->GetInstanceCount(); ++Index)
    {
        FTransform HouseTransform;
        if (!Buildings->GetInstanceTransform(Index, HouseTransform, true)) continue;
        const FVector HouseLocation = HouseTransform.GetLocation();
        if (IsReservedArea(HouseLocation)) continue;

        const float HouseYaw = HouseTransform.Rotator().Yaw;
        const FQuat Rotation(FRotator(0.0f, HouseYaw, 0.0f));

        auto YardLocation = [&](const float SideCm, const float BackCm) -> FVector
        {
            return HouseLocation + Rotation.RotateVector(FVector(BackCm, SideCm, 0.0f));
        };

        // Larger outbuildings are intentionally rare. They are visual-only until their collision footprint can be
        // validated in the finished map, so they cannot alter bot or vehicle paths during this art batch.
        if ((Index % 7) == 1 && SideShed.Component)
        {
            AddGrounded(SideShed, YardLocation(1450.0f, -1750.0f), 0.92f, HouseYaw + 88.0f);
            ++PropCount;
        }
        else if ((Index % 9) == 4 && Outhouse.Component)
        {
            AddGrounded(Outhouse, YardLocation(-1550.0f, -1900.0f), 0.88f, HouseYaw - 92.0f);
            ++PropCount;
        }

        if ((Index % 3) == 0 && LogPile.Component)
        {
            AddGrounded(LogPile, YardLocation(-1050.0f, -1180.0f), 0.92f + 0.04f * (Index % 3), HouseYaw + 15.0f);
            ++PropCount;
        }
        if ((Index % 5) == 2 && Wheelbarrow.Component)
        {
            AddGrounded(Wheelbarrow, YardLocation(950.0f, -900.0f), 0.92f, HouseYaw + 33.0f);
            ++PropCount;
        }
        if ((Index % 6) == 3 && Barrel.Component)
        {
            AddGrounded(Barrel, YardLocation(-750.0f, -720.0f), 0.94f, HouseYaw + 11.0f);
            ++PropCount;
        }
        if ((Index % 8) == 5 && Pallet.Component)
        {
            AddGrounded(Pallet, YardLocation(1120.0f, -1320.0f), 0.96f, HouseYaw - 19.0f);
            ++PropCount;
        }
        if ((Index % 10) == 6 && Tire.Component)
        {
            AddGrounded(Tire, YardLocation(720.0f, -620.0f), 0.90f, HouseYaw + 47.0f);
            ++PropCount;
        }
        if ((Index % 11) == 8 && UtilityBox.Component)
        {
            AddGrounded(UtilityBox, YardLocation(-820.0f, 650.0f), 0.90f, HouseYaw + 90.0f);
            ++PropCount;
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("R13.4 residential yards: %d restrained non-blocking rural props placed from authored house topology."),
        PropCount);
}
