#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OCWorldSectorOster.generated.h"

class UInstancedStaticMeshComponent;
class USceneComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

/**
 * S16A reference-driven source-only layout of central Oster, Chernihiv Oblast.
 *
 * Key public landmarks are now modeled from multiple public photo/video references rather than generic blocks:
 * - Solonyna house / Oster Local History Museum
 * - Oster central stadium
 * - Oster Vocational College of Construction and Design
 * - Oster city park
 * - Silpo supermarket at Bohdana Khmelnytskoho 54 (dedicated branch replacement pass)
 *
 * This remains a gameplay reconstruction, NOT survey-grade architectural documentation.
 * Public references determine silhouette, storey count, facade rhythm, roof character, site relationships,
 * vegetation character and major outdoor features. Private residences use reference-informed archetypes and are
 * not claimed to reproduce individual homes.
 *
 * Unreal units are centimeters. +X = east, +Y = north.
 */
UCLASS()
class OSTERCONFLICT_API AOCWorldSectorOster : public AActor
{
    GENERATED_BODY()

public:
    AOCWorldSectorOster();
    virtual void BeginPlay() override;

    static FVector MuseumAnchor();
    static FVector CollegeAnchor();
    static FVector ParkAnchor();
    static FVector CultureParkNorthAnchor();
    static FVector FormerCityAdministrationAnchor();
    static FVector HistoricCourtAnchor();
    static FVector ResurrectionChurchAnchor();
    static FVector StadiumAnchor();
    static FVector KrushelnytskaEnterableHouseAnchor();
    static float KrushelnytskaEnterableHouseYaw();

private:
    UPROPERTY() TObjectPtr<USceneComponent> SceneRoot;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Ground;

    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Roads;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Sidewalks;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Buildings;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> ResidentialRoofs;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> ResidentialDetails;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> LandmarkBlocks;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> LandmarkRoofs;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> LandmarkWindows;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> LandmarkDetails;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Fences;
    // S16B private-sector fence families. Public/landmark fencing remains in Fences.
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> WoodFences;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> MetalFences;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> LightSheetFences;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> TreeTrunks;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> TreeCrowns;
    // S16B source-only vegetation families. Final foliage meshes/materials are content assets.
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> SovietPoplarTrunks;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> SovietPoplarCrowns;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> BirchTrunks;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> BirchCrowns;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> PineTrunks;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> PineCrowns;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> GrassMown;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> GrassRough;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> GrassWetland;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> StadiumGeometry;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> StadiumDetails;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> ParkGeometry;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> ParkDetails;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Waterways;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Bridges;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> ReferenceMarkers;

    UPROPERTY() TObjectPtr<UTextRenderComponent> MuseumLabel;
    UPROPERTY() TObjectPtr<UTextRenderComponent> StadiumLabel;
    UPROPERTY() TObjectPtr<UTextRenderComponent> ParkLabel;
    UPROPERTY() TObjectPtr<UTextRenderComponent> CollegeLabel;
    UPROPERTY() TObjectPtr<UTextRenderComponent> KrushelnytskaStreetLabel;

    void BuildRoadNetwork();
    void BuildHydrography();
    void BuildVerifiedReferenceMarkers();
    void BuildMuseumAndStadium();
    void BuildCentralPark();
    void BuildCollegeSector();
    void BuildSolomiiKrushelnytskoiStreet();
    void BuildResidentialBlocks();
    void BuildVegetation();
    void BuildGameplayBases();

    static void AddBox(UInstancedStaticMeshComponent* Component, const FVector& Center, const FVector& SizeCm,
        float YawDegrees = 0.0f);
    static void AddBoxRotated(UInstancedStaticMeshComponent* Component, const FVector& Center, const FVector& SizeCm,
        const FRotator& Rotation);
    static void AddCylinder(UInstancedStaticMeshComponent* Component, const FVector& Center, float RadiusCm,
        float HeightCm);
    static void AddGableRoof(UInstancedStaticMeshComponent* Component, const FVector& Center, float WidthCm,
        float DepthCm, float RidgeZCm, float YawDegrees, float SlopeDegrees = 28.0f);
    static void AddFacadeWindow(UInstancedStaticMeshComponent* Component, const FVector& BuildingCenter,
        const FVector& LocalOffset, const FVector& SizeCm, float BuildingYawDegrees, bool bFrontFacade = true);
    static void ConfigureLabel(UTextRenderComponent* Label, const FString& Text, const FVector& Location);
};
