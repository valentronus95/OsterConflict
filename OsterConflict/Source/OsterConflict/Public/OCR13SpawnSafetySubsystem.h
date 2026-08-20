#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13SpawnSafetySubsystem.generated.h"

class AOCCharacter;
class AOCPlayerController;

/**
 * Validates each newly possessed human character against real collision before gameplay begins.
 * A bad spawn is snapped to walkable ground or rejected back to deployment instead of letting
 * the player fall through/behind the runtime map.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13SpawnSafetySubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;

private:
    bool ResolveGroundAt(const FVector& XYLocation, const AActor* IgnoreActor, FVector& OutCharacterLocation) const;
    bool ResolveSafeTeamFallback(const AOCPlayerController* PC, const AActor* IgnoreActor, FVector& OutCharacterLocation) const;
    bool ValidateNewPawn(AOCPlayerController* PC, AOCCharacter* Character);

    TMap<TWeakObjectPtr<AOCPlayerController>, TWeakObjectPtr<AOCCharacter>> LastValidatedPawn;
};
