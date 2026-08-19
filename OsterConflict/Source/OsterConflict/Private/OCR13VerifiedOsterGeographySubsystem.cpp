#include "OCR13VerifiedOsterGeographySubsystem.h"

#include "OCGameMode.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

namespace
{
    constexpr float LegacySlicePurgeDelaySeconds = 1.82f;

    bool IsLegacySliceComponent(const FName Name)
    {
        const FString Value = Name.ToString();
        return Value.StartsWith(TEXT("R12_House")) ||
            Value.StartsWith(TEXT("R12_Fence")) ||
            Value.StartsWith(TEXT("R12_Tree")) ||
            Value.StartsWith(TEXT("R12_Plants")) ||
            Value.StartsWith(TEXT("R12_Barrels")) ||
            Value.StartsWith(TEXT("R12_Crates")) ||
            Value.StartsWith(TEXT("R12_StreetLights")) ||
            Value.StartsWith(TEXT("R13_KrushelnytskaGrass")) ||
            Value.StartsWith(TEXT("R13_KrushelnytskaPine")) ||
            Value.StartsWith(TEXT("R13_KrushelnytskaUtility")) ||
            Value.StartsWith(TEXT("R13_KrushelnytskaPole"));
    }
}

bool UOCR13VerifiedOsterGeographySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13VerifiedOsterGeographySubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    // Migration-only ordering: delete obsolete R12/R13 near-spawn presentation before the legacy 1.95 s
    // residential styling pass can discover it. No permanent geography is created or moved here.
    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle LegacyPurgeTimer;
    InWorld.GetTimerManager().SetTimer(LegacyPurgeTimer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) SuppressLegacyNearSpawnSlice(*World);
        }), LegacySlicePurgeDelaySeconds, false);
}

void UOCR13VerifiedOsterGeographySubsystem::SuppressLegacyNearSpawnSlice(UWorld& World)
{
    int32 RemovedComponents = 0;
    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor) continue;

        TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
        Actor->GetComponents(Components);
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            if (!Component || !IsLegacySliceComponent(Component->GetFName())) continue;

            Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            Component->SetCanEverAffectNavigation(false);
            Component->DestroyComponent();
            ++RemovedComponents;
        }
    }

    if (RemovedComponents > 0)
    {
        UE_LOG(LogTemp, Display,
            TEXT("R13 location-first: purged %d obsolete near-spawn Krushelnytska presentation components; no geography was relocated."),
            RemovedComponents);
    }
}
