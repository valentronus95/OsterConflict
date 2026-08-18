#include "OCR13GroundSurfaceSubsystem.h"

#include "OCWorldSectorOster.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"

bool UOCR13GroundSurfaceSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13GroundSurfaceSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    // Source BeginPlay assigns the temporary solid-green debug tint. Apply the real surface immediately after
    // the source actor is expected to exist, without changing the ground transform, collision or compact bounds.
    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) ApplyGroundSurface(*World);
        }), 0.65f, false);
}

void UOCR13GroundSurfaceSubsystem::ApplyGroundSurface(UWorld& World)
{
    AOCWorldSectorOster* WorldSector = nullptr;
    for (TActorIterator<AOCWorldSectorOster> It(&World); It; ++It)
    {
        WorldSector = *It;
        if (WorldSector) break;
    }
    if (!WorldSector) return;

    UStaticMeshComponent* Ground = FindObjectFast<UStaticMeshComponent>(WorldSector, TEXT("Ground"));
    if (!Ground) return;

    // This material is already bundled with the rural environment content and is authored for a regular mesh,
    // unlike a Landscape-only material that may depend on Landscape coordinates/layers.
    UMaterialInterface* GroundMaterial = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Materials/Instances/Diorama_Ground.Diorama_Ground"));

    if (!GroundMaterial)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("R13.4 ground surface: Diorama_Ground material unavailable; preserving source ground tint."));
        return;
    }

    // Material-only replacement. Do not touch Ground collision, scale, location or visibility because the same
    // component is the authoritative broad walkable floor used by spawn-safety traces and vehicle grounding.
    Ground->SetMaterial(0, GroundMaterial);
    Ground->SetCastShadow(false);

    UE_LOG(LogTemp, Display,
        TEXT("R13.4 ground surface: committed terrain material applied; source collision and map bounds preserved."));
}
