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

    // R13 neutral art-QA daylight. The prior 70,000 intensity value was far above
    // the exposure assumptions used by this source-only runtime scene and washed the
    // complete gameplay view to white. Keep the neutral colour/scattering fix while
    // restoring the proven readable source-lighting intensity.
    SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
    SunLight->SetupAttachment(SceneRoot);
    SunLight->SetMobility(EComponentMobility::Movable);
    SunLight->SetRelativeRotation(FRotator(-38.0f, -28.0f, 0.0f));
    SunLight->SetIntensity(4.0f);
    SunLight->SetLightColor(FLinearColor::White);
    SunLight->SetAtmosphereSunLight(true);
    SunLight->SetAtmosphereSunLightIndex(0);
    SunLight->SetLightSourceAngle(0.5357f);
    SunLight->SetDynamicShadowCascades(3);
    SunLight->SetDynamicShadowDistanceMovableLight(30000.0f);

    SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
    SkyAtmosphere->SetupAttachment(SceneRoot);
    SkyAtmosphere->SetRayleighScatteringScale(1.0f);
    SkyAtmosphere->SetMieScatteringScale(0.004f);
    SkyAtmosphere->SetMieAnisotropy(0.72f);
    SkyAtmosphere->SetMultiScatteringFactor(1.0f);
    SkyAtmosphere->SetGroundAlbedo(FColor(78, 86, 72));
    SkyAtmosphere->SetHeightFogContribution(0.0f);

    SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
    SkyLight->SetupAttachment(SceneRoot);
    SkyLight->SetMobility(EComponentMobility::Movable);
    SkyLight->SetIntensity(1.0f);
    SkyLight->SetLightColor(FLinearColor(0.94f, 0.97f, 1.0f));
    SkyLight->SetLowerHemisphereColor(FLinearColor(0.075f, 0.085f, 0.07f));
    SkyLight->SetRealTimeCaptureEnabled(true);

    HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFog"));
    HeightFog->SetupAttachment(SceneRoot);

    // Neutral R13 art-QA mode intentionally disables height fog so imported art can
    // be judged without another colour layer on top of it.
    HeightFog->SetFogDensity(0.0f);
    HeightFog->SetFogHeightFalloff(0.20f);
    HeightFog->SetFogInscatteringColor(FLinearColor(0.67f, 0.72f, 0.78f));
    HeightFog->SetDirectionalInscatteringColor(FLinearColor(0.92f, 0.93f, 0.91f));
    HeightFog->SetStartDistance(2500.0f);
    HeightFog->SetFogMaxOpacity(0.0f);
    HeightFog->SetVolumetricFog(false);
}
