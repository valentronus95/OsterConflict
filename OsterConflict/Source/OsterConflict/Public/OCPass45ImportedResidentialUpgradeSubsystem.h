#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCPass45ImportedResidentialUpgradeSubsystem.generated.h"

/**
 * Replaces only generic source-cube residential instances with an already-imported local building mesh.
 * Verified landmarks are intentionally excluded and remain owned by their dedicated landmark systems.
 */
UCLASS()
class OSTERCONFLICT_API UOCPass45ImportedResidentialUpgradeSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
};
