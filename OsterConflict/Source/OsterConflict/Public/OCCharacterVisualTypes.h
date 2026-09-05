#pragma once

#include "CoreMinimal.h"
#include "OCCharacterVisualTypes.generated.h"

/** S16C visual faction preset. Two factions are selected for a match; these are art identities, not gameplay balance classes. */
UENUM(BlueprintType)
enum class EOCFactionArchetype : uint8
{
    UASpecialUnit UMETA(DisplayName="UA Special Unit"),
    MaskedFighters UMETA(DisplayName="Masked Fighters"),
    USRangers UMETA(DisplayName="US Rangers Style"),
    Insurgents UMETA(DisplayName="Insurgents")
};

UENUM(BlueprintType)
enum class EOCCharacterGearClass : uint8
{
    Light,
    Standard,
    Heavy
};

/** One-shot animation/presentation notifications. Continuous locomotion remains data-driven in the AnimInstance. */
UENUM(BlueprintType)
enum class EOCCharacterActionEvent : uint8
{
    Fire,
    ReloadStart,
    GrenadeThrow,
    ReviveStart,
    ReviveComplete,
    Interact,
    Equip,
    Downed,
    Revived,
    Death
};

USTRUCT(BlueprintType)
struct FOCCharacterAppearance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) EOCFactionArchetype Faction = EOCFactionArchetype::UASpecialUnit;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EOCCharacterGearClass GearClass = EOCCharacterGearClass::Standard;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 VariantSeed = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 HeadVariant = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 HelmetVariant = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 VestVariant = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 BackpackVariant = 0;
};

FORCEINLINE FString OCFactionToString(EOCFactionArchetype Faction)
{
    switch (Faction)
    {
    case EOCFactionArchetype::UASpecialUnit: return TEXT("UA SPECIAL UNIT");
    case EOCFactionArchetype::MaskedFighters: return TEXT("MASKED FIGHTERS");
    case EOCFactionArchetype::USRangers: return TEXT("US RANGERS STYLE");
    case EOCFactionArchetype::Insurgents: return TEXT("INSURGENTS");
    default: return TEXT("UNKNOWN");
    }
}
