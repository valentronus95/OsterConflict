#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCVisualFidelityGateKSubsystem.generated.h"

/**
 * PASS45 Gate K runtime truth guard.
 *
 * This does not mutate scenery. It observes the final player-facing gameplay world after startup cleanup and rejects
 * runtime-visible /Engine/BasicShapes static-mesh content regardless of which gameplay actor owns it.
 *
 * A first clean scan is not a permanent pass: gameplay can spawn weapons, ordnance, characters or vehicles later.
 * After the initial three-second cleanup window the guard therefore keeps a low-frequency observation watch active
 * for the lifetime of the gameplay world. Any later visible BasicShape is a factual Gate K failure.
 * A source verifier being green may never convert an unresolved visual-content gap into runtime acceptance.
 */
UCLASS()
class OSTERCONFLICT_API UOCVisualFidelityGateKSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override { return !bFinished; }

private:
    float ElapsedSeconds = 0.0f;
    float NextObservationSeconds = 0.0f;
    bool bReadyLogged = false;
    bool bFinished = false;
};
