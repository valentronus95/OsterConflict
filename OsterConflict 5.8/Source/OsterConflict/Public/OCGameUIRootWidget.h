#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OCLobbyTypes.h"
#include "OCTeamTypes.h"
#include "OCGameUIRootWidget.generated.h"

class UBorder;
class UButton;
class UCanvasPanel;
class UCheckBox;
class UComboBoxString;
class UEditableTextBox;
class UHorizontalBox;
class USlider;
class UTextBlock;
class UVerticalBox;
class UWidgetSwitcher;
class UWidget;
class AOCPlayerController;

/**
 * Source-only UMG root. S17A provides frontend/deployment/chat/admin; S17B adds a persistent multi-page
 * settings screen without requiring Widget Blueprint assets. Final art can replace this class while keeping APIs.
 */
UCLASS()
class OSTERCONFLICT_API UOCGameUIRootWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
    UPROPERTY() TObjectPtr<UCanvasPanel> RootCanvas;
    UPROPERTY() TObjectPtr<UBorder> FrontendPanel;
    UPROPERTY() TObjectPtr<UBorder> DeploymentPanel;
    UPROPERTY() TObjectPtr<UBorder> ScoreboardPanel;
    UPROPERTY() TObjectPtr<UBorder> ChatPanel;
    UPROPERTY() TObjectPtr<UBorder> AdminPanel;
    UPROPERTY() TObjectPtr<UBorder> SettingsPanel;

    UPROPERTY() TObjectPtr<UEditableTextBox> UsernameEntry;
    UPROPERTY() TObjectPtr<UEditableTextBox> AddressEntry;
    UPROPERTY() TObjectPtr<UTextBlock> FrontendStatusText;
    UPROPERTY() TObjectPtr<UButton> FrontendConnectButton;
    UPROPERTY() TObjectPtr<UButton> FrontendLocalButton;
    UPROPERTY() TObjectPtr<UButton> FrontendSettingsButton;
    UPROPERTY() TObjectPtr<UButton> FrontendCloseButton;

    UPROPERTY() TObjectPtr<UTextBlock> DeploymentIdentityText;
    UPROPERTY() TObjectPtr<UTextBlock> DeploymentPopulationText;
    UPROPERTY() TObjectPtr<UTextBlock> DeploymentRosterText;
    UPROPERTY() TObjectPtr<UTextBlock> DeploymentSpawnText;
    UPROPERTY() TObjectPtr<UTextBlock> ScoreboardText;
    UPROPERTY() TObjectPtr<UTextBlock> ChatLogText;
    UPROPERTY() TObjectPtr<UTextBlock> ChatChannelText;
    UPROPERTY() TObjectPtr<UEditableTextBox> ChatEntry;
    UPROPERTY() TObjectPtr<UButton> ChatChannelButton;
    UPROPERTY() TObjectPtr<UButton> ChatSendButton;
    UPROPERTY() TObjectPtr<UTextBlock> AdminActionText;
    UPROPERTY() TObjectPtr<UButton> DeploymentTeamOneButton;
    UPROPERTY() TObjectPtr<UButton> DeploymentTeamTwoButton;
    UPROPERTY() TObjectPtr<UButton> DeploymentRoleButton;
    UPROPERTY() TObjectPtr<UButton> DeploymentSquadButton;
    UPROPERTY() TObjectPtr<UButton> DeploymentBaseButton;
    UPROPERTY() TObjectPtr<UButton> DeploymentAButton;
    UPROPERTY() TObjectPtr<UButton> DeploymentBButton;
    UPROPERTY() TObjectPtr<UButton> DeploymentCButton;
    UPROPERTY() TObjectPtr<UButton> DeploymentReadyButton;
    UPROPERTY() TObjectPtr<UButton> AdminPrevButton;
    UPROPERTY() TObjectPtr<UButton> AdminNextButton;
    UPROPERTY() TObjectPtr<UButton> AdminExecuteButton;

    // S17B settings root.
    UPROPERTY() TObjectPtr<UWidgetSwitcher> SettingsSwitcher;
    UPROPERTY() TObjectPtr<UTextBlock> SettingsStatusText;
    UPROPERTY() TObjectPtr<UButton> SettingsGraphicsButton;
    UPROPERTY() TObjectPtr<UButton> SettingsAudioButton;
    UPROPERTY() TObjectPtr<UButton> SettingsControlsButton;
    UPROPERTY() TObjectPtr<UButton> SettingsInterfaceButton;
    UPROPERTY() TObjectPtr<UButton> SettingsAccessibilityButton;
    UPROPERTY() TObjectPtr<UButton> SettingsApplyButton;
    UPROPERTY() TObjectPtr<UButton> SettingsSaveButton;
    UPROPERTY() TObjectPtr<UButton> SettingsCancelButton;
    UPROPERTY() TObjectPtr<UButton> SettingsDefaultsButton;

    // Graphics.
    UPROPERTY() TObjectPtr<UComboBoxString> ResolutionCombo;
    UPROPERTY() TObjectPtr<UComboBoxString> WindowModeCombo;
    UPROPERTY() TObjectPtr<UComboBoxString> QualityPresetCombo;
    UPROPERTY() TObjectPtr<UComboBoxString> ViewDistanceCombo;
    UPROPERTY() TObjectPtr<UComboBoxString> ShadowCombo;
    UPROPERTY() TObjectPtr<UComboBoxString> TextureCombo;
    UPROPERTY() TObjectPtr<UComboBoxString> EffectsCombo;
    UPROPERTY() TObjectPtr<UComboBoxString> FoliageCombo;
    UPROPERTY() TObjectPtr<UComboBoxString> PostProcessCombo;
    UPROPERTY() TObjectPtr<UComboBoxString> AntiAliasingCombo;
    UPROPERTY() TObjectPtr<UComboBoxString> ShadingCombo;
    UPROPERTY() TObjectPtr<UComboBoxString> GlobalIlluminationCombo;
    UPROPERTY() TObjectPtr<UComboBoxString> ReflectionCombo;
    UPROPERTY() TObjectPtr<UComboBoxString> LandscapeCombo;
    UPROPERTY() TObjectPtr<UComboBoxString> FrameLimitCombo;
    UPROPERTY() TObjectPtr<USlider> ResolutionScaleSlider;
    UPROPERTY() TObjectPtr<UTextBlock> ResolutionScaleValue;
    UPROPERTY() TObjectPtr<UCheckBox> VSyncCheck;
    UPROPERTY() TObjectPtr<UCheckBox> DynamicResolutionCheck;

    // Audio.
    UPROPERTY() TObjectPtr<USlider> MasterVolumeSlider;
    UPROPERTY() TObjectPtr<USlider> WeaponsVolumeSlider;
    UPROPERTY() TObjectPtr<USlider> VehiclesVolumeSlider;
    UPROPERTY() TObjectPtr<USlider> CharactersVolumeSlider;
    UPROPERTY() TObjectPtr<USlider> WorldSFXVolumeSlider;
    UPROPERTY() TObjectPtr<USlider> AmbienceVolumeSlider;
    UPROPERTY() TObjectPtr<USlider> MusicVolumeSlider;
    UPROPERTY() TObjectPtr<USlider> UIVolumeSlider;
    UPROPERTY() TObjectPtr<USlider> VoiceChatVolumeSlider;
    UPROPERTY() TObjectPtr<USlider> DialogueVolumeSlider;
    UPROPERTY() TObjectPtr<UCheckBox> MasterAudioCheck;
    UPROPERTY() TObjectPtr<UCheckBox> WeaponsAudioCheck;
    UPROPERTY() TObjectPtr<UCheckBox> VehiclesAudioCheck;
    UPROPERTY() TObjectPtr<UCheckBox> CharactersAudioCheck;
    UPROPERTY() TObjectPtr<UCheckBox> WorldSFXAudioCheck;
    UPROPERTY() TObjectPtr<UCheckBox> AmbienceAudioCheck;
    UPROPERTY() TObjectPtr<UCheckBox> MusicAudioCheck;
    UPROPERTY() TObjectPtr<UCheckBox> MenuMusicCheck;
    UPROPERTY() TObjectPtr<UCheckBox> UIAudioCheck;
    UPROPERTY() TObjectPtr<UCheckBox> VoiceChatAudioCheck;
    UPROPERTY() TObjectPtr<UCheckBox> DialogueAudioCheck;
    UPROPERTY() TObjectPtr<UComboBoxString> DynamicRangeCombo;
    UPROPERTY() TObjectPtr<UComboBoxString> AudioOutputCombo;

    // Controls / interface / accessibility.
    UPROPERTY() TObjectPtr<USlider> MouseSensitivitySlider;
    UPROPERTY() TObjectPtr<UTextBlock> MouseSensitivityValue;
    UPROPERTY() TObjectPtr<USlider> AimSensitivitySlider;
    UPROPERTY() TObjectPtr<UTextBlock> AimSensitivityValue;
    UPROPERTY() TObjectPtr<UCheckBox> InvertYCheck;
    UPROPERTY() TObjectPtr<USlider> FOVSlider;
    UPROPERTY() TObjectPtr<UTextBlock> FOVValue;
    UPROPERTY() TObjectPtr<USlider> HUDScaleSlider;
    UPROPERTY() TObjectPtr<UTextBlock> HUDScaleValue;
    UPROPERTY() TObjectPtr<UCheckBox> ShowFPSCheck;
    UPROPERTY() TObjectPtr<UCheckBox> ShowPingCheck;
    UPROPERTY() TObjectPtr<UCheckBox> ShowCrosshairCheck;
    UPROPERTY() TObjectPtr<UCheckBox> ShowHitMarkerCheck;
    UPROPERTY() TObjectPtr<UComboBoxString> GoreCombo;
    UPROPERTY() TObjectPtr<UCheckBox> SubtitlesCheck;
    UPROPERTY() TObjectPtr<UCheckBox> ReduceFlashesCheck;
    UPROPERTY() TObjectPtr<USlider> CameraShakeSlider;
    UPROPERTY() TObjectPtr<UTextBlock> CameraShakeValue;
    UPROPERTY() TObjectPtr<UComboBoxString> ColorVisionCombo;

    TMap<FName, UTextBlock*> KeyBindingTexts;
    TMap<int32, UTextBlock*> AudioValueTexts;
    FName PendingRebindAction = NAME_None;

    EOCChatChannel SelectedChatChannel = EOCChatChannel::Team;
    FName SelectedSpawnId = TEXT("BASE");
    float RefreshAccumulator = 0.0f;
    int32 LastFocusContext = INDEX_NONE;

    TArray<TObjectPtr<UWidget>> FrontendFocusOrder;
    TArray<TObjectPtr<UWidget>> DeploymentFocusOrder;
    TArray<TObjectPtr<UWidget>> SettingsTabFocusOrder;
    TArray<TObjectPtr<UWidget>> SettingsFooterFocusOrder;
    TArray<TObjectPtr<UWidget>> AdminFocusOrder;

    void BuildWidgetTree();
    void BuildSettingsTree(UVerticalBox* SettingsRoot);
    void RefreshAll();
    void RefreshFrontend(AOCPlayerController* PC);
    void RefreshDeployment(AOCPlayerController* PC);
    void RefreshScoreboard(AOCPlayerController* PC);
    void RefreshChat(AOCPlayerController* PC);
    void RefreshAdmin(AOCPlayerController* PC);
    void RefreshSettingsLabels();
    void SyncSettingsWidgetsFromBackend();
    void ApplySettingsWidgets(bool bCloseAfterApply);
    void CancelPendingSettings();
    void ResetSettingsWidgetsToDefaults();
    void RefreshKeyBindingLabels();
    void BeginRebind(FName ActionId);
    void SetSettingsPage(int32 Index);
    void UpdateFocusForVisibleContext(AOCPlayerController* PC, bool bFrontend, bool bSettings);
    void FocusWidget(UWidget* Widget);
    void WireLinearNavigation(const TArray<TObjectPtr<UWidget>>& Widgets, bool bHorizontal);

    UTextBlock* MakeText(const FText& Text, int32 Size = 16, bool bBoldHint = false);
    UTextBlock* MakeText(const FString& Text, int32 Size = 16, bool bBoldHint = false);
    UButton* MakeButton(UVerticalBox* Parent, const FText& Label);
    UButton* MakeButton(UVerticalBox* Parent, const FString& Label);
    UBorder* MakePanel(const FString& DebugName, const FLinearColor& Color);
    void PlacePanel(UBorder* Panel, const FVector2D& Position, const FVector2D& Size, int32 ZOrder);
    USlider* MakeSliderRow(UVerticalBox* Parent, const FText& Label, UTextBlock*& OutValueText);
    UCheckBox* MakeCheckRow(UVerticalBox* Parent, const FText& Label);
    UComboBoxString* MakeComboRow(UVerticalBox* Parent, const FText& Label, const TArray<FString>& Options);
    UButton* MakeRebindRow(UVerticalBox* Parent, FName ActionId, const FText& Label);

    UFUNCTION() void OnConnectClicked();
    UFUNCTION() void OnLocalhostClicked();
    UFUNCTION() void OnCloseFrontendClicked();
    UFUNCTION() void OnOpenSettingsClicked();
    UFUNCTION() void OnSettingsGraphicsClicked();
    UFUNCTION() void OnSettingsAudioClicked();
    UFUNCTION() void OnSettingsControlsClicked();
    UFUNCTION() void OnSettingsInterfaceClicked();
    UFUNCTION() void OnSettingsAccessibilityClicked();
    UFUNCTION() void OnQualityPresetChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnSettingsApplyClicked();
    UFUNCTION() void OnSettingsSaveClicked();
    UFUNCTION() void OnSettingsCancelClicked();
    UFUNCTION() void OnSettingsDefaultsClicked();
    UFUNCTION() void OnTeamOneClicked();
    UFUNCTION() void OnTeamTwoClicked();
    UFUNCTION() void OnRoleClicked();
    UFUNCTION() void OnSquadClicked();
    UFUNCTION() void OnSpawnBaseClicked();
    UFUNCTION() void OnSpawnAClicked();
    UFUNCTION() void OnSpawnBClicked();
    UFUNCTION() void OnSpawnCClicked();
    UFUNCTION() void OnDeployClicked();
    UFUNCTION() void OnChatChannelClicked();
    UFUNCTION() void OnChatSendClicked();
    UFUNCTION() void OnChatCommitted(const FText& Text, ETextCommit::Type CommitMethod);
    UFUNCTION() void OnAdminPrevClicked();
    UFUNCTION() void OnAdminNextClicked();
    UFUNCTION() void OnAdminExecuteClicked();

    UFUNCTION() void OnBindMoveForward(); UFUNCTION() void OnBindMoveBackward();
    UFUNCTION() void OnBindMoveLeft(); UFUNCTION() void OnBindMoveRight();
    UFUNCTION() void OnBindJump(); UFUNCTION() void OnBindSprint(); UFUNCTION() void OnBindCrouch();
    UFUNCTION() void OnBindFire(); UFUNCTION() void OnBindAim(); UFUNCTION() void OnBindReload();
    UFUNCTION() void OnBindInteract(); UFUNCTION() void OnBindGrenade();
    UFUNCTION() void OnBindScoreboard(); UFUNCTION() void OnBindChat();
};
