#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCPass45DeploymentStabilitySubsystem.generated.h"

struct FStreamableHandle;
class SWidget;
class UWorld;

/**
 * Keeps frontend/deployment UI responsive and visually opaque while the critical city shell is prepared.
 * Heavy legacy timers are suppressed during the menu phase; the museum exterior is asynchronously preloaded
 * and materialised before deployment is released, so the player does not spawn into an unfinished world.
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

    bool IsMuseumPreparationComplete() const { return bMuseumBuildComplete; }

private:
    void EnsureDeploymentBackdrop();
    void RemoveDeploymentBackdrop();
    void SuppressSynchronousMuseumStartup(UWorld& World);
    void BeginMuseumBuildPreparation(UWorld& World);
    void CompleteMuseumBuildAfterAsyncLoad();
    void ApplyMenuPause(UWorld& World, bool bShouldPause);

    TSharedPtr<SWidget> DeploymentBackdrop;
    TSharedPtr<FStreamableHandle> MuseumPreloadHandle;
    bool bMuseumSuppressionLogged = false;
    bool bMuseumPreparationStarted = false;
    bool bMuseumBuildComplete = false;
    bool bMenuPauseOwned = false;
};
