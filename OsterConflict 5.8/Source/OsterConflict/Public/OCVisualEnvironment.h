#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OCVisualEnvironment.generated.h"

class USceneComponent;
class UDirectionalLightComponent;
class USkyLightComponent;
class USkyAtmosphereComponent;
class UExponentialHeightFogComponent;

/**
 * R11 source-only daylight rig.
 *
 * The release map is intentionally generated empty, so the runtime gameplay world
 * must also provide its own lighting/atmosphere.  Keeping this in C++ makes the
 * Launcher build usable without requiring content assets just to see the level.
 */
UCLASS()
class OSTERCONFLICT_API AOCVisualEnvironment : public AActor
{
    GENERATED_BODY()

public:
    AOCVisualEnvironment();

private:
    UPROPERTY() TObjectPtr<USceneComponent> SceneRoot;
    UPROPERTY() TObjectPtr<UDirectionalLightComponent> SunLight;
    UPROPERTY() TObjectPtr<USkyLightComponent> SkyLight;
    UPROPERTY() TObjectPtr<USkyAtmosphereComponent> SkyAtmosphere;
    UPROPERTY() TObjectPtr<UExponentialHeightFogComponent> HeightFog;
};
