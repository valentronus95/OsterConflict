#include "OCR145MuseumTreeLayoutSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    constexpr float R145TreeLayoutDelaySeconds = 6.34f;

    UInstancedStaticMeshComponent* MakeISM(AActor* Owner, USceneComponent* Root, UStaticMesh* Mesh,
        const TCHAR* Name, const int32 CullEndCm)
    {
        if (!Owner || !Root || !Mesh) return nullptr;
        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(
            Owner, MakeUniqueObjectName(Owner, UInstancedStaticMeshComponent::StaticClass(), FName(Name)));
        if (!Component) return nullptr;
        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionProfileName(TEXT("BlockAll"));
        Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(false);
        Component->SetCastShadow(false);
        Component->SetCullDistances(0, FMath::Min(CullEndCm, 45000));
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    void AddGroundedTree(UInstancedStaticMeshComponent* Component, UStaticMesh* Mesh,
        const FVector& GroundLocation, const float DesiredHeightCm, const float YawDegrees,
        const float WidthScale = 1.0f)
    {
        if (!Component || !Mesh) return;
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        if (NativeSize.Z <= 10.0f) return;

        const float HeightScale = FMath::Clamp(DesiredHeightCm / NativeSize.Z, 0.25f, 4.0f);
        const FVector Scale(HeightScale * WidthScale, HeightScale * WidthScale, HeightScale);
        const float LocalBottom = Bounds.Origin.Z - Bounds.BoxExtent.Z;
        FVector Location = GroundLocation;
        Location.Z += -LocalBottom * HeightScale;
        Component->AddInstance(FTransform(FRotator(0.0f, YawDegrees, 0.0f), Location, Scale), true);
    }

    void HideR137MuseumTrees(UWorld& World)
    {
        for (TActorIterator<AActor> It(&World); It; ++It)
        {
            AActor* Actor = *It;
            if (!Actor || !Actor->ActorHasTag(TEXT("R137_MuseumPhotoModel"))) continue;

            TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
            Actor->GetComponents(Components);
            for (UInstancedStaticMeshComponent* Component : Components)
            {
                if (!Component) continue;
                const FName Name = Component->GetFName();
                if (Name != TEXT("R137Museum_Pine01") &&
                    Name != TEXT("R137Museum_Pine03") &&
                    Name != TEXT("R137Museum_Deciduous01"))
                {
                    continue;
                }
                Component->SetVisibility(false, true);
                Component->SetHiddenInGame(true, true);
                Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            }
        }
    }
}

bool UOCR145MuseumTreeLayoutSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR145MuseumTreeLayoutSubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
            if (UWorld* World = WeakWorld.Get()) ReplaceMuseumTrees(*World);
        }), R145TreeLayoutDelaySeconds, false);
}

void UOCR145MuseumTreeLayoutSubsystem::ReplaceMuseumTrees(UWorld& World) const
{
    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        if (AActor* Actor = *It; Actor && Actor->ActorHasTag(TEXT("R145_MuseumPhotoTreeLayout"))) return;
    }

    UStaticMesh* Pine01 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_01.SM_Pine_Tree_01"));
    UStaticMesh* Pine03 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_03.SM_Pine_Tree_03"));
    UStaticMesh* Deciduous = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var01.SM_Tree_Var01"));
    if (!Pine01 && !Pine03 && !Deciduous) return;

    HideR137MuseumTrees(World);

    AActor* TreesActor = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!TreesActor) return;
    TreesActor->SetReplicates(false);
    TreesActor->Tags.Add(TEXT("R145_MuseumPhotoTreeLayout"));

    USceneComponent* Root = NewObject<USceneComponent>(TreesActor,
        MakeUniqueObjectName(TreesActor, USceneComponent::StaticClass(), FName(TEXT("R145MuseumTreeRoot"))));
    if (!Root)
    {
        TreesActor->Destroy();
        return;
    }
    TreesActor->SetRootComponent(Root);
    TreesActor->AddInstanceComponent(Root);
    Root->RegisterComponent();

    UInstancedStaticMeshComponent* Pine01ISM = MakeISM(TreesActor, Root, Pine01,
        TEXT("R145Museum_Pine01"), 100000);
    UInstancedStaticMeshComponent* Pine03ISM = MakeISM(TreesActor, Root, Pine03,
        TEXT("R145Museum_Pine03"), 100000);
    UInstancedStaticMeshComponent* DeciduousISM = MakeISM(TreesActor, Root, Deciduous,
        TEXT("R145Museum_Deciduous"), 90000);

    struct FTreeSeed
    {
        FVector Offset;
        float Height;
        float Yaw;
        float WidthScale;
        int32 Family;
    };

    // REF-04/05/07/10/14: conifers frame the approach irregularly. They are intentionally not mirrored.
    const FTreeSeed Seeds[] =
    {
        { FVector(-640.0f, -1280.0f, 0.0f), 2140.0f,  18.0f, 1.03f, 0 },
        { FVector( 815.0f, -1520.0f, 0.0f), 2260.0f,  71.0f, 0.95f, 1 },
        { FVector(-1110.0f,-2180.0f, 0.0f), 2380.0f, 127.0f, 1.08f, 1 },
        { FVector( 520.0f, -2460.0f, 0.0f), 2050.0f, 163.0f, 0.92f, 0 },
        { FVector( 1140.0f,-3010.0f, 0.0f), 2310.0f, 204.0f, 1.05f, 1 },
        { FVector(-760.0f, -3520.0f, 0.0f), 2210.0f, 249.0f, 0.96f, 0 },
        { FVector( 1290.0f,-4070.0f, 0.0f), 2440.0f, 292.0f, 1.02f, 0 },
        { FVector(-1460.0f,-4250.0f, 0.0f), 2280.0f, 326.0f, 0.98f, 1 },

        // REF-01/03/09/13: mature deciduous trees sit close to the building and break the conifer rhythm.
        { FVector( 1250.0f, -230.0f, 0.0f), 1870.0f,  37.0f, 1.08f, 2 },
        { FVector(-1340.0f,  390.0f, 0.0f), 1800.0f, 118.0f, 1.02f, 2 },
        { FVector( 1510.0f,  930.0f, 0.0f), 1950.0f, 211.0f, 1.10f, 2 },
        { FVector(-980.0f, 1240.0f, 0.0f), 1730.0f, 303.0f, 0.94f, 2 },

        // Side conifers visible beyond the long facade, kept clear of windows and the service entry.
        { FVector(-1730.0f, -420.0f, 0.0f), 2010.0f,  56.0f, 0.96f, 0 },
        { FVector( 1810.0f,  480.0f, 0.0f), 2160.0f, 176.0f, 1.00f, 1 }
    };

    UInstancedStaticMeshComponent* Families[] = { Pine01ISM, Pine03ISM, DeciduousISM };
    UStaticMesh* Meshes[] = { Pine01, Pine03, Deciduous };
    int32 Placed = 0;
    for (const FTreeSeed& Seed : Seeds)
    {
        if (Seed.Family < 0 || Seed.Family >= UE_ARRAY_COUNT(Families)) continue;
        if (!Families[Seed.Family] || !Meshes[Seed.Family]) continue;
        AddGroundedTree(Families[Seed.Family], Meshes[Seed.Family],
            AOCWorldSectorOster::MuseumAnchor() + Seed.Offset, Seed.Height, Seed.Yaw, Seed.WidthScale);
        ++Placed;
    }

    UE_LOG(LogTemp, Display,
        TEXT("R14.5 museum trees: hidden symmetric R13.7 tree pass and placed %d asymmetrical photo-oriented mature trees; central approach remains open."),
        Placed);
}
