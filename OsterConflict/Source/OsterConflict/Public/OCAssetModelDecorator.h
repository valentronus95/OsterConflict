#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OCAssetModelDecorator.generated.h"

class AActor;
class USceneComponent;
class UInstancedStaticMeshComponent;

/**
 * Runtime visual layer that turns the already-imported environment packs into
 * actual Oster scenery without changing the authoritative gameplay/collision
 * layout in AOCWorldSectorOster.
 *
 * The old primitive building cores remain as collision proxies for now. Their
 * obviously-placeholder roofs/details and primitive vegetation are hidden and
 * replaced by real meshes from AdvancedVillagePack and Modular_Rural_Cabin.
 */
UCLASS()
class OSTERCONFLICT_API AOCAssetModelDecorator : public AActor
{
    GENERATED_BODY()

public:
    AOCAssetModelDecorator();

    void PopulateForSector(AActor* SectorActor);

private:
    static void AddMeshInstance(UInstancedStaticMeshComponent* Component, const FVector& Location,
        float YawDegrees, const FVector& Scale = FVector(1.0f));
    static void AddFenceLine(UInstancedStaticMeshComponent* Component, const FVector& Center,
        float LengthCm, float YawDegrees, float ZCm = 0.0f);

    void HideReplacedProxyComponents(AActor* SectorActor) const;
    void BuildResidentialModels();
    void BuildVegetationModels();
    void BuildInfrastructureModels();
    void BuildAmbientProps();

    bool bPopulated = false;

    UPROPERTY() USceneComponent* SceneRoot;

    UPROPERTY() UInstancedStaticMeshComponent* HouseA;
    UPROPERTY() UInstancedStaticMeshComponent* HouseB;
    UPROPERTY() UInstancedStaticMeshComponent* TreeA;
    UPROPERTY() UInstancedStaticMeshComponent* TreeB;
    UPROPERTY() UInstancedStaticMeshComponent* TreeC;
    UPROPERTY() UInstancedStaticMeshComponent* PineA;
    UPROPERTY() UInstancedStaticMeshComponent* PineB;
    UPROPERTY() UInstancedStaticMeshComponent* OldFence;
    UPROPERTY() UInstancedStaticMeshComponent* StreetLight;
    UPROPERTY() UInstancedStaticMeshComponent* PowerPole;
    UPROPERTY() UInstancedStaticMeshComponent* Bridge;
    UPROPERTY() UInstancedStaticMeshComponent* SideShed;
    UPROPERTY() UInstancedStaticMeshComponent* Crate;
    UPROPERTY() UInstancedStaticMeshComponent* MetalBarrel;
    UPROPERTY() UInstancedStaticMeshComponent* ShoppingCart;
    UPROPERTY() UInstancedStaticMeshComponent* PicnicTable;
    UPROPERTY() UInstancedStaticMeshComponent* Tire;
    UPROPERTY() UInstancedStaticMeshComponent* Bush;
    UPROPERTY() UInstancedStaticMeshComponent* Well;
};
