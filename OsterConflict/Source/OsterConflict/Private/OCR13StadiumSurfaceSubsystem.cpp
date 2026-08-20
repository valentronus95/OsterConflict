#include "OCR13StadiumSurfaceSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"

namespace
{
    constexpr float StadiumDelaySeconds = 2.35f;

    UInstancedStaticMeshComponent* MakeVisualISM(AActor* Owner, USceneComponent* Root,
        UStaticMesh* Mesh, UMaterialInterface* Material, const FName Name, const bool bCastShadow)
    {
        if (!Owner || !Root || !Mesh) return nullptr;
        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, Name);
        if (!Component) return nullptr;
        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        if (Material) Component->SetMaterial(0, Material);
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(false);
        Component->SetCastShadow(bCastShadow);
        Component->SetCullDistances(0, 95000);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    UMaterialInstanceDynamic* MakeColor(AActor* Owner, UMaterialInterface* Base,
        const FName Name, const FLinearColor& Color)
    {
        if (!Owner || !Base) return nullptr;
        UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Base, Owner, Name);
        if (Material) Material->SetVectorParameterValue(TEXT("Color"), Color);
        return Material;
    }

    void AddBox(UInstancedStaticMeshComponent* Target, const FVector& Center,
        const FVector& SizeCm, const FRotator& Rotation = FRotator::ZeroRotator)
    {
        if (!Target) return;
        Target->AddInstance(FTransform(Rotation, Center, SizeCm / 100.0f), true);
    }

    void AddGroundedMesh(UInstancedStaticMeshComponent* Target, UStaticMesh* Mesh,
        FVector Location, const float DesiredLengthCm, const float Yaw)
    {
        if (!Target || !Mesh) return;
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector Size = Bounds.BoxExtent * 2.0f;
        if (Size.X <= 1.0f || Size.Z <= 1.0f) return;
        const float Scale = FMath::Clamp(DesiredLengthCm / Size.X, 0.25f, 4.0f);
        const float LocalBottom = Bounds.Origin.Z - Bounds.BoxExtent.Z;
        Location.Z = -LocalBottom * Scale;
        Target->AddInstance(FTransform(FRotator(0.0f, Yaw, 0.0f), Location, FVector(Scale)), true);
    }
}

bool UOCR13StadiumSurfaceSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13StadiumSurfaceSubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
            if (UWorld* World = WeakWorld.Get()) ApplyStadiumSurface(*World);
        }), StadiumDelaySeconds, false);
}

