#include "OCR13CentralParkCanopySubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

namespace
{
    constexpr float CanopyDelaySeconds = 2.15f;

    struct FTreeSeed
    {
        FVector Offset;
        float HeightCm;
        float Yaw;
        int32 Variant;
    };

    UInstancedStaticMeshComponent* MakeTreeISM(AActor* Owner, USceneComponent* Root,
        UStaticMesh* Mesh, const FName Name)
    {
        if (!Owner || !Root || !Mesh) return nullptr;
        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, Name);
        if (!Component) return nullptr;
        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(false);
        Component->SetCastShadow(true);
        Component->SetCullDistances(0, 95000);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    void AddHeightMatched(UInstancedStaticMeshComponent* Target, FVector GroundLocation,
        const float DesiredHeightCm, const float YawDegrees)
    {
        if (!Target || !Target->GetStaticMesh()) return;
        const FBoxSphereBounds Bounds = Target->GetStaticMesh()->GetBounds();
        const FVector Size = Bounds.BoxExtent * 2.0f;
        if (Size.Z <= 1.0f) return;
        const float Scale = FMath::Clamp(DesiredHeightCm / Size.Z, 0.25f, 4.0f);
        const float LocalBottom = Bounds.Origin.Z - Bounds.BoxExtent.Z;
        GroundLocation.Z = -LocalBottom * Scale;
        Target->AddInstance(FTransform(FRotator(0.0f, YawDegrees, 0.0f), GroundLocation, FVector(Scale)), true);
    }
}

bool UOCR13CentralParkCanopySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13CentralParkCanopySubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
            if (UWorld* World = WeakWorld.Get()) ApplyCentralParkCanopy(*World);
        }), CanopyDelaySeconds, false);
}

void UOCR13CentralParkCanopySubsystem::ApplyCentralParkCanopy(UWorld& World)
{
    if (bApplied) return;

    UStaticMesh* TreeMeshes[] = {
        LoadObject<UStaticMesh>(nullptr, TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var03.SM_Tree_Var03")),
        LoadObject<UStaticMesh>(nullptr, TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var04.SM_Tree_Var04")),
        LoadObject<UStaticMesh>(nullptr, TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var05.SM_Tree_Var05")),
        LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_03.SM_Pine_Tree_03")),
        LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_05.SM_Pine_Tree_05")),
    };

    AActor* ArtRoot = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!ArtRoot) return;
    ArtRoot->SetReplicates(false);
    ArtRoot->SetActorEnableCollision(false);

    USceneComponent* Root = NewObject<USceneComponent>(ArtRoot, TEXT("R13_CentralParkCanopyRoot"));
    if (!Root)
    {
        ArtRoot->Destroy();
        return;
    }
    Root->SetMobility(EComponentMobility::Static);
    ArtRoot->SetRootComponent(Root);
    ArtRoot->AddInstanceComponent(Root);
    Root->RegisterComponent();

    TArray<UInstancedStaticMeshComponent*> Trees;
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(TreeMeshes); ++Index)
    {
        if (UInstancedStaticMeshComponent* Component = MakeTreeISM(ArtRoot, Root, TreeMeshes[Index],
            FName(*FString::Printf(TEXT("R13_CentralParkCanopy%02d"), Index + 1))))
        {
            Trees.Add(Component);
        }
    }
    if (Trees.IsEmpty())
    {
        ArtRoot->Destroy();
        return;
    }

    // Seeds ring the park and sit behind secondary alleys. Keep the memorial center, primary cross-axis and
    // south-east skate pad deliberately open. A few conifers echo the mixed Soviet-era planting without dominating it.
    const FTreeSeed Seeds[] = {
        { FVector(-9300,-6600,0), 1880,  14,0 }, { FVector(-6900,-7100,0), 1720,  61,1 },
        { FVector(-4200,-7250,0), 1810, 103,2 }, { FVector(-1200,-7350,0), 1960, 151,3 },
        { FVector( 2100,-7300,0), 1760, 208,0 }, { FVector(-9500,-3400,0), 1840, 257,1 },
        { FVector(-9700, -500,0), 2050, 311,4 }, { FVector(-9550, 2900,0), 1770,  33,2 },
        { FVector(-9000, 6400,0), 1900,  79,0 }, { FVector(-6250, 7100,0), 1830, 126,1 },
        { FVector(-3500, 7300,0), 1740, 174,3 }, { FVector( -900, 7200,0), 1980, 222,2 },
        { FVector( 2500, 7200,0), 1800, 271,0 }, { FVector( 5400, 7000,0), 1920, 318,4 },
        { FVector( 8800, 6300,0), 1790,  41,1 }, { FVector( 9550, 3300,0), 1870,  88,2 },
        { FVector( 9700,  400,0), 2020, 137,0 }, { FVector( 9300,-2100,0), 1760, 183,3 },
        { FVector(-7200, 3600,0), 1660, 231,1 }, { FVector( 6600, 4200,0), 1710, 284,2 },
    };

    const FVector Park = AOCWorldSectorOster::ParkAnchor();
    int32 Added = 0;
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(Seeds); ++Index)
    {
        const FTreeSeed& Seed = Seeds[Index];
        UInstancedStaticMeshComponent* Target = Trees[Seed.Variant % Trees.Num()];
        AddHeightMatched(Target, Park + Seed.Offset, Seed.HeightCm, Seed.Yaw);
        ++Added;
    }

    bApplied = Added > 0;
    UE_LOG(LogTemp, Display,
        TEXT("R13.5 central park canopy: mature visual trees=%d; memorial plaza, primary alleys and skate pad kept clear."),
        Added);
}
