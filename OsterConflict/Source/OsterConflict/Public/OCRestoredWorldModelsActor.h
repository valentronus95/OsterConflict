#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OCRestoredWorldModelsActor.generated.h"

class USceneComponent;
class UInstancedStaticMeshComponent;

/**
 * Visual-only placement layer for environment packs recovered from R13 content staging.
 * Gameplay collision remains owned by the existing Oster world sector until each recovered
 * mesh has been verified in UE at its native scale.
 */
UCLASS()
class OSTERCONFLICT_API AOCRestoredWorldModelsActor : public AActor
{
    GENERATED_BODY()

public:
    AOCRestoredWorldModelsActor();
    void Populate();

private:
    static void AddInstance(UInstancedStaticMeshComponent* Component, const FVector& Location,
        float YawDegrees = 0.0f, const FVector& Scale = FVector(1.0f));

    void BuildForestTracks();
    void BuildUnfinishedSite();
    void BuildRoadworksProps();

    bool bPopulated = false;

    UPROPERTY() TObjectPtr<USceneComponent> SceneRoot;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> ForestPath;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> RoadGround;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> RoadAsphalt;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> UnfinishedFloor;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> UnfinishedWall;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> UnfinishedPillar;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> UnfinishedStair;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> CementBag;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> CableWheel;
};
