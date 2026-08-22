#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCMuseumSpawnGuardSubsystem.generated.h"

class AOCPlayerController;
class APawn;

/**
 * Runtime acceptance guard for the normal deployment route.
 *
 * It repairs/creates the authoritative Museum BASE set and remains active at a low frequency so every
 * newly possessed human pawn that explicitly selected BASE can be verified. If an old GameMode fallback
 * still places that pawn in the field, the server corrects it once to the real Museum BASE.
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
    TMap<TWeakObjectPtr<AOCPlayerController>, TWeakObjectPtr<APawn>> LastValidatedPawnByController;

    bool EnsureAuthoritativeMuseumBases();
    void ValidateBaseDeployments();
};
