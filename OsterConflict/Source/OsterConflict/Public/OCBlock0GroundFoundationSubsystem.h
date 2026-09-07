#pragma once

#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCBlock0GroundFoundationSubsystem.generated.h"

/**
 * PASS45 Block 0 ground foundation.
 *
 * The tracked authored ground mesh/material are prepared asynchronously during world startup and
 * applied from resident assets before gameplay is released. This subsystem owns no tick/timer path.
 */
UCLASS()
class OSTERCONFLICT_API UOCBlock0GroundFoundationSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

private:
    void RequestGroundPreload();
    void HandleGroundPreloadComplete();

    TSharedPtr<FStreamableHandle> GroundPreloadHandle;
    bool bGroundAttemptFinished = false;
};
