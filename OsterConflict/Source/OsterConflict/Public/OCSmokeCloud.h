#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OCSmokeCloud.generated.h"

class UNiagaraComponent;
class USceneComponent;

/**
 * Replicated gameplay smoke volume. Radius/height are maximum authoritative extents; the effective gameplay
 * volume expands from detonation instead of becoming full-size invisibly on frame zero. Visible presentation is
 * supplied by the imported authored Niagara donor. Missing VFX fails closed: no primitive sphere/cube substitute
 * is ever rendered as smoke. Exact visual/gameplay expansion matching remains a UE 5.8 runtime acceptance task.
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
    float GetSmokeExpansionSeconds() const { return SmokeExpansionSeconds; }

    UFUNCTION(BlueprintPure, Category="Smoke")
    bool ContainsPoint(const FVector& WorldPoint) const;

protected:
    UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> SceneRoot;
    UPROPERTY(VisibleAnywhere, Category="Smoke|VFX") TObjectPtr<UNiagaraComponent> SmokeVFX;
    UPROPERTY(EditDefaultsOnly, Category="Smoke") float SmokeRadiusCm = 620.0f;
    UPROPERTY(EditDefaultsOnly, Category="Smoke") float SmokeHalfHeightCm = 450.0f;
    // Source/gameplay default only. Final value must be calibrated against the authored Niagara in local UE 5.8.
    UPROPERTY(EditDefaultsOnly, Category="Smoke") float SmokeExpansionSeconds = 3.0f;
    UPROPERTY(EditDefaultsOnly, Category="Smoke") float LifetimeSeconds = 18.0f;
};