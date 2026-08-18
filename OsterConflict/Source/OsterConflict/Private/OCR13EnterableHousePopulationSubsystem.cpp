#include "OCR13EnterableHousePopulationSubsystem.h"

#include "OCEnterableHouse.h"
#include "OCGameMode.h"
#include "OCHouseTypes.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

namespace
{
    constexpr float PopulationDelaySeconds = 1.62f;
    constexpr int32 MaxAdditionalEnterableHouses = 3;
    constexpr float HouseClearanceCm = 4700.0f;
    constexpr float RoadsideOffsetCm = 3550.0f;

    UInstancedStaticMeshComponent* FindISM(AActor* Actor, const FName Name)
    {
        return Actor ? FindObjectFast<UInstancedStaticMeshComponent>(Actor, Name) : nullptr;
    }

    bool IsInsideCompactPlayableBounds(const FVector& Location)
    {
        return Location.X >= -66000.0f && Location.X <= 21000.0f &&
            Location.Y >= -21000.0f && Location.Y <= 46000.0f;
    }

    bool IsNearLandmark(const FVector& Location)
    {
        struct FProtectedAnchor
        {
            FVector Location;
            float RadiusCm;
        };

        const FProtectedAnchor Protected[] = {
            { AOCWorldSectorOster::MuseumAnchor(), 9800.0f },
            { AOCWorldSectorOster::StadiumAnchor(), 9800.0f },
            { AOCWorldSectorOster::ParkAnchor(), 13500.0f },
            { AOCWorldSectorOster::CollegeAnchor(), 13500.0f },
            { AOCWorldSectorOster::KrushelnytskaEnterableHouseAnchor(), 6200.0f },
        };

        for (const FProtectedAnchor& Anchor : Protected)
        {
            if (FVector::DistSquared2D(Location, Anchor.Location) < FMath::Square(Anchor.RadiusCm)) return true;
        }
        return false;
    }

    void GatherStructuralHouseLocations(UWorld& World, TArray<FVector>& OutLocations)
    {
        for (TActorIterator<AActor> It(&World); It; ++It)
        {
            AActor* Actor = *It;
            if (!Actor) continue;

            TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
            Actor->GetComponents(Components);
            for (UInstancedStaticMeshComponent* Component : Components)
            {
                if (!Component) continue;
                const FName Name = Component->GetFName();
                if (Name != TEXT("R13_House01") && Name != TEXT("R13_House02")) continue;

                for (int32 Index = 0; Index < Component->GetInstanceCount(); ++Index)
                {
                    FTransform Transform;
                    if (Component->GetInstanceTransform(Index, Transform, true))
                    {
                        OutLocations.Add(Transform.GetLocation());
                    }
                }
            }
        }

        for (TActorIterator<AOCEnterableHouse> It(&World); It; ++It)
        {
            if (AOCEnterableHouse* House = *It) OutLocations.Add(House->GetActorLocation());
        }
    }

    bool HasHouseClearance(const FVector& Candidate, const TArray<FVector>& Existing)
    {
        for (const FVector& Location : Existing)
        {
            if (FVector::DistSquared2D(Candidate, Location) < FMath::Square(HouseClearanceCm)) return false;
        }
        return true;
    }
}

bool UOCR13EnterableHousePopulationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13EnterableHousePopulationSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (InWorld.GetNetMode() == NM_Client) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) PopulateEnterableHouses(*World);
        }), PopulationDelaySeconds, false);
}

