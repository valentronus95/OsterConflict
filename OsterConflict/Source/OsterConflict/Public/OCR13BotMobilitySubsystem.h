#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13BotMobilitySubsystem.generated.h"

/**
 * Runtime fallback for source-generated maps that currently have no baked navigation data.
 * Existing AI/pathfinding remains authoritative whenever NavData exists.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13BotMobilitySubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
};
