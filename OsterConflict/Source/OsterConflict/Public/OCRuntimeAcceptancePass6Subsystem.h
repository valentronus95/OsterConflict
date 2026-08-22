#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "OCRuntimeAcceptancePass6Subsystem.generated.h"

/** Compatibility correction owner recovered from the earlier acceptance pass. */
UCLASS()
class OSTERCONFLICT_API UOCRuntimeAcceptancePass6Subsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

private:
    void ApplyAcceptanceCorrections();
    void RemoveLegacyGameplayBaseInstances();
    void NormalizeProductionStaticWeapons();

    FTimerHandle AcceptanceTimer;
    bool bLegacyBaseCleanupComplete = false;
};
