#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCRecoveredFoliageSubsystem.generated.h"

/** Lightweight visual-only use of restored PN foliage assets in compact R13. */
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