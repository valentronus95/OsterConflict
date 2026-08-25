#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCWorldProductionVisualsSubsystem.generated.h"

/**
 * Pass 45 B2 visual owner for generic central-Oster environment families.
 *
 * OCWorldSectorOster keeps compact semantic box geometry as collision/navigation/backstop data.
 * This subsystem performs one bounded startup conversion of those source transforms to already-imported
 * production meshes/materials and hides the corresponding BasicShape visuals. It never polls after success.
 */
UCLASS()
class OSTERCONFLICT_API UOCWorldProductionVisualsSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

private:
    void TryBuildProductionVisuals();

    TWeakObjectPtr<UWorld> RuntimeWorld;
    FTimerHandle RetryHandle;
    int32 Attempts = 0;
    bool bBuilt = false;
};
