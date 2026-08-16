#pragma once

#include "CoreMinimal.h"
#include "OCTeamTypes.generated.h"

UENUM(BlueprintType)
enum class EOCTeam : uint8
{
    None UMETA(DisplayName="None"),
    TeamOne UMETA(DisplayName="Team One"),
    TeamTwo UMETA(DisplayName="Team Two")
};

UENUM(BlueprintType)
enum class EOCPlayerRole : uint8
{
    Rifleman UMETA(DisplayName="Rifleman"),
    Medic UMETA(DisplayName="Medic"),
    Engineer UMETA(DisplayName="Engineer"),
    Support UMETA(DisplayName="Support")
};

UENUM(BlueprintType)
enum class EOCMatchPhase : uint8
{
    Waiting UMETA(DisplayName="Waiting"),
    InProgress UMETA(DisplayName="In Progress"),
    Ended UMETA(DisplayName="Ended")
};

inline FString OCTeamToString(EOCTeam Team)
{
    switch (Team)
    {
    case EOCTeam::TeamOne: return TEXT("TEAM 1");
    case EOCTeam::TeamTwo: return TEXT("TEAM 2");
    default: return TEXT("NEUTRAL");
    }
}

inline FString OCRoleToString(EOCPlayerRole Role)
{
    switch (Role)
    {
    case EOCPlayerRole::Medic: return TEXT("MEDIC");
    case EOCPlayerRole::Engineer: return TEXT("ENGINEER");
    case EOCPlayerRole::Support: return TEXT("SUPPORT");
    default: return TEXT("RIFLEMAN");
    }
}
