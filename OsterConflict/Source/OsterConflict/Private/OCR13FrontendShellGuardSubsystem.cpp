#include "OCR13FrontendShellGuardSubsystem.h"

#include "OCGameMode.h"
#include "OCPlayerController.h"

#include "Engine/World.h"
#include "GameFramework/Pawn.h"

bool UOCR13FrontendShellGuardSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13FrontendShellGuardSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    EnforceFrontendShell(InWorld);
}

void UOCR13FrontendShellGuardSubsystem::Tick(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World || !World->GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    EnforceFrontendShell(*World);
}

TStatId UOCR13FrontendShellGuardSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCR13FrontendShellGuardSubsystem, STATGROUP_Tickables);
}

void UOCR13FrontendShellGuardSubsystem::EnforceFrontendShell(UWorld& World)
{
    AOCGameMode* GameMode = World.GetAuthGameMode<AOCGameMode>();
    if (!GameMode || !GameMode->IsFrontendOnlySession()) return;

    AOCPlayerController* PC = Cast<AOCPlayerController>(World.GetFirstPlayerController());
    if (!PC || !PC->IsLocalController()) return;

    APawn* Pawn = PC->GetPawn();
    if (!Pawn) return;

    // The standalone frontend is explicitly UI-only. A possessed pawn here would make the menu subsystem classify
    // the screen as live gameplay and expose the world behind the pause presentation. Remove only this impossible
    // frontend-shell state; listen-server/local gameplay worlds are not FrontendOnlySession and never enter here.
    PC->UnPossess();
    if (Pawn->HasAuthority())
    {
        Pawn->Destroy();
    }

    if (!bLoggedPawnLeak)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("R13 frontend shell guard: removed leaked gameplay pawn from frontend-only session; static menu backdrop preserved."));
        bLoggedPawnLeak = true;
    }
}
