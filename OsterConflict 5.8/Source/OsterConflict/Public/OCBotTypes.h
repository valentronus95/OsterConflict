#pragma once

#include "CoreMinimal.h"
#include "OCBotTypes.generated.h"

UENUM(BlueprintType)
enum class EOCBotDifficulty : uint8
{
    Easy UMETA(DisplayName="Easy"),
    Normal UMETA(DisplayName="Normal"),
    Hard UMETA(DisplayName="Hard"),
    Veteran UMETA(DisplayName="Veteran")
};

inline FString OCBotDifficultyToString(EOCBotDifficulty Difficulty)
{
    switch (Difficulty)
    {
    case EOCBotDifficulty::Easy: return TEXT("EASY");
    case EOCBotDifficulty::Hard: return TEXT("HARD");
    case EOCBotDifficulty::Veteran: return TEXT("VETERAN");
    default: return TEXT("NORMAL");
    }
}
