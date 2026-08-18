#include "OCR13CivicLandscapingSubsystem.h"

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
    constexpr float CivicDelaySeconds = 2.05f;

    UStaticMesh* LoadCivicMesh(const TCHAR* Path)
    {
        return LoadObject<UStaticMesh>(nullptr, Path);
    }

    UInstancedStaticMeshComponent* MakeVisualISM(AActor* Owner, USceneComponent* Root,
        UStaticMesh* Mesh, const FName Name, const bool bCastShadow, const int32 CullEndCm)
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
        Component->SetCastShadow(bCastShadow);
        if (CullEndCm > 0) Component->SetCullDistances(0, CullEndCm);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    void AddHeightFitted(UInstancedStaticMeshComponent* Target, FVector GroundLocation,
        const float DesiredHeightCm, const float YawDegrees)
    {
        if (!Target || !Target->GetStaticMesh()) return;
        const FBoxSphereBounds Bounds = Target->GetStaticMesh()->GetBounds();
        const FVector Size = Bounds.BoxExtent * 2.0f;
        if (Size.Z <= 1.0f) return;
        const float Scale = FMath::Clamp(DesiredHeightCm / Size.Z, 0.18f, 4.0f);
        const FQuat Rotation = FRotator(0.0f, YawDegrees, 0.0f).Quaternion();
        GroundLocation.Z = -((Bounds.Origin.Z - Bounds.BoxExtent.Z) * Scale);
        Target->AddInstance(FTransform(Rotation, GroundLocation, FVector(Scale)), true);
    }

    void AddMuseumGarden(const FVector& Museum,
        const TArray<UInstancedStaticMeshComponent*>& Trees,
        const TArray<UInstancedStaticMeshComponent*>& Shrubs)
    {
        // Photo-reference subsystem already owns the characteristic mature pines and facade. This civic pass adds
        // the broader deciduous/shrub frame visible around the museum without blocking the entrance axis at X~1180.
        const FVector TreeOffsets[] = {
            FVector(-5600, -1800, 0), FVector(-4700, 3500, 0), FVector(-2500, 5200, 0),
            FVector(2550, 5000, 0), FVector(5250, 2600, 0), FVector(6100, -1200, 0),
            FVector(-6100, -5400, 0), FVector(6200, -5700, 0),
        };
        for (int32 Index = 0; Index < UE_ARRAY_COUNT(TreeOffsets); ++Index)
        {
            if (Trees.IsEmpty()) break;
            UInstancedStaticMeshComponent* Target = Trees[Index % Trees.Num()];
            AddHeightFitted(Target, Museum + TreeOffsets[Index],
                1450.0f + 115.0f * static_cast<float>(Index % 4), 31.0f * Index + 13.0f);
        }

        const FVector ShrubOffsets[] = {
            FVector(-3300, -2350, 0), FVector(-4100, 650, 0), FVector(-3350, 3000, 0),
            FVector(3150, 3100, 0), FVector(4100, 850, 0), FVector(3500, -2100, 0),
        };
        for (int32 Index = 0; Index < UE_ARRAY_COUNT(ShrubOffsets); ++Index)
        {
            if (Shrubs.IsEmpty()) break;
            AddHeightFitted(Shrubs[Index % Shrubs.Num()], Museum + ShrubOffsets[Index],
                95.0f + 14.0f * static_cast<float>(Index % 3), 47.0f * Index);
        }
    }

    void AddCollegeCampusPlanting(const FVector& College,
        const TArray<UInstancedStaticMeshComponent*>& Trees,
        const TArray<UInstancedStaticMeshComponent*>& Shrubs)
    {
        // Main entrance/broad stairs stay clear. Planting frames the side/back campus edges only.
        const FVector TreeOffsets[] = {
            FVector(-5400, -2500, 0), FVector(-5650, 650, 0), FVector(-5200, 3800, 0),
            FVector(5200, 3900, 0), FVector(5550, 900, 0), FVector(5350, -2450, 0),
            FVector(-1800, 5200, 0), FVector(2100, 5250, 0),
        };
        for (int32 Index = 0; Index < UE_ARRAY_COUNT(TreeOffsets); ++Index)
        {
            if (Trees.IsEmpty()) break;
            AddHeightFitted(Trees[(Index + 1) % Trees.Num()], College + TreeOffsets[Index],
                1250.0f + 95.0f * static_cast<float>(Index % 4), 39.0f * Index + 9.0f);
        }

        const FVector ShrubOffsets[] = {
            FVector(-3600, -3200, 0), FVector(-4300, 2100, 0), FVector(-2500, 4300, 0),
            FVector(2750, 4350, 0), FVector(4350, 2200, 0), FVector(3650, -3150, 0),
        };
        for (int32 Index = 0; Index < UE_ARRAY_COUNT(ShrubOffsets); ++Index)
        {
            if (Shrubs.IsEmpty()) break;
            AddHeightFitted(Shrubs[(Index + 1) % Shrubs.Num()], College + ShrubOffsets[Index],
                80.0f + 12.0f * static_cast<float>(Index % 3), 53.0f * Index);
        }
    }

    void AddStadiumPerimeterPlanting(const FVector& Stadium,
        const TArray<UInstancedStaticMeshComponent*>& Trees,
        const TArray<UInstancedStaticMeshComponent*>& Shrubs)
    {
        // Keep pitch, running track, goal mouths and central approaches fully clear. Greenery sits beyond spectator edges.
        const FVector TreeOffsets[] = {
            FVector(-7600, -6200, 0), FVector(-7600, -1800, 0), FVector(-7500, 3000, 0),
            FVector(-5200, 7200, 0), FVector(-800, 7350, 0), FVector(3700, 7200, 0),
            FVector(7600, 3500, 0), FVector(7700, -1600, 0), FVector(7550, -6100, 0),
        };
        for (int32 Index = 0; Index < UE_ARRAY_COUNT(TreeOffsets); ++Index)
        {
            if (Trees.IsEmpty()) break;
            AddHeightFitted(Trees[(Index + 2) % Trees.Num()], Stadium + TreeOffsets[Index],
                1350.0f + 85.0f * static_cast<float>(Index % 5), 29.0f * Index + 17.0f);
        }

        const FVector ShrubOffsets[] = {
            FVector(-6800, -6900, 0), FVector(-6800, 6100, 0), FVector(-3000, 7150, 0),
            FVector(3000, 7150, 0), FVector(6850, 6100, 0), FVector(6900, -6850, 0),
        };
        for (int32 Index = 0; Index < UE_ARRAY_COUNT(ShrubOffsets); ++Index)
        {
            if (Shrubs.IsEmpty()) break;
            AddHeightFitted(Shrubs[Index % Shrubs.Num()], Stadium + ShrubOffsets[Index],
                90.0f + 12.0f * static_cast<float>(Index % 4), 61.0f * Index);
        }
    }
}

