#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCRecoveredRoadsidePropsSubsystem.generated.h"

/** Visual-only dressing for selected restored roadside/construction props. */
UCLASS()
class OSTERCONFLICT_API UOCRecoveredRoadsidePropsSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void Populate(UWorld& World);
    bool bPopulated = false;
};
