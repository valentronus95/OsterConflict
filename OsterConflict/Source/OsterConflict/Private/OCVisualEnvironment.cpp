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

    // R12 Krushelnytska reference pass: lower late-afternoon sun, warmer direct light and longer shadows.
    // This matches the current Oster street references better than the higher R11 test-light position.
    SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
    SunLight->SetupAttachment(SceneRoot);
    SunLight->SetMobility(EComponentMobility::Movable);
    SunLight->SetRelativeRotation(FRotator(-22.0f, -55.0f, 0.0f));
    SunLight->SetIntensity(70000.0f);
    SunLight->SetLightColor(FLinearColor(1.0f, 0.89f, 0.74f));
    SunLight->SetAtmosphereSunLight(true);
    SunLight->SetAtmosphereSunLightIndex(0);
    SunLight->SetLightSourceAngle(0.5357f);
    SunLight->SetDynamicShadowCascades(3);
    SunLight->SetDynamicShadowDistanceMovableLight(36000.0f);

    SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
    SkyAtmosphere->SetupAttachment(SceneRoot);
    SkyAtmosphere->SetRayleighScatteringScale(1.0f);
    SkyAtmosphere->SetMieScatteringScale(0.72f);
    SkyAtmosphere->SetMieAnisotropy(0.80f);
    SkyAtmosphere->SetMultiScatteringFactor(1.0f);
    SkyAtmosphere->SetGroundAlbedo(FColor(96, 102, 74));
    SkyAtmosphere->SetHeightFogContribution(0.85f);

    SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
    SkyLight->SetupAttachment(SceneRoot);
    SkyLight->SetMobility(EComponentMobility::Movable);
    SkyLight->SetIntensity(0.68f);
    SkyLight->SetLightColor(FLinearColor(0.82f, 0.89f, 1.0f));
    SkyLight->SetLowerHemisphereColor(FLinearColor(0.075f, 0.085f, 0.065f));
    SkyLight->SetRealTimeCaptureEnabled(true);

    HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFog"));
    HeightFog->SetupAttachment(SceneRoot);
    HeightFog->SetFogDensity(0.0045f);
    HeightFog->SetFogHeightFalloff(0.22f);
    HeightFog->SetFogInscatteringColor(FLinearColor(0.66f, 0.72f, 0.78f));
    HeightFog->SetDirectionalInscatteringColor(FLinearColor(1.0f, 0.84f, 0.66f));
    HeightFog->SetStartDistance(4000.0f);
    HeightFog->SetFogMaxOpacity(0.55f);
    HeightFog->SetVolumetricFog(false);
}
