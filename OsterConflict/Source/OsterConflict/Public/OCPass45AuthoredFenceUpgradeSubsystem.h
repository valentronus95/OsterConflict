#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCPass45AuthoredFenceUpgradeSubsystem.generated.h"

/**
 * Replaces the canonical generic Fences BasicShape owner with tracked Street Props fence segments.
 * Existing authored wood/metal/light-sheet families are deliberately left alone.
 */
UCLASS()
class OSTERCONFLICT_API UOCPass45AuthoredFenceUpgradeSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override { return !bFinished; }

private:
    float ElapsedSeconds = 0.0f;
    bool bFinished = false;
};
