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

    void AddResidentialHouse(const FVector& Location, float YawDegrees, const FVector& Scale,
        int32 VariantSeed);
    UInstancedStaticMeshComponent* SelectResidentialFence(int32 VariantSeed) const;
    UInstancedStaticMeshComponent* SelectBridge(int32 VariantSeed) const;
    void AddAuthoredWell(const FVector& Location, float YawDegrees, const FVector& Scale,
        int32 VariantSeed);

    void HideReplacedProxyComponents(AActor* SectorActor) const;
    void BuildResidentialModels();
    void BuildVegetationModels();
    void BuildInfrastructureModels();
    void BuildAmbientProps();

    bool bPopulated = false;

    UPROPERTY() USceneComponent* SceneRoot;

    // Base authored houses stay as the collision-aligned visual shell. Extra meshes from the same
    // AdvancedVillagePack house family are layered at the exact same transform to create genuine
    // facade/silhouette variants without inventing new map ownership or fake primitive buildings.
    UPROPERTY() UInstancedStaticMeshComponent* HouseA;
    UPROPERTY() UInstancedStaticMeshComponent* HouseB;
    UPROPERTY() UInstancedStaticMeshComponent* HouseAExtra01;
    UPROPERTY() UInstancedStaticMeshComponent* HouseAExtra02;
    UPROPERTY() UInstancedStaticMeshComponent* HouseAExtra03;
    UPROPERTY() UInstancedStaticMeshComponent* HouseAExtra04;
    UPROPERTY() UInstancedStaticMeshComponent* HouseAExtra05;
    UPROPERTY() UInstancedStaticMeshComponent* HouseAExtra06;
    UPROPERTY() UInstancedStaticMeshComponent* HouseAExtra07;
    UPROPERTY() UInstancedStaticMeshComponent* HouseAExtra08;
    UPROPERTY() UInstancedStaticMeshComponent* HouseBExtra;

    UPROPERTY() UInstancedStaticMeshComponent* TreeA;
    UPROPERTY() UInstancedStaticMeshComponent* TreeB;
    UPROPERTY() UInstancedStaticMeshComponent* TreeC;
    UPROPERTY() UInstancedStaticMeshComponent* TreeD;
    UPROPERTY() UInstancedStaticMeshComponent* TreeE;
    UPROPERTY() UInstancedStaticMeshComponent* PineA;
    UPROPERTY() UInstancedStaticMeshComponent* PineB;

    UPROPERTY() UInstancedStaticMeshComponent* OldFence;
    UPROPERTY() UInstancedStaticMeshComponent* VillageFenceA;
    UPROPERTY() UInstancedStaticMeshComponent* VillageFenceB;
    UPROPERTY() UInstancedStaticMeshComponent* VillageFenceC;
    UPROPERTY() UInstancedStaticMeshComponent* VillageFenceD;

    UPROPERTY() UInstancedStaticMeshComponent* StreetLight;
    UPROPERTY() UInstancedStaticMeshComponent* PowerPole;
    UPROPERTY() UInstancedStaticMeshComponent* BridgeA;
    UPROPERTY() UInstancedStaticMeshComponent* BridgeB;
    UPROPERTY() UInstancedStaticMeshComponent* BridgeC;
    UPROPERTY() UInstancedStaticMeshComponent* BridgeD;
    UPROPERTY() UInstancedStaticMeshComponent* SideShed;
    UPROPERTY() UInstancedStaticMeshComponent* Crate;
    UPROPERTY() UInstancedStaticMeshComponent* MetalBarrel;
    UPROPERTY() UInstancedStaticMeshComponent* ShoppingCart;
    UPROPERTY() UInstancedStaticMeshComponent* PicnicTable;
    UPROPERTY() UInstancedStaticMeshComponent* Tire;
    UPROPERTY() UInstancedStaticMeshComponent* Bush;
    UPROPERTY() UInstancedStaticMeshComponent* Well;
    UPROPERTY() UInstancedStaticMeshComponent* WellExtra01;
    UPROPERTY() UInstancedStaticMeshComponent* WellExtra02;
    UPROPERTY() UInstancedStaticMeshComponent* WellExtra03;
    UPROPERTY() UInstancedStaticMeshComponent* WellExtra04;
};
