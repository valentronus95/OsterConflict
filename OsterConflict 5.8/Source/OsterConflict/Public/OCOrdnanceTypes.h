#pragma once

#include "CoreMinimal.h"
#include "OCOrdnanceTypes.generated.h"

UENUM(BlueprintType)
enum class EOCGrenadeType : uint8
{
    Fragmentation UMETA(DisplayName="Fragmentation"),
    Smoke UMETA(DisplayName="Smoke"),
    Flash UMETA(DisplayName="Flash / Stun")
};

/** Fifteen deliberately abstract GAMEPLAY presets. These are not real-world construction recipes. */
UENUM(BlueprintType)
enum class EOCTrapPreset : uint8
{
    ContactInfantry,
    ProximityInfantry,
    TripwireZone,
    DirectionalZone,
    RemoteGameTrap,
    AntiVehicle,
    AntiArmorGame,
    ElectronicDisruptor,
    SmokeTrap,
    FlashTrap,
    HazardZone,
    SignalMine,
    RepairJammer,
    DecoyTrap,
    ObjectiveTrap
};

inline FString OCGrenadeTypeToString(EOCGrenadeType Type)
{
    switch (Type)
    {
        case EOCGrenadeType::Fragmentation: return TEXT("FRAG");
        case EOCGrenadeType::Smoke: return TEXT("SMOKE");
        case EOCGrenadeType::Flash: return TEXT("FLASH");
        default: return TEXT("GRENADE");
    }
}

inline FString OCTrapPresetToString(EOCTrapPreset Preset)
{
    switch (Preset)
    {
        case EOCTrapPreset::ContactInfantry: return TEXT("CONTACT INFANTRY");
        case EOCTrapPreset::ProximityInfantry: return TEXT("PROXIMITY INFANTRY");
        case EOCTrapPreset::TripwireZone: return TEXT("TRIPWIRE ZONE");
        case EOCTrapPreset::DirectionalZone: return TEXT("DIRECTIONAL ZONE");
        case EOCTrapPreset::RemoteGameTrap: return TEXT("REMOTE GAME TRAP");
        case EOCTrapPreset::AntiVehicle: return TEXT("ANTI-VEHICLE");
        case EOCTrapPreset::AntiArmorGame: return TEXT("ANTI-ARMOR GAME");
        case EOCTrapPreset::ElectronicDisruptor: return TEXT("ELECTRONIC DISRUPTOR");
        case EOCTrapPreset::SmokeTrap: return TEXT("SMOKE TRAP");
        case EOCTrapPreset::FlashTrap: return TEXT("FLASH TRAP");
        case EOCTrapPreset::HazardZone: return TEXT("HAZARD ZONE");
        case EOCTrapPreset::SignalMine: return TEXT("SIGNAL MINE");
        case EOCTrapPreset::RepairJammer: return TEXT("REPAIR JAMMER");
        case EOCTrapPreset::DecoyTrap: return TEXT("DECOY TRAP");
        case EOCTrapPreset::ObjectiveTrap: return TEXT("OBJECTIVE TRAP");
        default: return TEXT("TRAP");
    }
}
