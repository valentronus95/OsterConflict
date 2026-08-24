#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCFoliageRuntimeGuardSubsystem.generated.h"

/**
 * Retires the old source-only cube ground-cover proxies and proves that runtime ground cover
 * is owned by the real dense foliage HISM pass instead.
 */
UCLASS()
class OSTERCONFLICT_API UOCFoliageRuntimeGuardSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override { return !bFinished; }

private:
    bool RetireSourceGroundCoverProxies();
    bool ValidateDenseFoliage(int32 MinGrassInstances, int32& OutGrassInstances, int32& OutDenseGrassComponents) const;
    void FailValidation(const FString& Reason);

    float ElapsedSeconds = 0.0f;
    bool bFinished = false;
    bool bProxyRetirementObserved = false;
};