#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCPass45TrenchSetpieceSubsystem.generated.h"

/**
 * Uses the already-imported Fab/Megascans sandbag packages as bounded visual set pieces around team bases.
 * The set pieces are deliberately non-colliding until dedicated-server/collision acceptance is done, so this
 * local authored upgrade cannot create client/server movement disagreement.
 */
UCLASS()
class OSTERCONFLICT_API UOCPass45TrenchSetpieceSubsystem : public UTickableWorldSubsystem
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
