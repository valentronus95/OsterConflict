#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCLocalInboxWorldAssetsSubsystem.generated.h"

/**
 * Optional local-inbox intake/validation route. Normal gameplay does not apply this arbitrary mesh-pool
 * replacement path; it is enabled only by explicit validation/intake command-line switches.
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
