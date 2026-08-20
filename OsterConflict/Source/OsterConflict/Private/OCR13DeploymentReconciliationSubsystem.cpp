#include "OCR13DeploymentReconciliationSubsystem.h"

#include "OCPlayerController.h"
#include "OCR13DeploymentFlowSubsystem.h"

#include "Engine/World.h"

bool UOCR13DeploymentReconciliationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13DeploymentReconciliationSubsystem::Tick(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World) return;

    AOCPlayerController* PC = Cast<AOCPlayerController>(World->GetFirstPlayerController());
    if (!PC || !PC->IsLocalController() || !PC->IsDeploymentPanelVisible()) return;

    if (UOCR13DeploymentFlowSubsystem* Flow = World->GetSubsystem<UOCR13DeploymentFlowSubsystem>())
    {
        Flow->ReconcileAuthoritativeState(PC, DeltaTime);
    }
}

TStatId UOCR13DeploymentReconciliationSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCR13DeploymentReconciliationSubsystem, STATGROUP_Tickables);
}
