#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCLandmarkStartupCoordinatorSubsystem.generated.h"

class UWorld;

/**
 * Current-main owner of landmark startup ordering.
 *
 * The historical Museum/Silpo/Culture stages remain responsible for their own geometry and gameplay.
 * This subsystem only collapses their old multi-second reveal chain into one deterministic startup pass,
 * preventing visible late rebuilds and ownership races without inventing another presentation layer.
 */
UCLASS()
class OSTERCONFLICT_API UOCLandmarkStartupCoordinatorSubsystem final : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void RunAuthoritativeStartup(UWorld& World);
};