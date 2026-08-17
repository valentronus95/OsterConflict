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

    // Neutral daytime sun. The world is still source-built, so keep the conservative
    // intensity that matches the project's exposure assumptions, but move the sun higher
    // and remove all warm light tinting.
    SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
    SunLight->SetupAttachment(SceneRoot);
    SunLight->SetMobility(EComponentMobility::Movable);
    SunLight->SetRelativeRotation(FRotator(-58.0f, -28.0f, 0.0f));
    SunLight->SetIntensity(4.0f);
    SunLight->SetLightColor(FLinearColor::White);
    SunLight->SetAtmosphereSunLight(true);
    SunLight->SetAtmosphereSunLightIndex(0);
    SunLight->SetLightSourceAngle(0.5357f);
    SunLight->SetDynamicShadowCascades(3);
    SunLight->SetDynamicShadowDistanceMovableLight(30000.0f);

    SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
    SkyAtmosphere->SetupAttachment(SceneRoot);

    // Explicit Earth-like scattering coefficients. The former R13 pass reduced Mie scale
    // almost to zero while retaining an olive ground albedo, producing the flat mustard/yellow
    // sky seen in the gameplay test. Rayleigh is wavelength dependent and gives daylight its
    // readable blue sky; Mie remains neutral rather than acting as a warm colour grade.
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
    SkyLight->SetRealTimeCaptureEnabled(true);

    HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFog"));
    HeightFog->SetupAttachment(SceneRoot);

    // Keep height fog out of the current art-QA pass. It can be reintroduced once the
    // environment assets and daylight are stable, rather than hiding colour problems in haze.
    HeightFog->SetFogDensity(0.0f);
    HeightFog->SetFogHeightFalloff(0.20f);
    HeightFog->SetFogInscatteringColor(FLinearColor(0.72f, 0.79f, 0.88f));
    HeightFog->SetDirectionalInscatteringColor(FLinearColor::White);
    HeightFog->SetStartDistance(2500.0f);
    HeightFog->SetFogMaxOpacity(0.0f);
    HeightFog->SetVolumetricFog(false);
}