void UOCR13StadiumSurfaceSubsystem::ApplyStadiumSurface(UWorld& World)
{
    if (bApplied) return;

    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UStaticMesh* BenchMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Old_Planks_Plank_1.Old_Planks_Plank_1"));
    UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!Cube || !BaseMaterial) return;

    AActor* ArtRoot = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!ArtRoot) return;
    ArtRoot->SetReplicates(false);
    ArtRoot->SetActorEnableCollision(false);

    USceneComponent* Root = NewObject<USceneComponent>(ArtRoot, TEXT("R13_StadiumSurfaceRoot"));
    if (!Root)
    {
        ArtRoot->Destroy();
        return;
    }
    Root->SetMobility(EComponentMobility::Static);
    ArtRoot->SetRootComponent(Root);
    ArtRoot->AddInstanceComponent(Root);
    Root->RegisterComponent();

    UMaterialInstanceDynamic* TurfMaterial = MakeColor(ArtRoot, BaseMaterial,
        TEXT("R13_StadiumTurfMat"), FLinearColor(0.13f, 0.28f, 0.085f, 1.0f));
    UMaterialInstanceDynamic* TrackMaterial = MakeColor(ArtRoot, BaseMaterial,
        TEXT("R13_StadiumTrackMat"), FLinearColor(0.22f, 0.105f, 0.075f, 1.0f));
    UMaterialInstanceDynamic* LineMaterial = MakeColor(ArtRoot, BaseMaterial,
        TEXT("R13_StadiumLineMat"), FLinearColor(0.92f, 0.91f, 0.84f, 1.0f));
    UMaterialInstanceDynamic* GoalMaterial = MakeColor(ArtRoot, BaseMaterial,
        TEXT("R13_StadiumGoalMat"), FLinearColor(0.82f, 0.83f, 0.80f, 1.0f));

    UInstancedStaticMeshComponent* Turf = MakeVisualISM(
        ArtRoot, Root, Cube, TurfMaterial, TEXT("R13_StadiumTurf"), false);
    UInstancedStaticMeshComponent* Track = MakeVisualISM(
        ArtRoot, Root, Cube, TrackMaterial, TEXT("R13_StadiumTrack"), false);
    UInstancedStaticMeshComponent* Lines = MakeVisualISM(
        ArtRoot, Root, Cube, LineMaterial, TEXT("R13_StadiumFieldLines"), false);
    UInstancedStaticMeshComponent* Goals = MakeVisualISM(
        ArtRoot, Root, Cube, GoalMaterial, TEXT("R13_StadiumGoalAccents"), true);
    UInstancedStaticMeshComponent* Benches = MakeVisualISM(
        ArtRoot, Root, BenchMesh, nullptr, TEXT("R13_StadiumSpectatorBenches"), true);

    const FVector Stadium = AOCWorldSectorOster::StadiumAnchor();

    // Source-authored sizes: stadium pad 119x82 m and football pitch 105x68 m.
    AddBox(Turf, Stadium + FVector(0.0f, 0.0f, 8.0f), FVector(10440.0f, 6740.0f, 8.0f));

    // Four visual track/apron strips fit inside the 119x82 m stadium pad and stay outside the 105x68 m pitch.
    AddBox(Track, Stadium + FVector(0.0f, 3740.0f, 7.0f), FVector(11800.0f, 650.0f, 7.0f));
    AddBox(Track, Stadium + FVector(0.0f,-3740.0f, 7.0f), FVector(11800.0f, 650.0f, 7.0f));
    AddBox(Track, Stadium + FVector(5560.0f, 0.0f, 7.0f), FVector(650.0f, 6800.0f, 7.0f));
    AddBox(Track, Stadium + FVector(-5560.0f,0.0f, 7.0f), FVector(650.0f, 6800.0f, 7.0f));

    // Touch/goal lines plus center line. Keep them slightly above turf to avoid z-fighting with the source pad.
    constexpr float LineZ = 14.0f;
    constexpr float LineWidth = 12.0f;
    AddBox(Lines, Stadium + FVector(0.0f, 3370.0f, LineZ), FVector(10480.0f, LineWidth, 3.0f));
    AddBox(Lines, Stadium + FVector(0.0f,-3370.0f, LineZ), FVector(10480.0f, LineWidth, 3.0f));
    AddBox(Lines, Stadium + FVector(5230.0f,0.0f, LineZ), FVector(LineWidth, 6750.0f, 3.0f));
    AddBox(Lines, Stadium + FVector(-5230.0f,0.0f, LineZ), FVector(LineWidth, 6750.0f, 3.0f));
    AddBox(Lines, Stadium + FVector(0.0f,0.0f, LineZ), FVector(LineWidth, 6750.0f, 3.0f));

    // Restrained penalty-area markings, enough to read as a real pitch without drawing every regulation detail.
    for (const float Side : { -1.0f, 1.0f })
    {
        const float GoalX = Side * 5220.0f;
        const float BoxInnerX = Side * 3650.0f;
        AddBox(Lines, Stadium + FVector(BoxInnerX,0.0f,LineZ), FVector(LineWidth, 4000.0f, 3.0f));
        AddBox(Lines, Stadium + FVector((GoalX + BoxInnerX) * 0.5f, 1995.0f, LineZ),
            FVector(FMath::Abs(GoalX - BoxInnerX), LineWidth, 3.0f));
        AddBox(Lines, Stadium + FVector((GoalX + BoxInnerX) * 0.5f,-1995.0f, LineZ),
            FVector(FMath::Abs(GoalX - BoxInnerX), LineWidth, 3.0f));

        // Thin visual goal frame placed on the same goal line. Source collision remains authoritative.
        AddBox(Goals, Stadium + FVector(GoalX, -365.0f, 125.0f), FVector(12.0f, 12.0f, 250.0f));
        AddBox(Goals, Stadium + FVector(GoalX,  365.0f, 125.0f), FVector(12.0f, 12.0f, 250.0f));
        AddBox(Goals, Stadium + FVector(GoalX, 0.0f, 245.0f), FVector(12.0f, 742.0f, 12.0f));
    }

    // Small spectator stand on the north side using the same real plank family already validated for park seating.
    if (Benches && BenchMesh)
    {
        int32 SeatIndex = 0;
        for (int32 Row = 0; Row < 3; ++Row)
        {
            for (int32 Col = -3; Col <= 3; ++Col)
            {
                FVector Location = Stadium + FVector(static_cast<float>(Col) * 580.0f,
                    4300.0f + static_cast<float>(Row) * 210.0f, 35.0f + static_cast<float>(Row) * 48.0f);
                AddGroundedMesh(Benches, BenchMesh, Location, 420.0f, 0.0f);
                ++SeatIndex;
            }
        }
    }

    bApplied = true;
    UE_LOG(LogTemp, Display,
        TEXT("R13.5 stadium surface: 105x68 m turf, track apron, field/penalty markings, goal accents and real-plank spectator seating added; source collision preserved."));
}
