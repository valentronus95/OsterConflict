#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCWorldAssetModelsSubsystem.generated.h"

class AActor;

/** Automatically attaches the imported model layer to Oster gameplay worlds. */
UCLASS()
class OSTERCONFLICT_API UOCWorldAssetModelsSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    TWeakObjectPtr<AActor> DecoratorActor;
};
