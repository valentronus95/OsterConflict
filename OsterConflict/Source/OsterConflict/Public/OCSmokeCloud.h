#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OCSmokeCloud.generated.h"

class USceneComponent;

/**
 * Replicated gameplay smoke volume. Pass45 intentionally renders no primitive stand-in: final smoke
 * requires accepted authored particle/Niagara content, while AI/gameplay can continue querying this radius.
 */
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
    UPROPERTY(EditDefaultsOnly, Category="Smoke") float SmokeRadiusCm = 620.0f;
    UPROPERTY(EditDefaultsOnly, Category="Smoke") float LifetimeSeconds = 18.0f;
};
