#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCDenseGroundFoliageSubsystem.generated.h"

class AActor;
class UHierarchicalInstancedStaticMeshComponent;
class UWorld;
struct FStreamableHandle;

/** Dense, collision-aware grass coverage for the playable Oster runtime map. */
UCLASS()
class OSTERCONFLICT_API UOCDenseGroundFoliageSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override { return bEligible && !bPopulated; }
    virtual bool IsTickableWhenPaused() const override { return true; }

    bool IsWorldFoliageReady() const { return !bEligible || bPopulated; }
    float GetWorldFoliageProgress() const;

private:
    void RequestPreload();
    bool BeginPopulation(UWorld& World);
    void PopulateBatch();

    TSharedPtr<FStreamableHandle> PreloadHandle;
    TWeakObjectPtr<AActor> FoliageActor;
    TArray<TWeakObjectPtr<UHierarchicalInstancedStaticMeshComponent>> GrassComponents;
    TWeakObjectPtr<UHierarchicalInstancedStaticMeshComponent> GroundPlants;
    TWeakObjectPtr<UHierarchicalInstancedStaticMeshComponent> Flowers;

    FRandomStream RandomStream;
    float CursorX = -96000.0f;
    float CursorY = -96000.0f;
    float PopulationMinX = -96000.0f;
    float PopulationMaxX = 96000.0f;
    float PopulationMinY = -96000.0f;
    float PopulationMaxY = 96000.0f;
    float ActiveGridStep = 4000.0f;
    int32 ActiveCellsPerBatch = 4;
    int32 GrassInstances = 0;
    int32 PlantInstances = 0;
    int32 FlowerInstances = 0;
    int32 ProcessedCells = 0;
    int32 CandidateTraceAttempts = 0;
    int32 CandidateAccepted = 0;
    int32 CandidateRejectedBlocked = 0;
    int32 CandidateRejectedTrace = 0;
    int32 CandidateRejectedBounds = 0;
    double EarliestPopulationWallTimeSeconds = 0.0;
    bool bEligible = false;
    bool bPreloadRequested = false;
    bool bLowCPUProfile = false;
    bool bPopulationStarted = false;
    bool bPopulated = false;
};
