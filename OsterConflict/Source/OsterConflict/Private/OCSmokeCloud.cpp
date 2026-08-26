#include "OCSmokeCloud.h"

#include "Components/SceneComponent.h"

AOCSmokeCloud::AOCSmokeCloud()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    SetReplicateMovement(false);

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    // Pass45 runtime evidence rejected the old cluster of Engine BasicShape spheres as smoke.
    // Until an authored particle/Niagara payload exists in the repository, this actor remains gameplay-only.
    // Missing visual content is intentionally visible in logs instead of being disguised with primitive geometry.
}

void AOCSmokeCloud::BeginPlay()
{
    Super::BeginPlay();
    SetLifeSpan(LifetimeSeconds);

    UE_LOG(LogTemp, Error,
        TEXT("PASS45_SMOKE_VFX_CONTENT_GAP authored_vfx=0 primitive_visible=0 gameplay_occlusion=1 radius_cm=%.1f lifetime_s=%.1f runtime_acceptance=0"),
        SmokeRadiusCm, LifetimeSeconds);
}

bool AOCSmokeCloud::ContainsPoint(const FVector& WorldPoint) const
{
    return FVector::DistSquared2D(GetActorLocation(), WorldPoint) <= FMath::Square(SmokeRadiusCm);
}
