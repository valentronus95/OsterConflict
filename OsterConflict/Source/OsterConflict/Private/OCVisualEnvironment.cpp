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

    // R13 neutral daylight pass. The previous custom atmosphere used too little Rayleigh and far too much Mie
    // scattering for this source-only scene, washing the entire runtime into yellow/amber. Use a clean white sun
    // and near-Earth scattering balance so sky/ground/material colours remain distinguishable.
    SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
    SunLight->SetupAttachment(SceneRoot);
    SunLight->SetMobility(EComponentMobility::Movable);
    SunLight->SetRelativeRotation(FRotator(-52.0f, -32.0f, 0.0f));
    SunLight->SetIntensity(3.6f);
    SunLight->SetLightColor(FLinearColor::White);
    SunLight->SetAtmosphereSunLight(true);
    SunLight->SetAtmosphereSunLightIndex(0);
    SunLight->SetLightSourceAngle(0.5357f);
    SunLight->SetDynamicShadowCascades(3);
    SunLight->SetDynamicShadowDistanceMovableLight(36000.0f);

    SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
    SkyAtmosphere->SetupAttachment(SceneRoot);
    SkyAtmosphere->SetRayleighScatteringScale(1.0f);
    SkyAtmosphere->SetMieScatteringScale(0.004f);
    SkyAtmosphere->SetMieAnisotropy(0.80f);
    SkyAtmosphere->SetMultiScatteringFactor(1.0f);
    SkyAtmosphere->SetGroundAlbedo(FColor(80, 82, 78));
    SkyAtmosphere->SetHeightFogContribution(0.0f);

    SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
    SkyLight->SetupAttachment(SceneRoot);
    SkyLight->SetMobility(EComponentMobility::Movable);
    SkyLight->SetIntensity(0.72f);
    SkyLight->SetLightColor(FLinearColor::White);
    SkyLight->SetLowerHemisphereColor(FLinearColor(0.075f, 0.085f, 0.075f));
    SkyLight->SetRealTimeCaptureEnabled(true);

    // Keep the component for later weather work, but do not tint or flatten the current clear-weather art pass.
    HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFog"));
    HeightFog->SetupAttachment(SceneRoot);
    HeightFog->SetFogDensity(0.0f);
    HeightFog->SetFogHeightFalloff(0.30f);
    HeightFog->SetStartDistance(30000.0f);
    HeightFog->SetFogMaxOpacity(0.0f);
    HeightFog->SetVolumetricFog(false);
}
