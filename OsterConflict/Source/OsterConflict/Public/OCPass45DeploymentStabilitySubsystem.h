#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCPass45DeploymentStabilitySubsystem.generated.h"

class SWidget;

/**
 * Keeps the pre-deployment shell responsive and visually opaque.
 * It also retires the legacy synchronous R13.7 museum startup timer before it can
 * block the game thread while the player is still choosing team/squad/role/spawn.
 */
UCLASS()
class OSTERCONFLICT_API UOCPass45DeploymentStabilitySubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual void Deinitialize() override;

private:
    void EnsureDeploymentBackdrop();
    void RemoveDeploymentBackdrop();
    void RetireSynchronousMuseumStartup();

    TSharedPtr<SWidget> DeploymentBackdrop;
    bool bMuseumStartupRetired = false;
};
