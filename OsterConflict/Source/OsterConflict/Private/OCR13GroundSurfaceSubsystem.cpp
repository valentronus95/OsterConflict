#include "OCR13GroundSurfaceSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
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
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    // Source BeginPlay assigns a temporary debug tint. Apply a deliberately matte city-ground surface after the
    // source actor exists. The previous Diorama_Ground material reads as broad wet/reflection patches when stretched
    // across the compact city floor, which made dry streets and yards look flooded in the R13 playtest.
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

    UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!BaseMaterial)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("R13.6 ground surface: matte engine material unavailable; preserving source ground tint."));
        return;
    }

    UMaterialInstanceDynamic* GroundMaterial = UMaterialInstanceDynamic::Create(
        BaseMaterial, Ground, TEXT("R13_MatteOsterGround"));
    if (!GroundMaterial) return;

    // Neutral muted grass/soil base. Dense grass, roads, sidewalks and later district materials provide the detail;
    // the broad authoritative floor should never impersonate a lake or glossy wetland.
    GroundMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.19f, 0.24f, 0.13f, 1.0f));

    // Material-only replacement. Do not touch Ground collision, scale, location or visibility because the same
    // component is the authoritative broad walkable floor used by spawn-safety traces and vehicle grounding.
    Ground->SetMaterial(0, GroundMaterial);
    Ground->SetCastShadow(false);

    UE_LOG(LogTemp, Display,
        TEXT("R13.6 ground surface: matte non-water city floor applied; source collision and compact-map bounds preserved."));
}