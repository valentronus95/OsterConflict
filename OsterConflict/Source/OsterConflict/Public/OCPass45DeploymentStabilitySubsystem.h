#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCPass45DeploymentStabilitySubsystem.generated.h"

class FStreamableHandle;
class SWidget;
class UWorld;

/**
 * Keeps the pre-deployment shell responsive and visually opaque.
 * Heavy museum packages are suppressed while any frontend/deployment/settings UI is active,
 * then preloaded asynchronously after a real gameplay pawn exists.
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
    void SuppressSynchronousMuseumStartup(UWorld& World);
    void ReleaseMuseumBuildToGameplay(UWorld& World);
    void CompleteMuseumBuildAfterAsyncLoad();

    TSharedPtr<SWidget> DeploymentBackdrop;
    TSharedPtr<FStreamableHandle> MuseumPreloadHandle;
    bool bMuseumSuppressionLogged = false;
    bool bMuseumBuildReleased = false;
};
