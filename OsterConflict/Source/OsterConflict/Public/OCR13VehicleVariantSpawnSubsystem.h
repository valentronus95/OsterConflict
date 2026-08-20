#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13VehicleVariantSpawnSubsystem.generated.h"

/** Spawns vehicle styles whose art already ships with R13 without modifying the legacy fleet layout. */
UCLASS()
class OSTERCONFLICT_API UOCR13VehicleVariantSpawnSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void TrySpawnBundledVehicleVariants(UWorld& World);
    void ScheduleSpawnAttempt(UWorld& World, float DelaySeconds);

    int32 SpawnAttemptCount = 0;
    bool bSpawnComplete = false;
};
