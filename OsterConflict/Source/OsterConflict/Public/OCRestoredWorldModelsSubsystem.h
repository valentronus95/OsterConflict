#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCRestoredWorldModelsSubsystem.generated.h"

class AActor;
class UWorld;

/** Spawns the recovered R13 environment-model layer once the Oster sector exists. */
UCLASS()
class OSTERCONFLICT_API UOCRestoredWorldModelsSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void TryAttach(UWorld* World);

    int32 AttachAttempts = 0;
    TWeakObjectPtr<AActor> SpawnedLayer;
};
