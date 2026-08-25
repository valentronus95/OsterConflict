#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OCHouseTypes.h"
#include "OCEnterableHouse.generated.h"

class UInstancedStaticMeshComponent;
class USceneComponent;
class UTextRenderComponent;

UCLASS()
class OSTERCONFLICT_API AOCEnterableHouse : public AActor
{
    GENERATED_BODY()

public:
    AOCEnterableHouse();
    virtual void BeginPlay() override;

    /** Rebuilds non-interactive household dressing deterministically. Gameplay interaction is intentionally absent. */
    void ConfigureInteriorVariantServer(int32 NewSeed, EOCHouseCondition NewCondition, int32 NewLayoutVariant);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="House") TObjectPtr<USceneComponent> SceneRoot;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="House") TObjectPtr<UInstancedStaticMeshComponent> Shell;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="House") TObjectPtr<UInstancedStaticMeshComponent> Interior;

    // Legacy cube groups are retained only for the small electronic/blockout details that do not yet
    // have an authored prop in the checked-in pack. Real furniture/clutter presentation is owned by
    // the dedicated mesh components below.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="House") TObjectPtr<UInstancedStaticMeshComponent> HouseholdFurniture;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="House") TObjectPtr<UInstancedStaticMeshComponent> HouseholdElectronics;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="House") TObjectPtr<UInstancedStaticMeshComponent> HouseholdClutter;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="House") TObjectPtr<UInstancedStaticMeshComponent> YardFences;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="House") TObjectPtr<UInstancedStaticMeshComponent> YardPaths;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="House|Models") TObjectPtr<UInstancedStaticMeshComponent> RealSofa;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="House|Models") TObjectPtr<UInstancedStaticMeshComponent> RealTable;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="House|Models") TObjectPtr<UInstancedStaticMeshComponent> RealPlasticChair;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="House|Models") TObjectPtr<UInstancedStaticMeshComponent> RealOfficeChair;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="House|Models") TObjectPtr<UInstancedStaticMeshComponent> RealFridge;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="House|Models") TObjectPtr<UInstancedStaticMeshComponent> RealCrate;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="House|Models") TObjectPtr<UInstancedStaticMeshComponent> RealMetalBarrel;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="House|Models") TObjectPtr<UInstancedStaticMeshComponent> RealWheelBarrow;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="House") TObjectPtr<UTextRenderComponent> DebugLabel;

    UPROPERTY(EditAnywhere, Category="House|Variation") int32 InteriorSeed = 1408;
    UPROPERTY(EditAnywhere, Category="House|Variation") EOCHouseCondition HouseCondition = EOCHouseCondition::Ordinary;
    UPROPERTY(EditAnywhere, Category="House|Variation", meta=(ClampMin="0", ClampMax="5")) int32 LayoutVariant = 0;

private:
    void BuildShell();
    void BuildInterior();
    void BuildHouseholdProps();
    void BuildYard();
    void SpawnInteractiveOpeningsServer();
    void ClearRealInteriorProps();

    static void AddBox(UInstancedStaticMeshComponent* Component, const FVector& Center,
        const FVector& SizeCm, float YawDegrees = 0.0f);
    static void AddChair(UInstancedStaticMeshComponent* Component, const FVector& Center, float YawDegrees);
    static void AddTable(UInstancedStaticMeshComponent* Component, const FVector& Center, const FVector2D& Size, float YawDegrees);
    static void AddSofa(UInstancedStaticMeshComponent* Component, const FVector& Center, float YawDegrees, float WidthCm);
    static void AddFittedGroundProp(UInstancedStaticMeshComponent* Component, const FVector& FootprintCenter,
        float TargetLongestDimensionCm, float YawDegrees, float GroundZCm = 0.0f);
    FTransform MakeWorldTransform(const FVector& LocalLocation, float LocalYawDegrees = 0.0f) const;
};