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

    // Pass 45: Pass 17 budgets were authored for the historical much larger sector. On the current
    // 960 x 940 m battlefield, 700-1300 m cull distances effectively kept almost every source family
    // alive at once. Primitive tree families are retired by item 26; only authored tree components
    // receive tree budgets now.
    constexpr FISMRenderBudget Budgets[] =
    {
        { TEXT("Roads"),                  0,  90000, false },
        { TEXT("Sidewalks"),           8000,  42000, false },
        { TEXT("Buildings"),          40000,  78000, true  },
        { TEXT("ResidentialRoofs"),   30000,  58000, false },
        { TEXT("ResidentialDetails"),  6000,  24000, false },
        { TEXT("LandmarkBlocks"),     50000,  95000, true  },
        { TEXT("LandmarkRoofs"),      40000,  76000, false },
        { TEXT("LandmarkWindows"),     6000,  30000, false },
        { TEXT("LandmarkDetails"),    10000,  40000, false },
        { TEXT("Fences"),              6000,  28000, false },
        { TEXT("WoodFences"),          6000,  28000, false },
        { TEXT("MetalFences"),         6000,  28000, false },
        { TEXT("LightSheetFences"),    6000,  28000, false },
        { TEXT("AuthoredDeciduousTrees"), 12000, 42000, true  },
        { TEXT("AuthoredPine01Trees"),    12000, 46000, true  },
        { TEXT("AuthoredPine03Trees"),    12000, 46000, true  },
        { TEXT("GrassMown"),              0,  16000, false },
        { TEXT("GrassRough"),             0,  18000, false },
        { TEXT("GrassWetland"),           0,  20000, false },
        { TEXT("StadiumGeometry"),         0,  55000, false },
        { TEXT("StadiumDetails"),       6000,  32000, false },
        { TEXT("ParkGeometry"),            0,  52000, false },
        { TEXT("ParkDetails"),          6000,  30000, false },
        { TEXT("Waterways"),               0,  60000, false },
        { TEXT("Bridges"),             30000,  75000, true  },
        { TEXT("ReferenceMarkers"),        0,   3000, false },
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
        TEXT("PASS45_COMPACT_WORLD_CULL_BUDGET_READY tuned=%d nav_disabled=%d families=%d map_m=960x940 max_landmark_cull_m=950 authored_tree_max_cull_m=460 small_detail_cull_m=240_400 primitive_tree_budget=0"),
        TunedCount,
        NavigationDisabledCount,
        static_cast<int32>(UE_ARRAY_COUNT(Budgets)));
    UE_LOG(LogTemp, Display,
        TEXT("PASS17_WORLD_ISM_BUDGET_READY tuned=%d nav_disabled=%d families=%d"),
        TunedCount,
        NavigationDisabledCount,
        static_cast<int32>(UE_ARRAY_COUNT(Budgets)));
}
