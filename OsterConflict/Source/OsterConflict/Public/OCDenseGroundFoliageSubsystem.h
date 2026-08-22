#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "OCDenseGroundFoliageSubsystem.generated.h"

class AActor;
class UHierarchicalInstancedStaticMeshComponent;
class UWorld;

/** Dense, collision-aware grass coverage for the playable Oster runtime map. */
UCLASS()
class OSTERCONFLICT_API UOCDenseGroundFoliageSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

private:
    void TryPopulateWhenGameplayReady();
    bool BeginPopulation(UWorld& World);
    void PopulateBatch();

    FTimerHandle GameplayReadyTimer;
    FTimerHandle PopulationBatchTimer;

    TWeakObjectPtr<AActor> FoliageActor;
    TArray<TWeakObjectPtr<UHierarchicalInstancedStaticMeshComponent>> GrassComponents;
    TWeakObjectPtr<UHierarchicalInstancedStaticMeshComponent> GroundPlants;
    TWeakObjectPtr<UHierarchicalInstancedStaticMeshComponent> Flowers;

    FRandomStream RandomStream;
    float CursorX = -96000.0f;
    float CursorY = -96000.0f;
    int32 GrassInstances = 0;
    int32 PlantInstances = 0;
    int32 FlowerInstances = 0;
    bool bPopulationStarted = false;
    bool bPopulated = false;
};
