#include "OCWorldRenderBudgetPass17Subsystem.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "OCWorldSectorOster.h"

namespace
{
    struct FISMRenderBudget
    {
        const TCHAR* Name;
        int32 StartCullCm;
        int32 EndCullCm;
        bool bCastShadow;
    };

    constexpr FISMRenderBudget Budgets[] =
    {
        { TEXT("Roads"),                  0, 130000, false },
        { TEXT("Sidewalks"),          10000,  70000, false },
        { TEXT("Buildings"),          60000, 130000, true  },
        { TEXT("ResidentialRoofs"),   45000,  95000, false },
        { TEXT("ResidentialDetails"), 10000,  35000, false },
        { TEXT("LandmarkBlocks"),     60000, 130000, true  },
        { TEXT("LandmarkRoofs"),      50000, 100000, false },
        { TEXT("LandmarkWindows"),    10000,  50000, false },
        { TEXT("LandmarkDetails"),    15000,  60000, false },
        { TEXT("Fences"),             10000,  50000, false },
        { TEXT("WoodFences"),         10000,  50000, false },
        { TEXT("MetalFences"),        10000,  50000, false },
        { TEXT("LightSheetFences"),   10000,  50000, false },
        { TEXT("TreeTrunks"),         25000,  70000, false },
        { TEXT("TreeCrowns"),         25000,  70000, false },
        { TEXT("SovietPoplarTrunks"), 25000,  70000, false },
        { TEXT("SovietPoplarCrowns"), 25000,  70000, false },
        { TEXT("BirchTrunks"),        25000,  70000, false },
        { TEXT("BirchCrowns"),        25000,  70000, false },
        { TEXT("PineTrunks"),         25000,  70000, false },
        { TEXT("PineCrowns"),         25000,  70000, false },
        { TEXT("GrassMown"),              0,  35000, false },
        { TEXT("GrassRough"),             0,  35000, false },
        { TEXT("GrassWetland"),           0,  45000, false },
        { TEXT("StadiumGeometry"),         0,  80000, false },
        { TEXT("StadiumDetails"),      10000,  50000, false },
        { TEXT("ParkGeometry"),            0,  80000, false },
        { TEXT("ParkDetails"),         10000,  50000, false },
        { TEXT("Waterways"),               0, 100000, false },
        { TEXT("Bridges"),             40000, 120000, true  },
        { TEXT("ReferenceMarkers"),        0,   5000, false },
    };
}

bool UOCWorldRenderBudgetPass17Subsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    const UWorld* World = Cast<UWorld>(Outer);
    if (!World) return false;
    return World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE;
}

void UOCWorldRenderBudgetPass17Subsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    if (InWorld.GetNetMode() == NM_DedicatedServer)
    {
        return;
    }

    TryApplyBudget();
}

void UOCWorldRenderBudgetPass17Subsystem::TryApplyBudget()
{
    if (bApplied) return;

    UWorld* World = GetWorld();
    if (!World) return;

    AOCWorldSectorOster* Sector = nullptr;
    for (TActorIterator<AOCWorldSectorOster> It(World); It; ++It)
    {
        Sector = *It;
        break;
    }

    if (!Sector)
    {
        ++Attempts;
        if (Attempts >= 20)
        {
            World->GetTimerManager().ClearTimer(RetryHandle);
            UE_LOG(LogTemp, Warning, TEXT("PASS17_WORLD_ISM_BUDGET_NOT_APPLIED sector_missing attempts=%d"), Attempts);
            return;
        }

        if (!World->GetTimerManager().IsTimerActive(RetryHandle))
        {
            World->GetTimerManager().SetTimer(
                RetryHandle,
                this,
                &UOCWorldRenderBudgetPass17Subsystem::TryApplyBudget,
                0.5f,
                true);
        }
        return;
    }

    TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
    Sector->GetComponents(Components);

    int32 TunedCount = 0;
    int32 NavigationDisabledCount = 0;

    for (UInstancedStaticMeshComponent* Component : Components)
    {
        if (!Component) continue;

        const FName ComponentName = Component->GetFName();
        for (const FISMRenderBudget& Budget : Budgets)
        {
            if (ComponentName != FName(Budget.Name)) continue;

            Component->SetCullDistances(Budget.StartCullCm, Budget.EndCullCm);
            Component->SetCastShadow(Budget.bCastShadow);

            // Non-colliding decoration must not participate in dynamic navigation generation.
            if (Component->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
            {
                Component->SetCanEverAffectNavigation(false);
                ++NavigationDisabledCount;
            }

            Component->MarkRenderStateDirty();
            ++TunedCount;
            break;
        }
    }

    bApplied = true;
    World->GetTimerManager().ClearTimer(RetryHandle);

    UE_LOG(LogTemp, Display,
        TEXT("PASS17_WORLD_ISM_BUDGET_READY tuned=%d nav_disabled=%d families=%d"),
        TunedCount,
        NavigationDisabledCount,
        UE_ARRAY_COUNT(Budgets));
}
