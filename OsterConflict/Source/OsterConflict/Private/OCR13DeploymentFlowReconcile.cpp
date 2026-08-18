#include "OCR13DeploymentFlowSubsystem.h"

#include "OCPlayerController.h"
#include "OCPlayerState.h"

#include "Components/TextBlock.h"

void UOCR13DeploymentFlowSubsystem::ReconcileAuthoritativeState(AOCPlayerController* PC, float DeltaSeconds)
{
    if (!PC || !PC->IsLocalController() || !PC->IsDeploymentPanelVisible())
    {
        AuthorityReconcileAge = 0.0f;
        return;
    }

    const bool bSelectionChanged = ReconcileTeamSnapshot != SelectedTeam ||
        ReconcileSquadSnapshot != SelectedSquad ||
        ReconcileRoleSnapshot != SelectedRole ||
        bReconcileRoleSelectedSnapshot != bRoleSelected;

    if (bSelectionChanged)
    {
        ReconcileTeamSnapshot = SelectedTeam;
        ReconcileSquadSnapshot = SelectedSquad;
        ReconcileRoleSnapshot = SelectedRole;
        bReconcileRoleSelectedSnapshot = bRoleSelected;
        AuthorityReconcileAge = 0.0f;
        return;
    }

    AuthorityReconcileAge += FMath::Max(0.0f, DeltaSeconds);
    constexpr float ReplicationGraceSeconds = 0.65f;
    if (AuthorityReconcileAge < ReplicationGraceSeconds) return;

    const AOCPlayerState* State = PC->GetPlayerState<AOCPlayerState>();
    if (!State) return;

    if (CurrentStep >= 1 && SelectedTeam != EOCTeam::None && State->GetTeamId() != SelectedTeam)
    {
        SelectedTeam = EOCTeam::None;
        SelectedSquad = INDEX_NONE;
        bRoleSelected = false;
        SelectedSpawn = NAME_None;
        SetStep(0);
        if (StatusText.IsValid())
        {
            StatusText->SetText(FText::FromString(
                TEXT("Сервер не підтвердив цю команду. Оберіть доступну сторону.")));
        }
        AuthorityReconcileAge = 0.0f;
        return;
    }

    if (CurrentStep >= 2 && SelectedSquad >= 0 && State->GetSquadId() != SelectedSquad)
    {
        SelectedSquad = INDEX_NONE;
        bRoleSelected = false;
        SelectedSpawn = NAME_None;
        SetStep(1);
        if (StatusText.IsValid())
        {
            StatusText->SetText(FText::FromString(
                TEXT("Група вже недоступна або заповнилась. Оберіть іншу групу.")));
        }
        AuthorityReconcileAge = 0.0f;
        return;
    }

    if (CurrentStep >= 3 && bRoleSelected && State->GetPlayerRole() != SelectedRole)
    {
        bRoleSelected = false;
        SelectedSpawn = NAME_None;
        SetStep(2);
        if (StatusText.IsValid())
        {
            StatusText->SetText(FText::FromString(
                TEXT("Роль уже зайнята в цій групі. Оберіть іншу роль.")));
        }
        AuthorityReconcileAge = 0.0f;
        return;
    }
}
