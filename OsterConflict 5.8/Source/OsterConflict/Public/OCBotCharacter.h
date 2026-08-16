#pragma once

#include "CoreMinimal.h"
#include "OCCharacter.h"
#include "OCBotCharacter.generated.h"

/** Server-driven infantry pawn used by S13 AI controllers. */
UCLASS()
class OSTERCONFLICT_API AOCBotCharacter : public AOCCharacter
{
    GENERATED_BODY()

public:
    AOCBotCharacter();
};
