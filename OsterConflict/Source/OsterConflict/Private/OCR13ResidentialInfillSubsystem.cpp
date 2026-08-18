#include "OCR13ResidentialInfillSubsystem.h"

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
    constexpr float InfillDelaySeconds = 1.20f;
    constexpr int32 MaxInfillHouses = 18;
    constexpr float CandidateSpacingCm = 8200.0f;
    constexpr float ExistingHouseClearanceCm = 3900.0f;
    constexpr float NewHouseClearanceCm = 5600.0f;

    struct FHouseFamily
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

    UInstancedStaticMeshComponent* MakeHouseISM(AActor* Owner, USceneComponent* Root,
        UStaticMesh* Mesh, const FName Name)
    {
        if (!Owner || !Root || !Mesh) return nullptr;
        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, Name);
        if (!Component) return nullptr;
        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionProfileName(TEXT("BlockAll"));
        Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(true);
        Component->SetCastShadow(true);
        Component->SetCullDistances(0, 110000);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    bool IsInsideCompactBounds(const FVector& P)
    {
        return P.X >= -66500.0f && P.X <= 21500.0f && P.Y >= -21500.0f && P.Y <= 46500.0f;
    }

    bool IsInsideKrushelnytskaReserved(const FVector& P)
    {
        const bool bGameplaySlice = FMath::Abs(P.X + 3400.0f) < 8200.0f && P.Y > -15500.0f && P.Y < 18500.0f;
        const bool bGeoCorridor = FMath::Abs(P.X + 33500.0f) < 6800.0f && P.Y > -17000.0f && P.Y < 48000.0f;
        return bGameplaySlice || bGeoCorridor;
    }

    bool IsNearLandmark(const FVector& P)
    {
        struct FReservedLandmark
        {
            FVector Center;
            float RadiusCm;
        };
        const FReservedLandmark Reserved[] = {
            { AOCWorldSectorOster::MuseumAnchor(), 11200.0f },
            { AOCWorldSectorOster::StadiumAnchor(), 12500.0f },
            { AOCWorldSectorOster::ParkAnchor(), 13500.0f },
            { AOCWorldSectorOster::CollegeAnchor(), 10800.0f },
        };

        for (const FReservedLandmark& Landmark : Reserved)
        {
            if (FVector::DistSquared2D(P, Landmark.Center) <= FMath::Square(Landmark.RadiusCm)) return true;
        }
        return false;
    }

    bool HasNearbySourceBuilding(UInstancedStaticMeshComponent* Buildings, const FVector& Candidate)
    {
        if (!Buildings) return false;
        const float LimitSq = FMath::Square(ExistingHouseClearanceCm);
        for (int32 Index = 0; Index < Buildings->GetInstanceCount(); ++Index)
        {
            FTransform Transform;
            if (!Buildings->GetInstanceTransform(Index, Transform, true)) continue;
            if (FVector::DistSquared2D(Candidate, Transform.GetLocation()) < LimitSq) return true;
        }
        return false;
    }

    bool HasNearbyNewBuilding(const TArray<FVector>& Accepted, const FVector& Candidate)
    {
        const float LimitSq = FMath::Square(NewHouseClearanceCm);
        for (const FVector& Existing : Accepted)
        {
            if (FVector::DistSquared2D(Candidate, Existing) < LimitSq) return true;
        }
        return false;
    }

    void AddGroundedHouse(const FHouseFamily& Family, FVector Location, const float Yaw, const float Scale)
    {
        if (!Family.Component || !Family.Mesh) return;
        const FBoxSphereBounds Bounds = Family.Mesh->GetBounds();
        const float LocalBottom = Bounds.Origin.Z - Bounds.BoxExtent.Z;
        Location.Z = -LocalBottom * Scale;
        Family.Component->AddInstance(FTransform(FRotator(0.0f, Yaw, 0.0f), Location, FVector(Scale)), true);
    }

    void AddHiddenSourceFootprint(UInstancedStaticMeshComponent* Buildings, UStaticMesh* HouseMesh,
        const FVector& Location, const float Yaw, const float HouseScale)
    {
        if (!Buildings || !HouseMesh) return;

        // WholeOster has already replaced/hid the cube building proxy before this 1.20 s pass. Keep it hidden and use
        // extra instances only as metadata footprints: EnvironmentDressing reads Buildings at 1.60 s for grass exclusion,
        // while ResidentialYard later reuses the same transform for unique yard props. Nothing here is rendered/colliding.
        Buildings->SetVisibility(false, true);
        Buildings->SetCollisionEnabled(ECollisionEnabled::NoCollision);

        const FVector MeshSize = HouseMesh->GetBounds().BoxExtent * 2.0f;
        const FVector FootprintScale(
            FMath::Max(3.0f, MeshSize.X * HouseScale / 100.0f),
            FMath::Max(3.0f, MeshSize.Y * HouseScale / 100.0f),
            1.0f);
        Buildings->AddInstance(FTransform(FRotator(0.0f, Yaw, 0.0f),
            FVector(Location.X, Location.Y, 0.0f), FootprintScale), true);
    }

    void AddFamilyIfValid(TArray<FHouseFamily>& Families, AActor* Owner, USceneComponent* Root,
        UStaticMesh* Mesh, const FName ComponentName)
    {
        if (!Mesh) return;
        FHouseFamily Family;
        Family.Component = MakeHouseISM(Owner, Root, Mesh, ComponentName);
        Family.Mesh = Mesh;
        if (Family.Component) Families.Add(Family);
    }
}

