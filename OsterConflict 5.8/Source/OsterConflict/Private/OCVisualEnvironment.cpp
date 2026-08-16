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

    // R13 diagnostic daylight: prioritise neutral material readability. The earlier atmospheric setup was technically
    // daylight but produced a persistent amber cast in the actual launcher build, making every asset look untextured.
    SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
    SunLight->SetupAttachment(SceneRoot);
    SunLight->SetMobility(EComponentMobility::Movable);
    SunLight->SetRelativeRotation(FRotator(-58.0f, -25.0f, 0.0f));
    SunLight->SetIntensity(2.25f);
    SunLight->SetLightColor(FLinearColor(0.96f, 0.985f, 1.0f));
    SunLight->SetAtmosphereSunLight(true);
    SunLight->SetAtmosphereSunLightIndex(0);
    SunLight->SetLightSourceAngle(0.5357f);
    SunLight->SetDynamicShadowCascades(3);
    SunLight->SetDynamicShadowDistanceMovableLight(36000.0f);

    SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
    SkyAtmosphere->SetupAttachment(SceneRoot);
    SkyAtmosphere->SetRayleighScatteringScale(0.52f);
    SkyAtmosphere->SetMieScatteringScale(0.015f);
    SkyAtmosphere->SetMieAnisotropy(0.55f);
    SkyAtmosphere->SetMultiScatteringFactor(0.65f);
    SkyAtmosphere->SetGroundAlbedo(FColor(62, 70, 58));
    SkyAtmosphere->SetHeightFogContribution(0.0f);

    SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
    SkyLight->SetupAttachment(SceneRoot);
    SkyLight->SetMobility(EComponentMobility::Movable);
    SkyLight->SetIntensity(0.42f);
    SkyLight->SetLightColor(FLinearColor(0.90f, 0.94f, 1.0f));
    SkyLight->SetLowerHemisphereColor(FLinearColor(0.035f, 0.045f, 0.035f));
    SkyLight->SetRealTimeCaptureEnabled(true);

    // Keep the component for later weather work, but disable the colour-washing fog during R13 art QA.
    HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFog"));
    HeightFog->SetupAttachment(SceneRoot);
    HeightFog->SetFogDensity(0.0f);
    HeightFog->SetFogHeightFalloff(0.30f);
    HeightFog->SetStartDistance(30000.0f);
    HeightFog->SetFogMaxOpacity(0.0f);
    HeightFog->SetVolumetricFog(false);
}
