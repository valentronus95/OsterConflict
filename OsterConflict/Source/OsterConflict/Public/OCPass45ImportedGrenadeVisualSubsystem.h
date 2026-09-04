#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "OCPass45ImportedGrenadeVisualSubsystem.generated.h"

/** Replaces the old shared grenade body with exact imported Fab frag/smoke/flash visuals when present. */
UCLASS()
class OSTERCONFLICT_API UOCPass45ImportedGrenadeVisualSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

private:
    void RefreshGrenadeVisuals();
    FTimerHandle RefreshTimer;
    int32 RefreshPass = 0;
};
