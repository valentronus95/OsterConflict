#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCMuseumSpawnGuardSubsystem.generated.h"

class AOCPlayerController;

/**
 * Initial BASE deployment acceptance guard.
 *
 * It repairs/creates the authoritative Museum BASE set and validates each human controller's initial
 * AOCCharacter deployment at most once. Ordinary vehicle possession/unpossession is never a deployment
 * event and must never be teleported to Museum.
 */
UCLASS()
class OSTERCONFLICT_API UOCMuseumSpawnGuardSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override { return true; }

private:
    bool bFinished = false;
    float ValidationAccumulator = 0.0f;
    TSet<TWeakObjectPtr<AOCPlayerController>> ValidatedBaseDeploymentControllers;

    bool EnsureAuthoritativeMuseumBases();
    void ValidateBaseDeployments();
};