#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCTreeContentUpgradeSubsystem.generated.h"

/**
 * One-shot Pass45 content-intake upgrade for the existing Oster tree families.
 * It mutates only the three tree ISMs already authored by AOCWorldSectorOster and preserves their placement.
 */
UCLASS()
class OSTERCONFLICT_API UOCTreeContentUpgradeSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
};
