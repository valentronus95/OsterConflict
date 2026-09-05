#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "OCReferenceDrivenResidentialValidationSubsystem.generated.h"

/**
 * Pass 45 Gate E validation-only audit for rejected generic residential presentation.
 *
 * The primary authoring path already retired procedural private houses/fences. This subsystem proves the
 * final gameplay world did not resurrect those families through another actor, imported mesh, or late owner.
 * It never destroys, hides, replaces, recolors, or relocates runtime content.
 */
UCLASS()
class OSTERCONFLICT_API UOCReferenceDrivenResidentialValidationSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

private:
    FTimerHandle ValidationTimer;
    TWeakObjectPtr<UWorld> ValidationWorld;

    void ValidateReferenceDrivenResidentialWorld();
};
