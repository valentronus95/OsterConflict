#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCPass45StreetInfrastructureSubsystem.generated.h"

/**
 * Reuses the canonical road transforms to place already-imported authored street/utility poles.
 * This avoids inventing a second road map or hard-coded duplicate coordinates.
 */
UCLASS()
class OSTERCONFLICT_API UOCPass45StreetInfrastructureSubsystem : public UTickableWorldSubsystem
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
