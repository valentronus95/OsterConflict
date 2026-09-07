#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCPass45DeploymentInputRecoverySubsystem.generated.h"

class AOCPlayerController;

/**
 * One-shot recovery for the START -> listen travel -> Deployment handoff.
 * It never reapplies input every frame: doing that can break Slate mouse-up routing.
 */
UCLASS()
class OSTERCONFLICT_API UOCPass45DeploymentInputRecoverySubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;

private:
    TWeakObjectPtr<AOCPlayerController> ArmedController;
    bool bDeploymentInputArmed = false;
};
