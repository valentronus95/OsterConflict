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
    SunLight->SetRelativeRotation(FRotator(-38.0f, -28.0f, 0.0f));
    SunLight->SetIntensity(85000.0f);
    SunLight->SetLightColor(FLinearColor(1.0f, 0.94f, 0.84f));
    SunLight->SetAtmosphereSunLight(true);
    SunLight->SetAtmosphereSunLightIndex(0);
    SunLight->SetLightSourceAngle(0.5357f);
    SunLight->SetDynamicShadowCascades(3);
    SunLight->SetDynamicShadowDistanceMovableLight(30000.0f);

    SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
    SkyAtmosphere->SetupAttachment(SceneRoot);
    SkyAtmosphere->SetRayleighScatteringScale(1.0f);
    SkyAtmosphere->SetMieScatteringScale(0.85f);
    SkyAtmosphere->SetMieAnisotropy(0.78f);
    SkyAtmosphere->SetMultiScatteringFactor(1.0f);
    SkyAtmosphere->SetGroundAlbedo(FColor(92, 104, 73));
    SkyAtmosphere->SetHeightFogContribution(1.0f);

    SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
    SkyLight->SetupAttachment(SceneRoot);
    SkyLight->SetMobility(EComponentMobility::Movable);
    SkyLight->SetIntensity(0.85f);
    SkyLight->SetLightColor(FLinearColor(0.82f, 0.90f, 1.0f));
    SkyLight->SetLowerHemisphereColor(FLinearColor(0.09f, 0.11f, 0.08f));
    SkyLight->SetRealTimeCaptureEnabled(true);

    HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFog"));
    HeightFog->SetupAttachment(SceneRoot);
    HeightFog->SetFogDensity(0.0085f);
    HeightFog->SetFogHeightFalloff(0.20f);
    HeightFog->SetFogInscatteringColor(FLinearColor(0.63f, 0.70f, 0.77f));
    HeightFog->SetDirectionalInscatteringColor(FLinearColor(1.0f, 0.88f, 0.72f));
    HeightFog->SetStartDistance(2500.0f);
    HeightFog->SetFogMaxOpacity(0.72f);
    HeightFog->SetVolumetricFog(false);
}
