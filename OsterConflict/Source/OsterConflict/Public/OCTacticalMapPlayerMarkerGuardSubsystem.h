#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "OCTacticalMapPlayerMarkerGuardSubsystem.generated.h"

class UOCTacticalMapWidget;

/**
 * Keeps the local player marker visually above objective/POI labels on the Tactical Map.
 * The existing map widget remains the sole owner of projection and marker position; this guard only
 * corrects presentation priority after the widget is constructed.
 */
UCLASS()
class OSTERCONFLICT_API UOCTacticalMapPlayerMarkerGuardSubsystem final : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

private:
    FTimerHandle MarkerPollTimer;
    TWeakObjectPtr<UOCTacticalMapWidget> LastAdjustedWidget;

    void RefreshPlayerMarkerPriority();
};
