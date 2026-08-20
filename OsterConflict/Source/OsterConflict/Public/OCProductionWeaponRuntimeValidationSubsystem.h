#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCProductionWeaponRuntimeValidationSubsystem.generated.h"

class UWorld;

/**
 * Explicit R14 production-weapon validation gate.
 *
 * The subsystem is inert during normal gameplay. It only runs when the process is
 * launched with -ValidateProductionWeapons, then spawns transient test weapons far
 * below the playable world, validates their production visuals, writes a report and
 * destroys the temporary actors.
 */
UCLASS()
class OSTERCONFLICT_API UOCProductionWeaponRuntimeValidationSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void ValidateProductionWeapons(UWorld& World);
};
