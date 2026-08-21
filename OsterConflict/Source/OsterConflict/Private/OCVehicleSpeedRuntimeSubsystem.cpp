#include "OCVehicleSpeedRuntimeSubsystem.h"

#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "OCBTR.h"
#include "OCPickupGunTruck.h"
#include "OCVehicleBase.h"
#include "UObject/UnrealType.h"

void UOCVehicleSpeedRuntimeSubsystem::Tick(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World || !World->IsGameWorld() || World->GetNetMode() == NM_Client)
    {
        return;
    }

    for (TActorIterator<AOCPickupGunTruck> It(World); It; ++It)
    {
        if (AOCPickupGunTruck* Vehicle = *It)
        {
            ApplySpeedContract(*Vehicle, 120.0f, 550.0f);
        }
    }

    for (TActorIterator<AOCBTR> It(World); It; ++It)
    {
        if (AOCBTR* Vehicle = *It)
        {
            ApplySpeedContract(*Vehicle, 90.0f, 320.0f);
        }
    }
}

TStatId UOCVehicleSpeedRuntimeSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCVehicleSpeedRuntimeSubsystem, STATGROUP_Tickables);
}

void UOCVehicleSpeedRuntimeSubsystem::ApplySpeedContract(AOCVehicleBase& Vehicle, float TargetKmh,
    float AssistAccelerationCmPerSecSq)
{
    if (Vehicle.IsVehicleDestroyed())
    {
        return;
    }

    UPrimitiveComponent* Body = Cast<UPrimitiveComponent>(Vehicle.GetRootComponent());
    if (!Body || !Body->IsSimulatingPhysics())
    {
        return;
    }

    // Keep the base vehicle's own limiter/HUD contract aligned with the current acceptance target.
    if (FFloatProperty* MaxSpeedProperty = FindFProperty<FFloatProperty>(AOCVehicleBase::StaticClass(), TEXT("MaxForwardSpeedKmh")))
    {
        if (float* MaxSpeedValue = MaxSpeedProperty->ContainerPtrToValuePtr<float>(&Vehicle))
        {
            *MaxSpeedValue = TargetKmh;
        }
    }

    const float PowerScale = DamagePowerScale(Vehicle);
    if (PowerScale <= KINDA_SMALL_NUMBER)
    {
        return;
    }

    const float EffectiveTargetKmh = FMath::Max(1.0f, TargetKmh * PowerScale);
    const float EffectiveTargetCmPerSec = EffectiveTargetKmh / 0.036f;
    const FVector Forward = Vehicle.GetActorForwardVector().GetSafeNormal();
    FVector Velocity = Body->GetPhysicsLinearVelocity(NAME_None);
    float ForwardSpeedCmPerSec = FVector::DotProduct(Velocity, Forward);

    // The source-only vehicle foundation historically treated MaxForwardSpeedKmh as a force falloff,
    // not a true cap. Clamp only the forward component so slopes/jumps/lateral motion remain physical.
    if (ForwardSpeedCmPerSec > EffectiveTargetCmPerSec)
    {
        Velocity += Forward * (EffectiveTargetCmPerSec - ForwardSpeedCmPerSec);
        Body->SetPhysicsLinearVelocity(Velocity, false, NAME_None);
        ForwardSpeedCmPerSec = EffectiveTargetCmPerSec;
    }

    const float Throttle = Vehicle.GetThrottleInput();
    if (Throttle <= 0.01f || Vehicle.IsHandbrakeApplied() || !HasGroundContact(Vehicle, *Body))
    {
        return;
    }

    const float ForwardSpeedKmh = FMath::Max(0.0f, ForwardSpeedCmPerSec * 0.036f);
    const float SpeedDeficitKmh = EffectiveTargetKmh - ForwardSpeedKmh;
    if (SpeedDeficitKmh <= 0.0f)
    {
        return;
    }

    // Compensate for the old cm/s-squared aerodynamic term that otherwise holds these vehicles
    // near roughly half their configured top speed. The assist fades out over the last 30 km/h.
    const float AssistAlpha = FMath::Clamp(SpeedDeficitKmh / 30.0f, 0.0f, 1.0f);
    const float MassKg = FMath::Max(1.0f, Body->GetMass());
    const FVector AssistForce = Forward * MassKg * AssistAccelerationCmPerSecSq * Throttle * AssistAlpha * PowerScale;
    Body->AddForce(AssistForce, NAME_None, false);
}

bool UOCVehicleSpeedRuntimeSubsystem::HasGroundContact(const AOCVehicleBase& Vehicle, const UPrimitiveComponent& Body) const
{
    UWorld* World = Vehicle.GetWorld();
    if (!World)
    {
        return false;
    }

    const FVector Up = Vehicle.GetActorUpVector().GetSafeNormal();
    const FVector Start = Body.GetComponentLocation();
    const float TraceDistance = FMath::Max(220.0f, Body.Bounds.BoxExtent.Z + 170.0f);
    const FVector End = Start - Up * TraceDistance;

    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(OCVehicleSpeedGround), false, &Vehicle);
    Params.AddIgnoredActor(&Vehicle);
    return World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
}

float UOCVehicleSpeedRuntimeSubsystem::DamagePowerScale(const AOCVehicleBase& Vehicle)
{
    switch (Vehicle.GetDamageStage())
    {
    case EOCVehicleDamageStage::Damaged: return 0.92f;
    case EOCVehicleDamageStage::Heavy: return 0.74f;
    case EOCVehicleDamageStage::Critical: return 0.48f;
    case EOCVehicleDamageStage::Wrecked: return 0.0f;
    default: return 1.0f;
    }
}