void UOCR13EnterableHousePopulationSubsystem::PopulateEnterableHouses(UWorld& World)
{
    if (bApplied || World.GetNetMode() == NM_Client) return;

    AOCWorldSectorOster* Sector = nullptr;
    for (TActorIterator<AOCWorldSectorOster> It(&World); It; ++It)
    {
        Sector = *It;
        if (Sector) break;
    }
    if (!Sector) return;

    UInstancedStaticMeshComponent* Roads = FindISM(Sector, TEXT("Roads"));
    if (!Roads || Roads->GetInstanceCount() <= 0) return;

    TArray<FVector> ExistingHouseLocations;
    GatherStructuralHouseLocations(World, ExistingHouseLocations);

    int32 ExistingEnterableCount = 0;
    for (TActorIterator<AOCEnterableHouse> It(&World); It; ++It) ++ExistingEnterableCount;

    int32 Added = 0;
    for (int32 RoadIndex = 0;
        RoadIndex < Roads->GetInstanceCount() && Added < MaxAdditionalEnterableHouses;
        ++RoadIndex)
    {
        FTransform RoadTransform;
        if (!Roads->GetInstanceTransform(RoadIndex, RoadTransform, true)) continue;

        const FVector RoadScale = RoadTransform.GetScale3D().GetAbs();
        const bool bLongX = RoadScale.X >= RoadScale.Y;
        const float RoadLengthCm = (bLongX ? RoadScale.X : RoadScale.Y) * 100.0f;
        const float RoadWidthCm = (bLongX ? RoadScale.Y : RoadScale.X) * 100.0f;
        if (RoadLengthCm < 17000.0f || RoadWidthCm > 1800.0f) continue;

        const FQuat RoadRotation = RoadTransform.GetRotation();
        const FVector AlongAxis = RoadRotation.RotateVector(
            bLongX ? FVector::ForwardVector : FVector::RightVector).GetSafeNormal2D();
        FVector SideAxis(-AlongAxis.Y, AlongAxis.X, 0.0f);
        const float HouseYaw = FMath::RadiansToDegrees(FMath::Atan2(AlongAxis.Y, AlongAxis.X));

        // Deterministic two-side/two-along candidates. Runtime spacing decides whether a gap is actually usable.
        for (int32 Attempt = 0; Attempt < 4 && Added < MaxAdditionalEnterableHouses; ++Attempt)
        {
            const float AlongSign = (Attempt < 2) ? -1.0f : 1.0f;
            const float SideSign = (Attempt % 2 == 0) ? 1.0f : -1.0f;
            const float AlongDistance = FMath::Min(RoadLengthCm * 0.28f, 7600.0f);
            FVector Candidate = RoadTransform.GetLocation() + AlongAxis * AlongDistance * AlongSign +
                SideAxis * RoadsideOffsetCm * SideSign;
            Candidate.Z = 0.0f;

            if (!IsInsideCompactPlayableBounds(Candidate) || IsNearLandmark(Candidate) ||
                !HasHouseClearance(Candidate, ExistingHouseLocations))
            {
                continue;
            }

            // Front wall is local -Y. Flip 180 degrees when the house is on the opposite side so the front yard
            // faces the road rather than the back wall facing traffic.
            const float FacingYaw = HouseYaw + (SideSign > 0.0f ? 0.0f : 180.0f);

            FActorSpawnParameters Params;
            Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
            AOCEnterableHouse* House = World.SpawnActor<AOCEnterableHouse>(
                AOCEnterableHouse::StaticClass(), Candidate, FRotator(0.0f, FacingYaw, 0.0f), Params);
            if (!House) continue;

            const int32 Seed = 2401 + Added * 977 + RoadIndex * 13;
            const EOCHouseCondition Condition = Added == 0 ? EOCHouseCondition::Ordinary
                : Added == 1 ? EOCHouseCondition::Worn : EOCHouseCondition::Maintained;
            House->ConfigureInteriorVariantServer(Seed, Condition, (ExistingEnterableCount + Added) % 6);

            ExistingHouseLocations.Add(House->GetActorLocation());
            ++Added;
        }
    }

    bApplied = true;
    UE_LOG(LogTemp, Display,
        TEXT("R13.4 enterable-house population: added %d/%d houses after road/compact/landmark/house-clearance validation; existing=%d."),
        Added, MaxAdditionalEnterableHouses, ExistingEnterableCount);
}
