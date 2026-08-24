#include "OCCentralPlayableAreaSubsystem.h"

#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"

namespace
{
    // User-approved 2026-08-24 central Oster map reference.
    // Museum is the georeference origin. These bounds keep the central street/landmark
    // cluster visible in the supplied screenshot while removing old ±1 km edge blockout.
    constexpr float MinPlayableX = -78000.0f;
    constexpr float MaxPlayableX =  18000.0f;
    constexpr float MinPlayableY = -12000.0f;
    constexpr float MaxPlayableY =  82000.0f;
    constexpr int32 MaxApplyAttempts = 20;
    constexpr float RetryIntervalSeconds = 0.10f;

    bool IsInsidePlayableArea(const FVector& WorldLocation)
    {
        return WorldLocation.X >= MinPlayableX && WorldLocation.X <= MaxPlayableX &&
            WorldLocation.Y >= MinPlayableY && WorldLocation.Y <= MaxPlayableY;
    }
}

bool UOCCentralPlayableAreaSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCCentralPlayableAreaSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (InWorld.GetNetMode() == NM_DedicatedServer)
    {
        // The authoritative server still needs the same collision/nav bounds even with no renderer,
        // so do not suppress the trim. Dedicated mode is intentionally allowed below.
    }

    ApplyAttempts = 0;
    InWorld.GetTimerManager().SetTimer(
        ApplyTimer,
        this,
        &UOCCentralPlayableAreaSubsystem::ApplyCompactPlayableArea,
        RetryIntervalSeconds,
        true,
        0.05f);
}

void UOCCentralPlayableAreaSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(ApplyTimer);
    ApplyAttempts = 0;
    Super::Deinitialize();
}

void UOCCentralPlayableAreaSubsystem::ApplyCompactPlayableArea()
{
    UWorld* World = GetWorld();
    if (!World) return;

    ++ApplyAttempts;

    AOCWorldSectorOster* Sector = nullptr;
    for (TActorIterator<AOCWorldSectorOster> It(World); It; ++It)
    {
        if (IsValid(*It))
        {
            Sector = *It;
            break;
        }
    }

    if (!Sector)
    {
        if (ApplyAttempts >= MaxApplyAttempts)
        {
            World->GetTimerManager().ClearTimer(ApplyTimer);
            UE_LOG(LogTemp, Error,
                TEXT("PASS44_COMPACT_PLAYABLE_AREA_FAIL reason=world_sector_missing attempts=%d"),
                ApplyAttempts);
        }
        return;
    }

    World->GetTimerManager().ClearTimer(ApplyTimer);

    const FVector SectorLocation = Sector->GetActorLocation();
    const float CenterX = (MinPlayableX + MaxPlayableX) * 0.5f;
    const float CenterY = (MinPlayableY + MaxPlayableY) * 0.5f;
    const float WidthCm = MaxPlayableX - MinPlayableX;
    const float HeightCm = MaxPlayableY - MinPlayableY;

    int32 RemovedInstances = 0;
    int32 KeptInstances = 0;
    int32 TrimmedComponents = 0;
    bool bGroundResized = false;

    TInlineComponentArray<UStaticMeshComponent*> StaticComponents;
    Sector->GetComponents(StaticComponents);
    for (UStaticMeshComponent* Component : StaticComponents)
    {
        if (!IsValid(Component)) continue;
        if (Component->GetName().Equals(TEXT("Ground"), ESearchCase::IgnoreCase))
        {
            // The engine cube is 100 cm. Keep the original 200 cm thickness but replace the 2.4 km square.
            Component->SetRelativeLocation(FVector(CenterX - SectorLocation.X, CenterY - SectorLocation.Y, -100.0f));
            Component->SetRelativeScale3D(FVector(WidthCm / 100.0f, HeightCm / 100.0f, 2.0f));
            bGroundResized = true;
            break;
        }
    }

    TInlineComponentArray<UInstancedStaticMeshComponent*> InstanceComponents;
    Sector->GetComponents(InstanceComponents);
    for (UInstancedStaticMeshComponent* Component : InstanceComponents)
    {
        if (!IsValid(Component)) continue;

        const int32 Before = Component->GetInstanceCount();
        int32 RemovedFromComponent = 0;
        for (int32 Index = Before - 1; Index >= 0; --Index)
        {
            FTransform InstanceWorld;
            if (!Component->GetInstanceTransform(Index, InstanceWorld, true)) continue;

            if (!IsInsidePlayableArea(InstanceWorld.GetLocation()))
            {
                if (Component->RemoveInstance(Index))
                {
                    ++RemovedInstances;
                    ++RemovedFromComponent;
                }
            }
            else
            {
                ++KeptInstances;
            }
        }
        if (RemovedFromComponent > 0) ++TrimmedComponents;
    }

    UE_LOG(LogTemp, Display,
        TEXT("PASS44_COMPACT_PLAYABLE_AREA_READY bounds_m=960x940 x_m=[-780,180] y_m=[-120,820] ground_resized=%d removed_instances=%d kept_instances=%d trimmed_components=%d legacy_2400m_ground=0 reference=oster_central_playable_area_20260824"),
        bGroundResized ? 1 : 0,
        RemovedInstances,
        KeptInstances,
        TrimmedComponents);
}
