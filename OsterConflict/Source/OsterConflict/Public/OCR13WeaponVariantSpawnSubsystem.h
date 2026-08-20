#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13WeaponVariantSpawnSubsystem.generated.h"

/** Exposes bundled R13 weapon models as real world-pickup gameplay variants without changing starter loadouts. */
UCLASS()
class OSTERCONFLICT_API UOCR13WeaponVariantSpawnSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void TrySpawnBundledVariants(UWorld& World);
    void ScheduleSpawnAttempt(UWorld& World, float DelaySeconds);

    int32 SpawnAttemptCount = 0;
    bool bSpawnComplete = false;
};
