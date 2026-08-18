#pragma once

#include "CoreMinimal.h"
#include "OCGameplayMode.h"
#include "OCTeamTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13DeploymentFlowSubsystem.generated.h"

class AOCPlayerController;
class UBorder;
class UButton;
class UOCGameUIRootWidget;
class UTextBlock;
class UWidgetSwitcher;

/**
 * R13.3 player-facing deployment flow.
 *
 * Replaces the legacy all-in-one three-column lobby with four explicit stages:
 * team -> squad -> role -> spawn. The legacy widget remains in the source tree for
 * compatibility but is rendered inert while this subsystem owns deployment.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13DeploymentFlowSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;

private:
    void EnsureBuilt(UOCGameUIRootWidget* Root, AOCPlayerController* PC);
    void SetStep(int32 NewStep);
    void RefreshState(AOCPlayerController* PC);
    void ResetFlow();

    int32 CountSquadMembers(EOCTeam Team, int32 SquadId, const AOCPlayerController* PC) const;
    bool IsRoleAvailable(EOCTeam Team, int32 SquadId, EOCPlayerRole Role, const AOCPlayerController* PC) const;
    bool IsSpawnAvailable(EOCTeam Team, FName SpawnId) const;
    FString BuildSelectedSquadRoster(EOCTeam Team, int32 SquadId, const AOCPlayerController* PC) const;

    UButton* MakeActionButton(UObject* Owner, const FString& Label, UTextBlock*& OutLabel);
    UBorder* MakeSection(UObject* Owner, const FString& Title, UTextBlock*& OutBody);

    TWeakObjectPtr<UOCGameUIRootWidget> ActiveRoot;
    TWeakObjectPtr<UBorder> FlowPanel;
    TWeakObjectPtr<UWidgetSwitcher> PageSwitcher;
    TWeakObjectPtr<UTextBlock> StepText;
    TWeakObjectPtr<UTextBlock> MatchText;
    TWeakObjectPtr<UTextBlock> SelectionText;
    TWeakObjectPtr<UTextBlock> SquadRosterText;
    TWeakObjectPtr<UTextBlock> SpawnSelectionText;
    TWeakObjectPtr<UTextBlock> StatusText;
    TWeakObjectPtr<UButton> BackButton;
    TWeakObjectPtr<UButton> DeployButton;

    TArray<TWeakObjectPtr<UButton>> SquadButtons;
    TArray<TWeakObjectPtr<UTextBlock>> SquadButtonTexts;
    TArray<TWeakObjectPtr<UButton>> RoleButtons;
    TArray<TWeakObjectPtr<UTextBlock>> RoleButtonTexts;
    TArray<TWeakObjectPtr<UButton>> SpawnButtons;
    TArray<TWeakObjectPtr<UTextBlock>> SpawnButtonTexts;

    int32 CurrentStep = 0;
    EOCTeam SelectedTeam = EOCTeam::None;
    int32 SelectedSquad = INDEX_NONE;
    EOCPlayerRole SelectedRole = EOCPlayerRole::Rifleman;
    bool bRoleSelected = false;
    FName SelectedSpawn = NAME_None;
    bool bWasVisible = false;
    float RefreshAccumulator = 0.0f;

    UFUNCTION() void OnTeamOne();
    UFUNCTION() void OnTeamTwo();
    UFUNCTION() void OnSquadAlpha();
    UFUNCTION() void OnSquadBravo();
    UFUNCTION() void OnSquadCharlie();
    UFUNCTION() void OnSquadDelta();
    UFUNCTION() void OnRoleRifleman();
    UFUNCTION() void OnRoleMedic();
    UFUNCTION() void OnRoleEngineer();
    UFUNCTION() void OnRoleSupport();
    UFUNCTION() void OnSpawnBase();
    UFUNCTION() void OnSpawnA();
    UFUNCTION() void OnSpawnB();
    UFUNCTION() void OnSpawnC();
    UFUNCTION() void OnBack();
    UFUNCTION() void OnDeploy();
};
