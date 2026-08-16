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

    // R12.1 exposure hotfix. The first low-sun R12 pass pushed the source-only runtime rig into a
    // white/yellow wash on the Launcher test path. Keep a late-afternoon direction, but use a deliberately
    // game-scaled light level and a much more neutral direct colour until a proper exposure/post-process
    // pipeline is authored. Readability wins over physically correct lux in this source-only test rig.
    SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
    SunLight->SetupAttachment(SceneRoot);
    SunLight->SetMobility(EComponentMobility::Movable);
    SunLight->SetRelativeRotation(FRotator(-34.0f, -48.0f, 0.0f));
    SunLight->SetIntensity(12.0f);
    SunLight->SetLightColor(FLinearColor(1.0f, 0.97f, 0.91f));
    SunLight->SetAtmosphereSunLight(true);
    SunLight->SetAtmosphereSunLightIndex(0);
    SunLight->SetLightSourceAngle(0.5357f);
    SunLight->SetDynamicShadowCascades(3);
    SunLight->SetDynamicShadowDistanceMovableLight(36000.0f);

    SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
    SkyAtmosphere->SetupAttachment(SceneRoot);
    SkyAtmosphere->SetRayleighScatteringScale(1.0f);
    SkyAtmosphere->SetMieScatteringScale(0.55f);
    SkyAtmosphere->SetMieAnisotropy(0.76f);
    SkyAtmosphere->SetMultiScatteringFactor(1.0f);
    SkyAtmosphere->SetGroundAlbedo(FColor(90, 96, 72));
    SkyAtmosphere->SetHeightFogContribution(0.65f);

    SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
    SkyLight->SetupAttachment(SceneRoot);
    SkyLight->SetMobility(EComponentMobility::Movable);
    SkyLight->SetIntensity(0.82f);
    SkyLight->SetLightColor(FLinearColor(0.90f, 0.94f, 1.0f));
    SkyLight->SetLowerHemisphereColor(FLinearColor(0.07f, 0.08f, 0.06f));
    SkyLight->SetRealTimeCaptureEnabled(true);

    HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFog"));
    HeightFog->SetupAttachment(SceneRoot);
    HeightFog->SetFogDensity(0.0025f);
    HeightFog->SetFogHeightFalloff(0.24f);
    HeightFog->SetFogInscatteringColor(FLinearColor(0.70f, 0.75f, 0.80f));
    HeightFog->SetDirectionalInscatteringColor(FLinearColor(1.0f, 0.94f, 0.84f));
    HeightFog->SetStartDistance(5500.0f);
    HeightFog->SetFogMaxOpacity(0.42f);
    HeightFog->SetVolumetricFog(false);
}
