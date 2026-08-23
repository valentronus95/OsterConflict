#include "OCMuseumVisibilityPass37Subsystem.h"

#include "OCGameMode.h"
#include "OCR138MuseumInteractiveArchitectureSubsystem.h"
#include "OCWorldSectorOster.h"

#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"

namespace
{
    constexpr float FirstPollDelaySeconds = 0.85f;
    constexpr float PollIntervalSeconds = 0.35f;
    constexpr float LateStartupSettleSeconds = 6.40f;
    constexpr int32 MaxPollCount = 24;
    constexpr int32 MinVisibleStructuralComponents = 12;
    constexpr float MuseumStructureRadiusCm = 2600.0f;

    const FName MuseumPrototypeTag(TEXT("R137_MuseumPhotoModel"));
    const FName MuseumArchitectureTag(TEXT("R138_MuseumHighFidelityArchitecture"));
    const FName MuseumStructuralTag(TEXT("MuseumStructural"));

    struct FArchitectureSnapshot
    {
        AActor* BestActor = nullptr;
        int32 OwnerCount = 0;
        int32 TotalStructural = 0;
        int32 VisibleStructural = 0;
        int32 VisibleNearAnchor = 0;
    };

    bool HasPrototypeOwner(UWorld& World)
    {
        for (TActorIterator<AActor> It(&World); It; ++It)
        {
            const AActor* Actor = *It;
            if (Actor && !Actor->IsActorBeingDestroyed() && Actor->ActorHasTag(MuseumPrototypeTag)) return true;
        }
        return false;
    }

    int32 CountActorVisibleStructural(AActor& Actor, int32& OutTotal, int32& OutNearAnchor)
    {
        OutTotal = 0;
        OutNearAnchor = 0;
        int32 Visible = 0;
        const FVector Museum = AOCWorldSectorOster::MuseumAnchor();

        TInlineComponentArray<UPrimitiveComponent*> Components;
        Actor.GetComponents(Components);
        for (UPrimitiveComponent* Component : Components)
        {
            if (!IsValid(Component) || !Component->ComponentHasTag(MuseumStructuralTag)) continue;
            ++OutTotal;

            const bool bVisible = Component->IsRegistered() && Component->IsVisible() && !Component->bHiddenInGame;
            if (!bVisible) continue;
            ++Visible;

            if (FVector::DistSquared2D(Component->Bounds.Origin, Museum) <= FMath::Square(MuseumStructureRadiusCm))
            {
                ++OutNearAnchor;
            }
        }
        return Visible;
    }

    FArchitectureSnapshot SnapshotArchitecture(UWorld& World)
    {
        FArchitectureSnapshot Snapshot;
        int32 BestNear = -1;
        int32 BestVisible = -1;

        for (TActorIterator<AActor> It(&World); It; ++It)
        {
            AActor* Actor = *It;
            if (!Actor || Actor->IsActorBeingDestroyed() || !Actor->ActorHasTag(MuseumArchitectureTag)) continue;
            ++Snapshot.OwnerCount;

            int32 Total = 0;
            int32 Near = 0;
            const int32 Visible = CountActorVisibleStructural(*Actor, Total, Near);
            Snapshot.TotalStructural += Total;
            Snapshot.VisibleStructural += Visible;
            Snapshot.VisibleNearAnchor += Near;

            if (Near > BestNear || (Near == BestNear && Visible > BestVisible))
            {
                BestNear = Near;
                BestVisible = Visible;
                Snapshot.BestActor = Actor;
            }
        }
        return Snapshot;
    }

    void ForceStructuralVisibility(AActor& Actor)
    {
        Actor.SetActorHiddenInGame(false);
        TInlineComponentArray<UPrimitiveComponent*> Components;
        Actor.GetComponents(Components);
        for (UPrimitiveComponent* Component : Components)
        {
            if (!IsValid(Component) || !Component->ComponentHasTag(MuseumStructuralTag)) continue;
            Component->SetVisibility(true, true);
            Component->SetHiddenInGame(false, true);
            Component->MarkRenderStateDirty();
        }
    }

    int32 RetireOtherArchitectureOwners(UWorld& World, AActor* Keep)
    {
        TArray<AActor*> Duplicates;
        for (TActorIterator<AActor> It(&World); It; ++It)
        {
            AActor* Actor = *It;
            if (!Actor || Actor == Keep || Actor->IsActorBeingDestroyed() || !Actor->ActorHasTag(MuseumArchitectureTag)) continue;
            Duplicates.Add(Actor);
        }
        for (AActor* Actor : Duplicates)
        {
            Actor->Destroy();
        }
        return Duplicates.Num();
    }

