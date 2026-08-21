#pragma once

#include "CoreMinimal.h"
#include "OCTeamTypes.h"
#include "OCLobbyTypes.generated.h"

UENUM(BlueprintType)
enum class EOCChatChannel : uint8
{
    Global UMETA(DisplayName="Global"),
    Team UMETA(DisplayName="Team"),
    Squad UMETA(DisplayName="Squad")
};

UENUM(BlueprintType)
enum class EOCSquadOrderType : uint8
{
    None UMETA(DisplayName="None"),
    AttackObjective UMETA(DisplayName="Attack Objective"),
    DefendObjective UMETA(DisplayName="Defend Objective"),
    Move UMETA(DisplayName="Move"),
    Regroup UMETA(DisplayName="Regroup")
};

USTRUCT(BlueprintType)
struct FOCChatMessage
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) FString SenderName;
    UPROPERTY(BlueprintReadOnly) FString Message;
    UPROPERTY(BlueprintReadOnly) EOCChatChannel Channel = EOCChatChannel::Global;
    UPROPERTY(BlueprintReadOnly) EOCTeam Team = EOCTeam::None;
    UPROPERTY(BlueprintReadOnly) int32 SquadId = INDEX_NONE;
    UPROPERTY(BlueprintReadOnly) float ServerTime = 0.0f;
};

USTRUCT(BlueprintType)
struct FOCTacticalPing
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) FVector WorldLocation = FVector::ZeroVector;
    UPROPERTY(BlueprintReadOnly) FString IssuerName;
    UPROPERTY(BlueprintReadOnly) EOCTeam Team = EOCTeam::None;
    UPROPERTY(BlueprintReadOnly) int32 SquadId = INDEX_NONE;
    UPROPERTY(BlueprintReadOnly) float ServerTime = 0.0f;
};

USTRUCT(BlueprintType)
struct FOCSquadOrder
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) EOCSquadOrderType Type = EOCSquadOrderType::None;
    UPROPERTY(BlueprintReadOnly) FName ObjectiveId = NAME_None;
    UPROPERTY(BlueprintReadOnly) FVector WorldLocation = FVector::ZeroVector;
    UPROPERTY(BlueprintReadOnly) FString IssuerName;
    UPROPERTY(BlueprintReadOnly) float ServerTime = 0.0f;

    bool IsActive() const { return Type != EOCSquadOrderType::None; }
};

inline FString OCChatChannelToString(EOCChatChannel Channel)
{
    switch (Channel)
    {
    case EOCChatChannel::Team: return TEXT("TEAM");
    case EOCChatChannel::Squad: return TEXT("SQUAD");
    default: return TEXT("ALL");
    }
}

inline FString OCSquadName(int32 SquadId)
{
    static const TCHAR* Names[] = { TEXT("ALPHA"), TEXT("BRAVO"), TEXT("CHARLIE"), TEXT("DELTA"),
        TEXT("ECHO"), TEXT("FOXTROT"), TEXT("GOLF"), TEXT("HOTEL") };
    return (SquadId >= 0 && SquadId < UE_ARRAY_COUNT(Names)) ? FString(Names[SquadId]) : FString(TEXT("AUTO"));
}

inline FString OCSquadOrderToString(const FOCSquadOrder& Order)
{
    switch (Order.Type)
    {
    case EOCSquadOrderType::AttackObjective: return FString::Printf(TEXT("ATTACK %s"), *Order.ObjectiveId.ToString());
    case EOCSquadOrderType::DefendObjective: return FString::Printf(TEXT("DEFEND %s"), *Order.ObjectiveId.ToString());
    case EOCSquadOrderType::Move: return TEXT("MOVE");
    case EOCSquadOrderType::Regroup: return TEXT("REGROUP");
    default: return TEXT("NO ORDER");
    }
}
