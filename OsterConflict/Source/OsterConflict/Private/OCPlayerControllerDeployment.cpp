#include "OCPlayerController.h"

void AOCPlayerController::UICommitDeployment()
{
    // Keep legacy callers source-compatible, but route the final action through the same proven ready/spawn path.
    UIReadyDeploy();
    UE_LOG(LogTemp, Display, TEXT("PASS45_UI_COMMIT_DEPLOYMENT_READY direct_ready_path=1"));
}
