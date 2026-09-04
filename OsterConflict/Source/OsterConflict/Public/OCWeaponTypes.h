#pragma once

#include "CoreMinimal.h"
#include "OCWeaponTypes.generated.h"

UENUM(BlueprintType)
enum class EOCWeaponClass : uint8
{
    AssaultRifle UMETA(DisplayName="Assault Rifle"),
    SMG UMETA(DisplayName="SMG"),
    Pistol UMETA(DisplayName="Pistol"),
    SniperRifle UMETA(DisplayName="Sniper Rifle"),
    Shotgun UMETA(DisplayName="Shotgun"),
    LMG UMETA(DisplayName="LMG"),
    Launcher UMETA(DisplayName="Launcher")
};

UENUM(BlueprintType)
enum class EOCInventorySlot : uint8
{
    None UMETA(DisplayName="None"),
    Primary UMETA(DisplayName="Primary"),
    Secondary UMETA(DisplayName="Secondary")
};

UENUM(BlueprintType)
enum class EOCAmmoType : uint8
{
    Any UMETA(DisplayName="Any"),
    Rifle UMETA(DisplayName="Rifle"),
    Pistol UMETA(DisplayName="Pistol"),
    Shell UMETA(DisplayName="Shell"),
    Precision UMETA(DisplayName="Precision"),
    Rocket UMETA(DisplayName="Rocket")
};

/** Live selector positions. Safe is represented by input/state gating rather than a fake fire pulse. */
UENUM(BlueprintType)
enum class EOCFireMode : uint8
{
    SemiAutomatic UMETA(DisplayName="Semi"),
    Burst3 UMETA(DisplayName="3-Round Burst"),
    Automatic UMETA(DisplayName="Automatic")
};

/**
 * Mechanical action family. This is presentation/gameplay metadata, not a weapon-class alias: two rifles can share
 * a class while requiring completely different post-shot actions and animation/audio contracts.
 */
UENUM(BlueprintType)
enum class EOCWeaponActionType : uint8
{
    GasOperated UMETA(DisplayName="Gas Operated"),
    DelayedBlowback UMETA(DisplayName="Delayed Blowback"),
    Blowback UMETA(DisplayName="Blowback"),
    ShortRecoil UMETA(DisplayName="Short Recoil"),
    Revolver UMETA(DisplayName="Revolver"),
    BoltAction UMETA(DisplayName="Bolt Action"),
    PumpAction UMETA(DisplayName="Pump Action"),
    LeverAction UMETA(DisplayName="Lever Action"),
    BeltFed UMETA(DisplayName="Belt Fed"),
    LauncherSingleShot UMETA(DisplayName="Single-Shot Launcher")
};

UENUM(BlueprintType)
enum class EOCAttachmentSlot : uint8
{
    Optic UMETA(DisplayName="Optic"),
    Muzzle UMETA(DisplayName="Muzzle"),
    Underbarrel UMETA(DisplayName="Underbarrel"),
    Magazine UMETA(DisplayName="Magazine"),
    Stock UMETA(DisplayName="Stock")
};

USTRUCT(BlueprintType)
struct FOCWeaponAttachmentState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EOCAttachmentSlot Slot = EOCAttachmentSlot::Optic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName AttachmentId = NAME_None;
};

USTRUCT(BlueprintType)
struct FOCWeaponTuning
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName WeaponId = FName(TEXT("OC_AR1"));

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DisplayName = TEXT("OC-AR1");

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EOCWeaponClass WeaponClass = EOCWeaponClass::AssaultRifle;

    /** Mechanical action drives bolt/pump/lever/belt presentation and post-shot state. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EOCWeaponActionType ActionType = EOCWeaponActionType::GasOperated;

    /** Explicit post-shot cycle for manual actions. Zero means no separate manual cycle gate. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="5.0"))
    float ManualActionCycleSeconds = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EOCInventorySlot PreferredSlot = EOCInventorySlot::Primary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EOCAmmoType AmmoType = EOCAmmoType::Rifle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Damage = 34.0f;

    /** Number of independent hitscan pellets fired per trigger shot. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PelletsPerShot = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RangeCm = 12000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RoundsPerMinute = 650.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HipSpreadDegrees = 1.30f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ADSSpreadDegrees = 0.22f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovingSpreadMultiplier = 1.70f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RecoilPitchMin = 0.55f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RecoilPitchMax = 0.95f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RecoilYawMax = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MagazineSize = 30;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 InitialReserveAmmo = 120;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxReserveAmmo = 240;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ReloadDuration = 2.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSupportsSemiAutomatic = true;

    /** Explicit because a family/model name alone does not prove a 3-round selector is fitted. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSupportsBurst3 = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSupportsAutomatic = true;

    /** Audio/ballistic presentation metadata. Does not alter authoritative hit resolution. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSupersonicAmmo = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="2.0"))
    float AudioLoudnessScale = 1.0f;
};
