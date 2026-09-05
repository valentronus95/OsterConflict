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

    SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
    SunLight->SetupAttachment(SceneRoot);
    SunLight->SetMobility(EComponentMobility::Movable);
    SunLight->SetRelativeRotation(FRotator(-58.0f, -28.0f, 0.0f));
    // Pass45 P0: Directional Light intensity is lux in UE 5.8. The previous 4 lux value was
    // incompatible with the project's disabled exposure adaptation and could leave the outdoor
    // world effectively unlit. This physical daylight value is paired with AutoExposure=True in
    // DefaultEngine.ini; neither side of that pair may be changed independently.
    SunLight->SetIntensity(120000.0f);
    SunLight->SetLightColor(FLinearColor::White);
    SunLight->SetAtmosphereSunLight(true);
    SunLight->SetAtmosphereSunLightIndex(0);
    SunLight->SetLightSourceAngle(0.5357f);
    // Pass 14 folds in the conservative distant-shadow stabilization: more resolution inside a
    // shorter useful range instead of spending movable-shadow work on 300 m of tiny geometry.
    SunLight->SetDynamicShadowCascades(4);
    SunLight->SetDynamicShadowDistanceMovableLight(18000.0f);

    SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
    SkyAtmosphere->SetupAttachment(SceneRoot);
    SkyAtmosphere->SetRayleighScatteringScale(1.0f);
    SkyAtmosphere->SetRayleighScattering(FLinearColor(0.005802f, 0.013558f, 0.033100f));
    SkyAtmosphere->SetRayleighExponentialDistribution(8.0f);
    SkyAtmosphere->SetMieScatteringScale(1.0f);
    SkyAtmosphere->SetMieScattering(FLinearColor(0.003996f, 0.003996f, 0.003996f));
    SkyAtmosphere->SetMieAnisotropy(0.80f);
    SkyAtmosphere->SetMultiScatteringFactor(1.0f);
    SkyAtmosphere->SetGroundAlbedo(FColor(96, 96, 96));
    SkyAtmosphere->SetHeightFogContribution(0.0f);
    SkyAtmosphere->SetSkyAndAerialPerspectiveLuminanceFactor(FLinearColor(0.94f, 1.00f, 1.08f));

    SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
    SkyLight->SetupAttachment(SceneRoot);
    SkyLight->SetMobility(EComponentMobility::Movable);
    SkyLight->SetIntensity(0.85f);
    SkyLight->SetLightColor(FLinearColor::White);
    SkyLight->SetLowerHemisphereColor(FLinearColor(0.055f, 0.065f, 0.055f));
    // The Oster daylight rig is static during a match. Continuous cubemap recapture was pure GPU
    // overhead, especially painful on the 4-7 FPS playtest. Registration performs the initial capture.
    SkyLight->SetRealTimeCaptureEnabled(false);

    HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFog"));
    HeightFog->SetupAttachment(SceneRoot);
    HeightFog->SetFogDensity(0.0f);
    HeightFog->SetFogHeightFalloff(0.20f);
    HeightFog->SetFogInscatteringColor(FLinearColor(0.72f, 0.79f, 0.88f));
    HeightFog->SetDirectionalInscatteringColor(FLinearColor::White);
    HeightFog->SetStartDistance(2500.0f);
    HeightFog->SetFogMaxOpacity(0.0f);
    HeightFog->SetVolumetricFog(false);

    UE_LOG(LogTemp, Display,
        TEXT("PASS14_RENDER_BUDGET_READY shadow_cascades=4 shadow_cm=18000 skylight_realtime=0"));
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_DAYLIGHT_EXPOSURE_CONTRACT_READY sun_lux=120000 expected_auto_exposure=1 component_owned=1 replicated=1"));
}
