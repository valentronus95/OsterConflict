#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "OCGameplayMode.h"
#include "OCLobbyTypes.h"
#include "OCPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class UOCGameUIRootWidget;

UCLASS()
class OSTERCONFLICT_API AOCPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AOCPlayerController();
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

    UFUNCTION(BlueprintPure, Category="HUD") bool IsScoreboardVisible() const { return bScoreboardVisible; }
    UFUNCTION(BlueprintPure, Category="Lobby") bool IsDeploymentPanelVisible() const { return bDeploymentPanelVisible; }
    UFUNCTION(BlueprintPure, Category="Sandbox") bool IsAdminPanelVisible() const { return bAdminPanelVisible; }
    UFUNCTION(BlueprintPure, Category="Sandbox") int32 GetSelectedAdminActionIndex() const { return SelectedAdminActionIndex; }
    UFUNCTION(BlueprintPure, Category="UI") bool IsFrontendMenuVisible() const { return bFrontendMenuVisible; }
    UFUNCTION(BlueprintPure, Category="UI") bool IsChatInputActive() const { return bChatInputActive; }
    UFUNCTION(BlueprintPure, Category="UI") bool IsSettingsVisible() const { return bSettingsVisible; }
    UFUNCTION(BlueprintPure, Category="UI") bool HasRichUI() const { return RichUIRoot != nullptr; }
    UFUNCTION(BlueprintPure, Category="UI") FName GetRequestedDeploymentSpawn() const { return RequestedDeploymentSpawn; }
    bool IsSandboxAdmin() const;
    bool IsSandboxGodMode() const { return bSandboxGodMode; }
    FString GetAdminActionLabel(int32 Index) const;

    const TArray<FOCChatMessage>& GetRecentChatMessages() const { return RecentChatMessages; }
    const FOCSquadOrder& GetCurrentSquadOrder() const { return CurrentSquadOrder; }

    UFUNCTION(Exec) void ConnectToServer(const FString& Address);
    UFUNCTION(Exec) void SetNickname(const FString& NewNickname);
    UFUNCTION(Exec) void DisconnectFromServer();
    UFUNCTION(Exec) void PerfReport();
    UFUNCTION(Client, Reliable) void ClientReceivePerfReport(const FString& Report);
    UFUNCTION(Client, Reliable) void ClientSetSandboxAdminAllowed(bool bAllowed);
    /** Deployment stays open until the authoritative server confirms that a pawn was actually created. */
    UFUNCTION(Client, Reliable) void ClientCompleteDeployment(bool bSuccess);

    /** S14 chat backend. The final S17 widget will call the same functions. */
    UFUNCTION(Exec) void SayGlobal(const FString& Message);
    UFUNCTION(Exec) void SayTeam(const FString& Message);
    UFUNCTION(Exec) void SaySquad(const FString& Message);

    /** Battlefield-style squad leader orders. */
    UFUNCTION(Exec) void SquadAttack(const FString& ObjectiveId);
    UFUNCTION(Exec) void SquadDefend(const FString& ObjectiveId);
    UFUNCTION(Exec) void SquadRegroup();
    UFUNCTION(Exec) void SquadMoveHere();

    UFUNCTION(Client, Reliable) void ClientReceiveChat(const FOCChatMessage& Message);
    UFUNCTION(Client, Reliable) void ClientReceiveSquadOrder(const FOCSquadOrder& Order);

    // S17A / R13 UMG-facing API. Selection is explicit so deployment can be a real staged flow.
    UFUNCTION(BlueprintCallable, Category="UI") void UIConnect(const FString& Address, const FString& Username);
    UFUNCTION(BlueprintCallable, Category="UI") void UIToggleFrontend();
    UFUNCTION(BlueprintCallable, Category="UI") void UIRequestTeam(EOCTeam Team);
    UFUNCTION(BlueprintCallable, Category="UI") void UIRequestSquad(int32 SquadId);
    UFUNCTION(BlueprintCallable, Category="UI") void UIRequestRole(EOCPlayerRole Role);
    UFUNCTION(BlueprintCallable, Category="UI") void UICycleRole();
    UFUNCTION(BlueprintCallable, Category="UI") void UICycleSquad();
    UFUNCTION(BlueprintCallable, Category="UI") void UISelectSpawn(FName SpawnId);
    UFUNCTION(BlueprintCallable, Category="UI") void UIReadyDeploy();
    /** R13 staged flow keeps the deployment panel visible while the server validates/grounds the spawn. */
    UFUNCTION(BlueprintCallable, Category="UI") void UICommitDeployment();
    UFUNCTION(BlueprintCallable, Category="UI") void UISendChat(EOCChatChannel Channel, const FString& Message);
    UFUNCTION(BlueprintCallable, Category="UI") void UIEndChatInput();
    UFUNCTION(BlueprintCallable, Category="UI") void UIAdminPrevious();
    UFUNCTION(BlueprintCallable, Category="UI") void UIAdminNext();
    UFUNCTION(BlueprintCallable, Category="UI") void UIAdminExecute();
    UFUNCTION(BlueprintCallable, Category="UI") void UIOpenSettings();
    UFUNCTION(BlueprintCallable, Category="UI") void UICloseSettings();
    UFUNCTION(BlueprintCallable, Category="UI") void UICloseDeployment();
    UFUNCTION(BlueprintCallable, Category="UI") void UICloseAdmin();
    UFUNCTION(BlueprintCallable, Category="UI") void UIApplyLocalPreferences();

