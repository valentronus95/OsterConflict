#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCLocalInboxWorldAssetsSubsystem.generated.h"

/**
 * Replaces source-only BasicShape world presentation with meshes supplied through models_game_OC.
 * Existing authored transforms/collision remain authoritative; the local meshes become the visible layer.
 */
UCLASS()
class OSTERCONFLICT_API UOCLocalInboxWorldAssetsSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

private:
    void ApplyWorldAssets();
    FTimerHandle ApplyTimer;
};
