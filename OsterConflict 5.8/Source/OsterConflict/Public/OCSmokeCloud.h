#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OCSmokeCloud.generated.h"

class USceneComponent;
class UStaticMeshComponent;

/** Source-only smoke stand-in. Gameplay/AI can query its radius; final visuals move to Niagara. */
UCLASS()
class OSTERCONFLICT_API AOCSmokeCloud : public AActor
{
    GENERATED_BODY()
public:
    AOCSmokeCloud();
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintPure, Category="Smoke")
    float GetSmokeRadiusCm() const { return SmokeRadiusCm; }

    UFUNCTION(BlueprintPure, Category="Smoke")
    bool ContainsPoint(const FVector& WorldPoint) const;

protected:
    UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> SceneRoot;
    UPROPERTY(VisibleAnywhere) TArray<TObjectPtr<UStaticMeshComponent>> Puffs;
    UPROPERTY(EditDefaultsOnly, Category="Smoke") float SmokeRadiusCm = 620.0f;
    UPROPERTY(EditDefaultsOnly, Category="Smoke") float LifetimeSeconds = 18.0f;
};
