#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCPass45DeploymentStabilitySubsystem.generated.h"

struct FStreamableHandle;
class SWidget;
class UWorld;

/**
 * Keeps frontend/deployment UI responsive and visually opaque.
 * Heavy world timers are prevented from running while a blocking menu owns the screen,
 * then normal gameplay resumes once the player has deployed.
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
    virtual bool IsTickableWhenPaused() const override { return true; }

private:
    void EnsureDeploymentBackdrop();
    void RemoveDeploymentBackdrop();
    void SuppressSynchronousMuseumStartup(UWorld& World);
    void ReleaseMuseumBuildToGameplay(UWorld& World);
    void CompleteMuseumBuildAfterAsyncLoad();
    void ApplyMenuPause(UWorld& World, bool bShouldPause);

    TSharedPtr<SWidget> DeploymentBackdrop;
    TSharedPtr<FStreamableHandle> MuseumPreloadHandle;
    bool bMuseumSuppressionLogged = false;
    bool bMuseumBuildReleased = false;
    bool bMenuPauseOwned = false;
};
