#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCFoliageRuntimeGuardSubsystem.generated.h"

/**
 * Retires source-only ground-cover proxies and proves that normal runtime vegetation is not owned by
 * primitive Cube/Cylinder/Sphere tree families. PASS45 item 26 requires authored tree meshes at source.
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
    bool ValidateSourceAuthoredTrees();
    bool ValidateDenseFoliage(int32 MinGrassInstances, int32& OutGrassInstances, int32& OutDenseGrassComponents) const;
    void FailValidation(const FString& Reason);

    float ElapsedSeconds = 0.0f;
    float ValidationAccumulator = 0.0f;
    bool bFinished = false;
    bool bProxyRetirementObserved = false;
    bool bAuthoredTreeValidationObserved = false;
};