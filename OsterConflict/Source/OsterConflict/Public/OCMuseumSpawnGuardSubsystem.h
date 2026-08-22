#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCMuseumSpawnGuardSubsystem.generated.h"

/**
 * Runtime acceptance guard for the normal deployment route.
 *
 * The game mode still owns normal spawn creation. This subsystem only repairs the authoritative BASE set when
 * a gameplay world exists but a team has no usable BASE spawn. That prevents RestartPlayer() from falling through
 * to the legacy map-origin emergency transform and putting the player back in an empty field.
 */
UCLASS()
class OSTERCONFLICT_API UOCMuseumSpawnGuardSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override { return !bFinished; }

private:
    bool bFinished = false;

    bool EnsureAuthoritativeMuseumBases();
};
