#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "OCPass45ImportedDoorVisualSubsystem.generated.h"

/**
 * Presentation-only bridge for already-imported local door meshes.
 * The existing replicated AOCInteractableDoor remains the sole gameplay/interaction owner.
 */
UCLASS()
class OSTERCONFLICT_API UOCPass45ImportedDoorVisualSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

private:
    FTimerHandle RefreshTimer;
    int32 RefreshPass = 0;

    void RefreshDoorVisuals();
};
