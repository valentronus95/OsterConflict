#include "OCSmokeCloud.h"

#include "Components/SceneComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "UObject/SoftObjectPath.h"

namespace
{
    constexpr const TCHAR* Pass45SmokeNiagaraPath =
        TEXT("/Game/PotaVFX_Smoke/VFX/System/ColorSmoke/NS_SmokeGradient_Loop.NS_SmokeGradient_Loop");
}

AOCSmokeCloud::AOCSmokeCloud()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    SetReplicateMovement(false);

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    SmokeVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SmokeVFX"));
    SmokeVFX->SetupAttachment(SceneRoot);
    SmokeVFX->SetAutoActivate(false);
    SmokeVFX->SetIsReplicated(false);

    // Pass45 runtime evidence rejected Engine BasicShape spheres as a smoke substitute.
    // The imported Niagara system is the sole visible presentation owner; load failure stays visually fail-closed.
}

void AOCSmokeCloud::BeginPlay()
{
    Super::BeginPlay();
    SetLifeSpan(LifetimeSeconds);

    if (GetNetMode() == NM_DedicatedServer)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_SMOKE_VFX_SERVER_SKIP gameplay_occlusion=1 finite_volume=1 gameplay_volume_expands=1 radius_cm=%.1f half_height_cm=%.1f expansion_s=%.1f lifetime_s=%.1f runtime_acceptance=0"),
            SmokeRadiusCm, SmokeHalfHeightCm, SmokeExpansionSeconds, LifetimeSeconds);
        return;
    }

    // GAME_RECOVERY: the smoke Niagara package is async-preloaded before deployment release. ResolveObject is
    // lookup-only; never synchronously load the package when the first smoke grenade detonates.
    UNiagaraSystem* SmokeSystem = Cast<UNiagaraSystem>(FSoftObjectPath(Pass45SmokeNiagaraPath).ResolveObject());
    if (!SmokeSystem)
    {
        SmokeVFX->DeactivateImmediate();
        UE_LOG(LogTemp, Error,
            TEXT("GAME_RECOVERY_GRENADE_PRELOAD_MISS asset=%s phase=smoke_vfx sync_package_loads=0 primitive_visible=0 gameplay_occlusion=1 finite_volume=1 runtime_acceptance=0"),
            Pass45SmokeNiagaraPath);
        return;
    }

    SmokeVFX->SetAsset(SmokeSystem);
    SmokeVFX->Activate(true);
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_SMOKE_VFX_DONOR_WIRED asset=%s authored_niagara=1 primitive_visible=0 gameplay_occlusion=1 finite_volume=1 gameplay_volume_expands=1 radius_cm=%.1f half_height_cm=%.1f expansion_s=%.1f lifetime_s=%.1f sync_package_loads=0 runtime_acceptance=0"),
        Pass45SmokeNiagaraPath, SmokeRadiusCm, SmokeHalfHeightCm, SmokeExpansionSeconds, LifetimeSeconds);

    // Automated runtime readiness proves only that the authored Niagara payload was already resident and was activated
    // in a real gameplay client. It deliberately does NOT claim exact visual/gameplay expansion synchronization or
    // that smoke scale/look/performance was visually accepted.
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_SMOKE_VFX_RUNTIME_READY asset=%s runtime_loaded=1 activated=1 primitive_visible=0 gameplay_occlusion=1 finite_volume=1 gameplay_volume_expands=1 expansion_s=%.1f exact_visual_sync=0 manual_visual_acceptance=0 sync_package_loads=0"),
        Pass45SmokeNiagaraPath, SmokeExpansionSeconds);
}

bool AOCSmokeCloud::ContainsPoint(const FVector& WorldPoint) const
{
    // No Tick is required. Game-time age makes each visibility query observe the same bounded expansion curve.
    // This prevents a full-size invisible occlusion volume from existing before the authored smoke has had time
    // to develop. The 3 s source default is deliberately not labelled visually calibrated until local UE 5.8.
    const float SafeExpansionSeconds = FMath::Max(0.05f, SmokeExpansionSeconds);
    const float ExpansionAlpha = FMath::Clamp(GetGameTimeSinceCreation() / SafeExpansionSeconds, 0.0f, 1.0f);
    const float EffectiveRadiusCm = SmokeRadiusCm * ExpansionAlpha;
    const float EffectiveHalfHeightCm = SmokeHalfHeightCm * ExpansionAlpha;

    const FVector Delta = WorldPoint - GetActorLocation();
    if (FMath::Abs(Delta.Z) > EffectiveHalfHeightCm) return false;
    return FVector2D(Delta.X, Delta.Y).SizeSquared() <= FMath::Square(EffectiveRadiusCm);
}