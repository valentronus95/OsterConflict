#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCVisualTestSpawnSubsystem.generated.h"

/**
 * R12 visual-test routing used only when the launcher passes -R12VisualSlice.
 * Keeps normal Conquest geography untouched while moving test spawns and nearby combat vehicles
 * onto the current Krushelnytska vertical slice so QA does not begin with a kilometre-long jog.
 */
UCLASS()
class OSTERCONFLICT_API UOCVisualTestSpawnSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void RepositionVisualTestContent(UWorld& World);
};
