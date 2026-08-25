#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OCAssetModelDecorator.generated.h"

class AActor;
class USceneComponent;
class UInstancedStaticMeshComponent;

/**
 * Runtime decoration for environment details that are not rejected residential replacements.
 *
 * Pass 45 post-merge rule: generic AdvancedVillagePack houses/fences and Modular_Rural_Cabin
 * Side_Shed are not Oster-authentic residential production content. The semantic residential
 * baseline in AOCWorldSectorOster stays visible until a reference-faithful Oster family exists.
 * This decorator may add accepted vegetation/infrastructure/ambient props, but it may not hide
 * the residential baseline and replace it with the rejected village-pack family.
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

    UInstancedStaticMeshComponent* SelectBridge(int32 VariantSeed) const;
    void AddAuthoredWell(const FVector& Location, float YawDegrees, const FVector& Scale,
        int32 VariantSeed);

    void HideReplacedProxyComponents(AActor* SectorActor) const;
    void BuildVegetationModels();
    void BuildInfrastructureModels();
    void BuildAmbientProps();

    bool bPopulated = false;

    UPROPERTY() USceneComponent* SceneRoot;

    UPROPERTY() UInstancedStaticMeshComponent* TreeA;
    UPROPERTY() UInstancedStaticMeshComponent* TreeB;
    UPROPERTY() UInstancedStaticMeshComponent* TreeC;
    UPROPERTY() UInstancedStaticMeshComponent* TreeD;
    UPROPERTY() UInstancedStaticMeshComponent* TreeE;
    UPROPERTY() UInstancedStaticMeshComponent* PineA;
    UPROPERTY() UInstancedStaticMeshComponent* PineB;

    UPROPERTY() UInstancedStaticMeshComponent* StreetLight;
    UPROPERTY() UInstancedStaticMeshComponent* PowerPole;
    UPROPERTY() UInstancedStaticMeshComponent* BridgeA;
    UPROPERTY() UInstancedStaticMeshComponent* BridgeB;
    UPROPERTY() UInstancedStaticMeshComponent* BridgeC;
    UPROPERTY() UInstancedStaticMeshComponent* BridgeD;
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
