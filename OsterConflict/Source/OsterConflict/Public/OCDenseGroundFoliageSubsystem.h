#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCDenseGroundFoliageSubsystem.generated.h"

/** Dense, collision-aware grass coverage for the playable Oster runtime map. */
UCLASS()
class OSTERCONFLICT_API UOCDenseGroundFoliageSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void Populate(UWorld& World);
    bool bPopulated = false;
};
