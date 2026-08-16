#pragma once

#include "CoreMinimal.h"
#include "OCHouseTypes.generated.h"

UENUM(BlueprintType)
enum class EOCHouseCondition : uint8
{
    Worn UMETA(DisplayName="Worn / inexpensive"),
    Ordinary UMETA(DisplayName="Ordinary"),
    Maintained UMETA(DisplayName="Maintained")
};
