#include "OCVisualEnvironment.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"

AOCVisualEnvironment::AOCVisualEnvironment()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    bAlwaysRelevant = true;
    SetReplicateMovement(false);

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    // Neutral overcast/daylight baseline. Keep the sun bright enough for an outdoor FPS,
    // but do not bake a warm amber tint into every material in the scene.
    SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
    SunLight->SetupAttachment(SceneRoot);
    SunLight->SetMobility(EComponentMobility::Movable);
    SunLight->SetRelativeRotation(FRotator(-38.0f, -28.0f, 0.0f));
    SunLight->SetIntensity(70000.0f);
    SunLight->SetLightColor(FLinearColor(1.0f, 0.985f, 0.96f));
    SunLight->SetAtmosphereSunLight(true);
    SunLight->SetAtmosphereSunLightIndex(0);
    SunLight->SetLightSourceAngle(0.5357f);
    SunLight->SetDynamicShadowCascades(3);
    SunLight->SetDynamicShadowDistanceMovableLight(30000.0f);

    SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
    SkyAtmosphere->SetupAttachment(SceneRoot);
    SkyAtmosphere->SetRayleighScatteringScale(1.0f);
    SkyAtmosphere->SetMieScatteringScale(0.35f);
    SkyAtmosphere->SetMieAnisotropy(0.72f);
    SkyAtmosphere->SetMultiScatteringFactor(1.0f);
    SkyAtmosphere->SetGroundAlbedo(FColor(78, 86, 72));
    SkyAtmosphere->SetHeightFogContribution(1.0f);

    SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
    SkyLight->SetupAttachment(SceneRoot);
    SkyLight->SetMobility(EComponentMobility::Movable);
    SkyLight->SetIntensity(1.0f);
    SkyLight->SetLightColor(FLinearColor(0.94f, 0.97f, 1.0f));
    SkyLight->SetLowerHemisphereColor(FLinearColor(0.075f, 0.085f, 0.07f));
    SkyLight->SetRealTimeCaptureEnabled(true);

    HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFog"));
    HeightFog->SetupAttachment(SceneRoot);
    HeightFog->SetFogDensity(0.0060f);
    HeightFog->SetFogHeightFalloff(0.20f);
    HeightFog->SetFogInscatteringColor(FLinearColor(0.67f, 0.72f, 0.78f));
    HeightFog->SetDirectionalInscatteringColor(FLinearColor(0.92f, 0.93f, 0.91f));
    HeightFog->SetStartDistance(2500.0f);
    HeightFog->SetFogMaxOpacity(0.62f);
    HeightFog->SetVolumetricFog(false);
}
