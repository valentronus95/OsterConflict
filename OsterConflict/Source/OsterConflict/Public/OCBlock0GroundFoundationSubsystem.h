#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCBlock0GroundFoundationSubsystem.generated.h"

/**
 * PASS45 Block 0 pre-tick ground foundation.
 *
 * AOCWorldSectorOster still carries the historical Cube transform as compact-map sizing data.
 * This subsystem replaces that source proxy with the tracked authored ground mesh/material during
 * UWorld::BeginPlay, before the first gameplay tick, so the later world-surface upgrader only validates
 * an already-authored Ground component instead of owning a delayed 0.75 s ground mutation.
 */
UCLASS()
class OSTERCONFLICT_API UOCBlock0GroundFoundationSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
};