protected:
    UFUNCTION(Server, Reliable) void ServerSetNickname(const FString& NewNickname);
    UFUNCTION(Server, Reliable) void ServerExecuteSandboxAdminAction(int32 ActionIndex);
    UFUNCTION(Server, Reliable) void ServerSendChat(EOCChatChannel Channel, const FString& Message);
    UFUNCTION(Server, Reliable) void ServerCycleRole();
    UFUNCTION(Server, Reliable) void ServerRequestRole(EOCPlayerRole RequestedRole);
    UFUNCTION(Server, Reliable) void ServerRequestSquad(int32 SquadId);
    UFUNCTION(Server, Reliable) void ServerSetLobbyReady(bool bReady);
    UFUNCTION(Server, Reliable) void ServerSubmitSquadOrder(EOCSquadOrderType Type, FName ObjectiveId, FVector Location);
    UFUNCTION(Server, Reliable) void ServerRequestTeam(EOCTeam RequestedTeam);
    UFUNCTION(Server, Reliable) void ServerSetDeploymentSpawn(FName SpawnId);
    UFUNCTION(Server, Reliable) void ServerRequestPerfReport();

private:
    UPROPERTY() TObjectPtr<UInputMappingContext> ControllerMappingContext;
    UPROPERTY() TObjectPtr<UInputAction> ScoreboardAction;
    UPROPERTY() TObjectPtr<UInputAction> AdminToggleAction;
    UPROPERTY() TObjectPtr<UInputAction> AdminUpAction;
    UPROPERTY() TObjectPtr<UInputAction> AdminDownAction;
    UPROPERTY() TObjectPtr<UInputAction> AdminExecuteAction;
    UPROPERTY() TObjectPtr<UInputAction> DeploymentToggleAction;
    UPROPERTY() TObjectPtr<UInputAction> RoleCycleAction;
    UPROPERTY() TObjectPtr<UInputAction> SquadCycleAction;
    UPROPERTY() TObjectPtr<UInputAction> ReadyAction;
    UPROPERTY() TObjectPtr<UInputAction> MenuToggleAction;
    UPROPERTY() TObjectPtr<UInputAction> ChatToggleAction;
    UPROPERTY() TObjectPtr<UOCGameUIRootWidget> RichUIRoot;

    bool bScoreboardVisible = false;
    bool bFrontendMenuVisible = false;
    bool bChatInputActive = false;
    bool bSettingsVisible = false;
    bool bDeploymentPanelVisible = true;
    bool bAdminPanelVisible = false;
    int32 SelectedAdminActionIndex = 0;
    bool bSandboxGodMode = false;
    bool bSandboxAdminAllowed = false;
    double LastChatServerTime = -100.0;
    TArray<FOCChatMessage> RecentChatMessages;
    FOCSquadOrder CurrentSquadOrder;
    FName RequestedDeploymentSpawn = TEXT("BASE");

    void ConfigureControllerInput();
    void RefreshControllerUserKeyMappings();
    void ShowScoreboard();
    void HideScoreboard();
    void ToggleDeploymentPanel();
    void ToggleFrontendMenu();
    void ToggleChatInput();
    void CreateRichUI();
    void ApplyUIInputMode();
    void CycleRole();
    void CycleSquad();
    void ToggleReady();
    void ApplyDeploymentInputLock();
    void ToggleAdminPanel();
    void AdminSelectUp();
    void AdminSelectDown();
    void AdminExecute();
    void ExecuteSandboxAdminActionServer(EOCSandboxAdminAction Action);
    void SendChat(EOCChatChannel Channel, const FString& Message);
    void SubmitSquadOrder(EOCSquadOrderType Type, FName ObjectiveId, const FVector& Location);
    static FString SanitizeNickname(const FString& RawName);
    static FString SanitizeChat(const FString& RawMessage);
};
