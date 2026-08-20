#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13BotMobilitySubsystem.generated.h"

class AOCBotCharacter;

/**
 * Runtime fallback for source-generated maps with missing, partial or disconnected navigation coverage.
 * Existing AI/pathfinding remains authoritative only when a complete path actually exists.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13BotMobilitySubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;

private:
    // Expensive synchronous path checks are cached, while fallback AddMovementInput remains continuous every frame.
    TMap<TWeakObjectPtr<AOCBotCharacter>, double> NavPathRecheckAt;
    TSet<TWeakObjectPtr<AOCBotCharacter>> NavPathTrustedBots;
};
