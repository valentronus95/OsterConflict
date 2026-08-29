#include "OCSmokeCloud.h"

#include "Components/SceneComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

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
            TEXT("PASS45_SMOKE_VFX_SERVER_SKIP gameplay_occlusion=1 radius_cm=%.1f lifetime_s=%.1f runtime_acceptance=0"),
            SmokeRadiusCm, LifetimeSeconds);
        return;
    }

    UNiagaraSystem* SmokeSystem = LoadObject<UNiagaraSystem>(nullptr, Pass45SmokeNiagaraPath);
    if (!SmokeSystem)
    {
        SmokeVFX->DeactivateImmediate();
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_SMOKE_VFX_LOAD_FAIL asset=%s authored_niagara=0 primitive_visible=0 gameplay_occlusion=1 radius_cm=%.1f lifetime_s=%.1f runtime_acceptance=0"),
            Pass45SmokeNiagaraPath, SmokeRadiusCm, LifetimeSeconds);
        return;
    }

    SmokeVFX->SetAsset(SmokeSystem);
    SmokeVFX->Activate(true);
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_SMOKE_VFX_DONOR_WIRED asset=%s authored_niagara=1 primitive_visible=0 gameplay_occlusion=1 radius_cm=%.1f lifetime_s=%.1f runtime_acceptance=0"),
        Pass45SmokeNiagaraPath, SmokeRadiusCm, LifetimeSeconds);

    // Automated runtime readiness proves only that the authored Niagara payload loaded and was activated in a
    // real gameplay client. It deliberately does NOT claim that smoke scale/look/performance was visually accepted.
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_SMOKE_VFX_RUNTIME_READY asset=%s runtime_loaded=1 activated=1 primitive_visible=0 gameplay_occlusion=1 manual_visual_acceptance=0"),
        Pass45SmokeNiagaraPath);
}

bool AOCSmokeCloud::ContainsPoint(const FVector& WorldPoint) const
{
    return FVector::DistSquared2D(GetActorLocation(), WorldPoint) <= FMath::Square(SmokeRadiusCm);
}