    int32 RetireAllArchitectureOwners(UWorld& World)
    {
        return RetireOtherArchitectureOwners(World, nullptr);
    }
}

bool UOCMuseumVisibilityPass37Subsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCMuseumVisibilityPass37Subsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    if (InWorld.GetNetMode() == NM_DedicatedServer) return;
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    InWorld.GetTimerManager().SetTimer(
        VisibilityPollTimer,
        this,
        &UOCMuseumVisibilityPass37Subsystem::ValidateVisibleMuseum,
        PollIntervalSeconds,
        true,
        FirstPollDelaySeconds);
}

void UOCMuseumVisibilityPass37Subsystem::Deinitialize()
{
    if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(VisibilityPollTimer);
    Super::Deinitialize();
}

void UOCMuseumVisibilityPass37Subsystem::ValidateVisibleMuseum()
{
    UWorld* World = GetWorld();
    if (!World) return;

    ++PollCount;
    ElapsedPollSeconds += PollIntervalSeconds;

    FArchitectureSnapshot Snapshot = SnapshotArchitecture(*World);
    const bool bVisibleCoreReady = Snapshot.BestActor &&
        Snapshot.VisibleNearAnchor >= MinVisibleStructuralComponents;

    if (!bVisibleCoreReady && HasPrototypeOwner(*World))
    {
        const int32 Retired = RetireAllArchitectureOwners(*World);
        if (UOCR138MuseumInteractiveArchitectureSubsystem* Architecture =
            World->GetSubsystem<UOCR138MuseumInteractiveArchitectureSubsystem>())
        {
            Architecture->RunAuthoritativeUpgradeNow(*World);
            bRebuildAttempted = true;
            UE_LOG(LogTemp, Warning,
                TEXT("PASS37_MUSEUM_VISIBLE_CORE_REBUILD retired_owners=%d prior_visible=%d prior_near_anchor=%d"),
                Retired, Snapshot.VisibleStructural, Snapshot.VisibleNearAnchor);
        }

        Snapshot = SnapshotArchitecture(*World);
    }

    if (Snapshot.BestActor && Snapshot.VisibleNearAnchor >= MinVisibleStructuralComponents)
    {
        ForceStructuralVisibility(*Snapshot.BestActor);
        const int32 RetiredDuplicates = RetireOtherArchitectureOwners(*World, Snapshot.BestActor);
        if (RetiredDuplicates > 0)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("PASS37_MUSEUM_DUPLICATE_ARCHITECTURE_RETIRED count=%d"), RetiredDuplicates);
        }

        // Keep polling through the historical R13.8 5.35 s delayed startup. It can otherwise create
        // a second owner after an early recovery and recreate the old building-inside-building problem.
        if (ElapsedPollSeconds >= LateStartupSettleSeconds)
        {
            const FArchitectureSnapshot FinalSnapshot = SnapshotArchitecture(*World);
            if (FinalSnapshot.OwnerCount == 1 &&
                FinalSnapshot.VisibleNearAnchor >= MinVisibleStructuralComponents)
            {
                World->GetTimerManager().ClearTimer(VisibilityPollTimer);
                UE_LOG(LogTemp, Display,
                    TEXT("PASS37_MUSEUM_VISIBLE_CORE_READY owners=%d structural_total=%d structural_visible=%d near_anchor=%d min_required=%d rebuilt=%d anchor=%s"),
                    FinalSnapshot.OwnerCount,
                    FinalSnapshot.TotalStructural,
                    FinalSnapshot.VisibleStructural,
                    FinalSnapshot.VisibleNearAnchor,
                    MinVisibleStructuralComponents,
                    bRebuildAttempted ? 1 : 0,
                    *AOCWorldSectorOster::MuseumAnchor().ToCompactString());
                return;
            }
        }
    }

    if (PollCount >= MaxPollCount)
    {
        World->GetTimerManager().ClearTimer(VisibilityPollTimer);
        const FArchitectureSnapshot FinalSnapshot = SnapshotArchitecture(*World);
        UE_LOG(LogTemp, Error,
            TEXT("PASS37_MUSEUM_VISIBLE_CORE_FAIL owners=%d structural_total=%d structural_visible=%d near_anchor=%d min_required=%d prototype=%d"),
            FinalSnapshot.OwnerCount,
            FinalSnapshot.TotalStructural,
            FinalSnapshot.VisibleStructural,
            FinalSnapshot.VisibleNearAnchor,
            MinVisibleStructuralComponents,
            HasPrototypeOwner(*World) ? 1 : 0);
    }
}
