#include "OCWorldAuthoringLabelSuppressionSubsystem.h"

#include "OCWorldSectorOster.h"

#include "Components/TextRenderComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"

bool UOCWorldAuthoringLabelSuppressionSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCWorldAuthoringLabelSuppressionSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    if (InWorld.GetNetMode() == NM_DedicatedServer) return;
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    int32 HiddenLabels = 0;
    for (TActorIterator<AOCWorldSectorOster> It(&InWorld); It; ++It)
    {
        AOCWorldSectorOster* Sector = *It;
        if (!IsValid(Sector) || Sector->IsActorBeingDestroyed()) continue;

        TArray<UTextRenderComponent*> Labels;
        Sector->GetComponents<UTextRenderComponent>(Labels);
        for (UTextRenderComponent* Label : Labels)
        {
            if (!IsValid(Label)) continue;

            // These components are editor/authoring landmarks, not player-facing world geometry.
            Label->SetHiddenInGame(true);
            Label->SetCastShadow(false);
            Label->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            ++HiddenLabels;
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_RUNTIME_AUTHORING_LABELS_HIDDEN labels=%d gameplay_text_geometry=0"),
        HiddenLabels);
}