bool UOCR13ResidentialInfillSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13ResidentialInfillSubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
            if (UWorld* World = WeakWorld.Get()) BuildResidentialInfill(*World);
        }), InfillDelaySeconds, false);
}

void UOCR13ResidentialInfillSubsystem::BuildResidentialInfill(UWorld& World)
{
    if (bApplied) return;

    AOCWorldSectorOster* WorldSector = nullptr;
    for (TActorIterator<AOCWorldSectorOster> It(&World); It; ++It)
    {
        WorldSector = *It;
        if (WorldSector) break;
    }
    if (!WorldSector) return;

    UInstancedStaticMeshComponent* Roads = FindISM(WorldSector, TEXT("Roads"));
    UInstancedStaticMeshComponent* Buildings = FindISM(WorldSector, TEXT("Buildings"));
    if (!Roads || !Buildings) return;

    UStaticMesh* House01 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_House_Var01.SM_House_Var01"));
    UStaticMesh* House02 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_House_Var02.SM_House_Var02"));
    if (!House01 && !House02) return;

    AActor* ArtRoot = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!ArtRoot) return;
    ArtRoot->SetReplicates(false);
    ArtRoot->SetActorEnableCollision(true);

    USceneComponent* Root = NewObject<USceneComponent>(ArtRoot, TEXT("R13_ResidentialInfillRoot"));
    if (!Root)
    {
        ArtRoot->Destroy();
        return;
    }
    Root->SetMobility(EComponentMobility::Static);
    ArtRoot->SetRootComponent(Root);
    ArtRoot->AddInstanceComponent(Root);
    Root->RegisterComponent();

    TArray<FHouseFamily> Families;
    // Canonical component names intentionally match WholeOster. EnvironmentDressing scans those names at 1.60 s,
    // so infill houses automatically receive the same companion house modules and base yard dressing as source houses.
    AddFamilyIfValid(Families, ArtRoot, Root, House01, TEXT("R13_House01"));
    AddFamilyIfValid(Families, ArtRoot, Root, House02, TEXT("R13_House02"));
    if (Families.IsEmpty())
    {
        ArtRoot->Destroy();
        return;
    }

    TArray<FVector> AcceptedLocations;
    int32 ConsideredCandidates = 0;

    for (int32 RoadIndex = 0; RoadIndex < Roads->GetInstanceCount() && AcceptedLocations.Num() < MaxInfillHouses; ++RoadIndex)
    {
        FTransform RoadTransform;
        if (!Roads->GetInstanceTransform(RoadIndex, RoadTransform, true)) continue;

        const FVector RawScale = RoadTransform.GetScale3D();
        const FVector Scale(FMath::Abs(RawScale.X), FMath::Abs(RawScale.Y), FMath::Abs(RawScale.Z));
        const float RoadLengthCm = Scale.X * 100.0f;
        const float RoadWidthCm = Scale.Y * 100.0f;
        if (RoadLengthCm < 16000.0f || RoadWidthCm > 1800.0f) continue;

        const int32 Samples = FMath::Clamp(FMath::FloorToInt(RoadLengthCm / CandidateSpacingCm), 2, 12);
        const float SideOffsetCm = FMath::Clamp(RoadWidthCm * 0.5f + 2450.0f, 2850.0f, 3600.0f);
        const FQuat RoadRotation(FRotator(0.0f, RoadTransform.Rotator().Yaw, 0.0f));

        for (int32 Sample = 0; Sample < Samples && AcceptedLocations.Num() < MaxInfillHouses; ++Sample)
        {
            const float Alpha = (static_cast<float>(Sample) + 0.5f) / static_cast<float>(Samples);
            const float Along = FMath::Lerp(-RoadLengthCm * 0.44f, RoadLengthCm * 0.44f, Alpha);
            const float SideSign = ((RoadIndex + Sample) % 2 == 0) ? 1.0f : -1.0f;
            const FVector Candidate = RoadTransform.GetLocation() +
                RoadRotation.RotateVector(FVector(Along, SideSign * SideOffsetCm, 0.0f));
            ++ConsideredCandidates;

            if (!IsInsideCompactBounds(Candidate)) continue;
            if (IsInsideKrushelnytskaReserved(Candidate)) continue;
            if (IsNearLandmark(Candidate)) continue;
            if (HasNearbySourceBuilding(Buildings, Candidate)) continue;
            if (HasNearbyNewBuilding(AcceptedLocations, Candidate)) continue;

            const int32 Variant = AcceptedLocations.Num();
            const float HouseYaw = RoadTransform.Rotator().Yaw +
                (SideSign > 0.0f ? 0.0f : 180.0f) + static_cast<float>((Variant % 5) - 2) * 1.7f;
            const float HouseScale = 0.88f + 0.035f * static_cast<float>(Variant % 4);
            const FHouseFamily& Family = Families[Variant % Families.Num()];
            AddGroundedHouse(Family, Candidate, HouseYaw, HouseScale);
            AddHiddenSourceFootprint(Buildings, Family.Mesh, Candidate, HouseYaw, HouseScale);
            AcceptedLocations.Add(Candidate);
        }
    }

    bApplied = true;
    UE_LOG(LogTemp, Display,
        TEXT("R13.4 residential infill: accepted=%d/%d candidates, cap=%d; canonical house families feed EnvironmentDressing and hidden source footprints reserve grass/yard space while placement stays clear of source houses, landmarks and Krushelnytska."),
        AcceptedLocations.Num(), ConsideredCandidates, MaxInfillHouses);
}