bool UOCR13CivicLandscapingSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13CivicLandscapingSubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
            if (UWorld* World = WeakWorld.Get()) ApplyCivicLandscaping(*World);
        }), CivicDelaySeconds, false);
}

void UOCR13CivicLandscapingSubsystem::ApplyCivicLandscaping(UWorld& World)
{
    if (bApplied) return;

    UStaticMesh* TreeMeshes[] = {
        LoadCivicMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var03.SM_Tree_Var03")),
        LoadCivicMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var04.SM_Tree_Var04")),
        LoadCivicMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var05.SM_Tree_Var05")),
    };
    UStaticMesh* ShrubMeshes[] = {
        LoadCivicMesh(TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/Shrubs_1.Shrubs_1")),
        LoadCivicMesh(TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/Shrubs_1_Single.Shrubs_1_Single")),
        LoadCivicMesh(TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/Bush_1.Bush_1")),
    };

    AActor* ArtRoot = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!ArtRoot) return;
    ArtRoot->SetReplicates(false);
    ArtRoot->SetActorEnableCollision(false);

    USceneComponent* Root = NewObject<USceneComponent>(ArtRoot, TEXT("R13_CivicLandscapingRoot"));
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
    TArray<UInstancedStaticMeshComponent*> Shrubs;
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(TreeMeshes); ++Index)
    {
        if (UInstancedStaticMeshComponent* Component = MakeVisualISM(ArtRoot, Root, TreeMeshes[Index],
            FName(*FString::Printf(TEXT("R13_CivicTree%02d"), Index + 1)), true, 90000))
        {
            Trees.Add(Component);
        }
    }
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(ShrubMeshes); ++Index)
    {
        if (UInstancedStaticMeshComponent* Component = MakeVisualISM(ArtRoot, Root, ShrubMeshes[Index],
            FName(*FString::Printf(TEXT("R13_CivicShrub%02d"), Index + 1)), false, 42000))
        {
            Shrubs.Add(Component);
        }
    }

    if (Trees.IsEmpty() && Shrubs.IsEmpty())
    {
        ArtRoot->Destroy();
        return;
    }

    AddMuseumGarden(AOCWorldSectorOster::MuseumAnchor(), Trees, Shrubs);
    AddCollegeCampusPlanting(AOCWorldSectorOster::CollegeAnchor(), Trees, Shrubs);
    AddStadiumPerimeterPlanting(AOCWorldSectorOster::StadiumAnchor(), Trees, Shrubs);

    bApplied = true;
    UE_LOG(LogTemp, Display,
        TEXT("R13.5 civic landscaping: museum garden + college campus edge + stadium perimeter planted; entrances/pitch/navigation remain clear."));
}
