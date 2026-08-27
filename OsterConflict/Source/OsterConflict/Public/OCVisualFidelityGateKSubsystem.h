#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCVisualFidelityGateKSubsystem.generated.h"

/**
 * PASS45 Gate K runtime truth guard.
 *
 * This does not mutate scenery. It observes the final player-facing world after startup cleanup and rejects
 * visible /Engine/BasicShapes static-mesh content owned by the canonical Oster sector or authoritative stadium.
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
    bool bFinished = false;
};
