#pragma once

#include "CoreMinimal.h"
#include "OCAudioTypes.generated.h"

UENUM(BlueprintType)
enum class EOCAcousticEnvironment : uint8
{
    Outdoor UMETA(DisplayName="Outdoor"),
    SemiIndoor UMETA(DisplayName="Semi Indoor"),
    Indoor UMETA(DisplayName="Indoor")
};

UENUM(BlueprintType)
enum class EOCWeaponAudioEvent : uint8
{
    ReloadStart UMETA(DisplayName="Reload Start"),
    ReloadEnd UMETA(DisplayName="Reload End"),
    ReloadCancel UMETA(DisplayName="Reload Cancel"),
    DryFire UMETA(DisplayName="Dry Fire"),
    FireModeSwitch UMETA(DisplayName="Fire Mode Switch"),
    Equip UMETA(DisplayName="Equip"),
    Drop UMETA(DisplayName="Drop")
};

UENUM(BlueprintType)
enum class EOCAudioBus : uint8
{
    Master,
    Weapons,
    Vehicles,
    Characters,
    WorldSFX,
    Ambience,
    Music,
    UI,
    VoiceChat,
    Dialogue
};

UENUM(BlueprintType)
enum class EOCDynamicRange : uint8
{
    Night UMETA(DisplayName="Night / Compressed"),
    Standard UMETA(DisplayName="Standard"),
    High UMETA(DisplayName="High Dynamic Range")
};

UENUM(BlueprintType)
enum class EOCAudioOutputMode : uint8
{
    StereoSpeakers UMETA(DisplayName="Stereo Speakers"),
    Headphones UMETA(DisplayName="Headphones"),
    SpatialHeadphones UMETA(DisplayName="3D / Spatial Headphones")
};

UENUM(BlueprintType)
enum class EOCWorldAudioEvent : uint8
{
    InteractionGeneric,
    DoorOpen,
    DoorClose,
    GateOpen,
    GateClose,
    LightOn,
    LightOff,
    WindowBreak,
    DestructionWood,
    DestructionMetal,
    DestructionMasonry,
    ExplosionSmall,
    ExplosionLarge,
    Debris
};

UENUM(BlueprintType)
enum class EOCMenuAudioEvent : uint8
{
    Hover,
    Click,
    Back,
    Confirm,
    Error,
    OpenPanel,
    ClosePanel
};
