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

    // R12.2 QA daylight. The previous warm low-sun pass made every material read as orange/brown and hid
    // whether the imported assets were actually working. Use neutral daylight first; cinematic grading comes later.
    SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
    SunLight->SetupAttachment(SceneRoot);
    SunLight->SetMobility(EComponentMobility::Movable);
    SunLight->SetRelativeRotation(FRotator(-45.0f, -30.0f, 0.0f));
    SunLight->SetIntensity(4.0f);
    SunLight->SetLightColor(FLinearColor(1.0f, 1.0f, 1.0f));
    SunLight->SetAtmosphereSunLight(true);
    SunLight->SetAtmosphereSunLightIndex(0);
    SunLight->SetLightSourceAngle(0.5357f);
    SunLight->SetDynamicShadowCascades(3);
    SunLight->SetDynamicShadowDistanceMovableLight(36000.0f);

    SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
    SkyAtmosphere->SetupAttachment(SceneRoot);
    SkyAtmosphere->SetRayleighScatteringScale(1.0f);
    SkyAtmosphere->SetMieScatteringScale(0.12f);
    SkyAtmosphere->SetMieAnisotropy(0.72f);
    SkyAtmosphere->SetMultiScatteringFactor(1.0f);
    SkyAtmosphere->SetGroundAlbedo(FColor(82, 92, 74));
    SkyAtmosphere->SetHeightFogContribution(0.18f);

    SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
    SkyLight->SetupAttachment(SceneRoot);
    SkyLight->SetMobility(EComponentMobility::Movable);
    SkyLight->SetIntensity(0.68f);
    SkyLight->SetLightColor(FLinearColor(0.98f, 0.99f, 1.0f));
    SkyLight->SetLowerHemisphereColor(FLinearColor(0.055f, 0.065f, 0.05f));
    SkyLight->SetRealTimeCaptureEnabled(true);

    HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFog"));
    HeightFog->SetupAttachment(SceneRoot);
    HeightFog->SetFogDensity(0.0006f);
    HeightFog->SetFogHeightFalloff(0.28f);
    HeightFog->SetFogInscatteringColor(FLinearColor(0.86f, 0.90f, 0.94f));
    HeightFog->SetDirectionalInscatteringColor(FLinearColor(1.0f, 0.99f, 0.96f));
    HeightFog->SetStartDistance(10000.0f);
    HeightFog->SetFogMaxOpacity(0.18f);
    HeightFog->SetVolumetricFog(false);
}
