#pragma once

#include "CoreMinimal.h"
#include "Engine/NetSerialization.h"
#include "OCWeaponTypes.h"
#include "OCTraumaTypes.generated.h"

UENUM(BlueprintType)
enum class EOCBodyZone : uint8
{
    Unknown,
    HeadNeck,
    Torso,
    Pelvis,
    LeftArm,
    RightArm,
    LeftLeg,
    RightLeg
};

UENUM(BlueprintType)
enum class EOCBloodSeverity : uint8
{
    None,
    Light,
    Medium,
    Heavy,
    Extreme
};

UENUM(BlueprintType)
enum class EOCDismembermentSeverity : uint8
{
    None,
    SingleLimb,
    MultiPart,
    Catastrophic
};

UENUM(BlueprintType, meta=(Bitflags))
enum class EOCDismembermentPart : uint8
{
    None     = 0 UMETA(Hidden),
    LeftArm  = 1 << 0,
    RightArm = 1 << 1,
    LeftLeg  = 1 << 2,
    RightLeg = 1 << 3
};
ENUM_CLASS_FLAGS(EOCDismembermentPart);

UENUM(BlueprintType)
enum class EOCImpactSurface : uint8
{
    Default,
    Flesh,
    Glass,
    Wood,
    Metal,
    Masonry,
    Dirt
};

/**
 * Compact server-authored visual result for one damaging event.
 * Gameplay damage remains authoritative in HealthComponent. This struct only tells clients what to render.
 */
USTRUCT(BlueprintType)
struct FOCReplicatedTraumaEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) int32 Sequence = 0;
    UPROPERTY(BlueprintReadOnly) FVector_NetQuantize HitLocation = FVector::ZeroVector;
    UPROPERTY(BlueprintReadOnly) FVector_NetQuantizeNormal IncomingDirection = FVector::ForwardVector;
    UPROPERTY(BlueprintReadOnly) FName BoneName = NAME_None;
    UPROPERTY(BlueprintReadOnly) EOCBodyZone BodyZone = EOCBodyZone::Unknown;
    UPROPERTY(BlueprintReadOnly) EOCBloodSeverity BloodSeverity = EOCBloodSeverity::None;
    UPROPERTY(BlueprintReadOnly) EOCDismembermentSeverity DismembermentSeverity = EOCDismembermentSeverity::None;
    UPROPERTY(BlueprintReadOnly, meta=(Bitmask, BitmaskEnum="/Script/OsterConflict.EOCDismembermentPart")) uint8 DismembermentMask = 0;
    UPROPERTY(BlueprintReadOnly) EOCWeaponClass WeaponClass = EOCWeaponClass::AssaultRifle;
    UPROPERTY(BlueprintReadOnly) float Damage = 0.0f;
    UPROPERTY(BlueprintReadOnly) bool bExplosive = false;
    UPROPERTY(BlueprintReadOnly) bool bFatal = false;
};
