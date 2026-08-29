#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OCSmokeCloud.generated.h"

class UNiagaraComponent;
class USceneComponent;

/**
 * Replicated gameplay smoke volume. The gameplay radius/height remain authoritative while the visible
 * presentation is supplied by the imported authored Niagara donor. Missing VFX fails closed: no
 * primitive sphere/cube substitute is ever rendered as smoke.
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
    float GetSmokeHalfHeightCm() const { return SmokeHalfHeightCm; }

    UFUNCTION(BlueprintPure, Category="Smoke")
    bool ContainsPoint(const FVector& WorldPoint) const;

protected:
    UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> SceneRoot;
    UPROPERTY(VisibleAnywhere, Category="Smoke|VFX") TObjectPtr<UNiagaraComponent> SmokeVFX;
    UPROPERTY(EditDefaultsOnly, Category="Smoke") float SmokeRadiusCm = 620.0f;
    UPROPERTY(EditDefaultsOnly, Category="Smoke") float SmokeHalfHeightCm = 450.0f;
    UPROPERTY(EditDefaultsOnly, Category="Smoke") float LifetimeSeconds = 18.0f;
};
