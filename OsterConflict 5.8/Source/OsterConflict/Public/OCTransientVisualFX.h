#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OCTransientVisualFX.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UPointLightComponent;
class UMaterialInstanceDynamic;

/** Lightweight source-only replacement for DrawDebug combat visuals. */
UCLASS(NotBlueprintable)
class OSTERCONFLICT_API AOCTransientVisualFX : public AActor
{
    GENERATED_BODY()

public:
    AOCTransientVisualFX();

    void ConfigureTracer(const FVector& Start, const FVector& End, const FLinearColor& Color,
        float RadiusCm = 0.7f, float LifetimeSeconds = 0.055f);
    void ConfigureImpact(const FVector& Location, const FVector& Normal, const FLinearColor& Color,
        float RadiusCm = 5.0f, float LifetimeSeconds = 0.12f);
    void ConfigureMuzzle(const FVector& Location, const FVector& Direction, const FLinearColor& Color,
        float LifetimeSeconds = 0.045f);

private:
    UPROPERTY() TObjectPtr<USceneComponent> SceneRoot;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Mesh;
    UPROPERTY() TObjectPtr<UPointLightComponent> PointLight;
    UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

    void ApplyColor(const FLinearColor& Color);
};
