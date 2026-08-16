#include "OCBotCharacter.h"

#include "OCAIController.h"

AOCBotCharacter::AOCBotCharacter()
{
    AIControllerClass = AOCAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::Disabled;
}
