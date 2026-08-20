#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCRecoveredWeaponVariantSubsystem.generated.h"

/** Makes restored R13 weapon variants directly testable in Sandbox without replacing existing slots. */
UCLASS()
class OSTERCONFLICT_API UOCRecoveredWeaponVariantSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void SpawnSandboxRack();
};
