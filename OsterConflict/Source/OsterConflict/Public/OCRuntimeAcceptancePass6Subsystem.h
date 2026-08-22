#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "OCRuntimeAcceptancePass6Subsystem.generated.h"

/**
 * Narrow runtime correction owner for acceptance pass 6.
 *
 * This exists to bridge two legacy-generation defects without scattering per-weapon/per-map hacks:
 * - remove the obsolete source-only gameplay BASE instances still emitted by OCWorldSectorOster;
 * - normalize restored production StaticMesh weapon axes and keep hidden fallback geometry inert.
 *
 * Runtime approval is still required before these corrections can be marked VERIFIED.
 */
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
