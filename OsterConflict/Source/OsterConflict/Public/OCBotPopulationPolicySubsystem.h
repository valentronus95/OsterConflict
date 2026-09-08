#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCBotPopulationPolicySubsystem.generated.h"

/**
 * Restores the player-approved local/listen bot-fill behavior after frontend travel.
 * Frontend-only menu worlds and dedicated servers are deliberately left untouched.
 */
UCLASS()
class OSTERCONFLICT_API UOCBotPopulationPolicySubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;

private:
    bool bPolicyApplied = false;
};