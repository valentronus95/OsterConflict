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
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="House") TObjectPtr<UInstancedStaticMeshComponent> HouseholdFurniture;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="House") TObjectPtr<UInstancedStaticMeshComponent> HouseholdElectronics;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="House") TObjectPtr<UInstancedStaticMeshComponent> HouseholdClutter;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="House") TObjectPtr<UInstancedStaticMeshComponent> YardFences;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="House") TObjectPtr<UInstancedStaticMeshComponent> YardPaths;
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

    static void AddBox(UInstancedStaticMeshComponent* Component, const FVector& Center,
        const FVector& SizeCm, float YawDegrees = 0.0f);
    static void AddChair(UInstancedStaticMeshComponent* Component, const FVector& Center, float YawDegrees);
    static void AddTable(UInstancedStaticMeshComponent* Component, const FVector& Center, const FVector2D& Size, float YawDegrees);
    static void AddSofa(UInstancedStaticMeshComponent* Component, const FVector& Center, float YawDegrees, float WidthCm);
    FTransform MakeWorldTransform(const FVector& LocalLocation, float LocalYawDegrees = 0.0f) const;
};
