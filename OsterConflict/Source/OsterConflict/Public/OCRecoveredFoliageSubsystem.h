#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCRecoveredFoliageSubsystem.generated.h"

/**
 * Lightweight visual-only pass for the restored PN foliage collection.
 * It deliberately uses instancing, no collision and short cull distances.
 */
UCLASS()
class OSTERCONFLICT_API UOCRecoveredFoliageSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void Populate(UWorld& World);
    bool bPopulated = false;
};
