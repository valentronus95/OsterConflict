#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13FrontendMenuSubsystem.generated.h"

class AOCPlayerController;
class UBorder;
class UButton;
class UEditableTextBox;
class UImage;
class UOCGameUIRootWidget;
class UTextBlock;
class UVerticalBox;

/**
 * Dedicated R13 player-facing frontend.
 *
 * The frontend is a shell only. Main-menu START never spawns gameplay directly: it opens
 * the explicit host-server setup. A listen/client world owns deployment and cannot resurrect
 * the startup frontend after travel.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13FrontendMenuSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;

private:
    UFUNCTION() void OnPrimaryClicked();
    UFUNCTION() void OnSecondaryClicked();
    UFUNCTION() void OnNetworkClicked();
    UFUNCTION() void OnSettingsClicked();
    UFUNCTION() void OnQuitClicked();

    UOCGameUIRootWidget* ResolveRoot(UWorld* World, AOCPlayerController* PC) const;
    void EnsureFrontend(UOCGameUIRootWidget* Root, AOCPlayerController* PC);
    void BuildFrontend(UOCGameUIRootWidget* Root, AOCPlayerController* PC);
    void ApplyPage();
    void ApplyPausePage();
    void SetPresentationVisibility(bool bShowMenu, bool bShowBackdrop, bool bDimGameplay = false);
    void SuppressLegacyFrontendLayers(UOCGameUIRootWidget* Root);
    void StartHostedGameplay();
    void StartNetworkGameplay();
    void ForceMenuInput();
    void ReleaseMenuInput();
    bool HasPendingFrontendAction() const;
    void ArmDeferredActionFence();

    TWeakObjectPtr<UOCGameUIRootWidget> ActiveRoot;
    TWeakObjectPtr<AOCPlayerController> ActiveController;

    TWeakObjectPtr<UBorder> WorldBlocker;
    TWeakObjectPtr<UImage> MenuBackground;
    TWeakObjectPtr<UBorder> MenuShade;
    TArray<TWeakObjectPtr<UBorder>> MenuGradientLayers;
    TWeakObjectPtr<UBorder> MenuPanel;

    TWeakObjectPtr<UVerticalBox> MenuBox;
    TWeakObjectPtr<UTextBlock> BrandOsterText;
    TWeakObjectPtr<UTextBlock> BrandConflictText;
    TWeakObjectPtr<UTextBlock> TitleText;
    TWeakObjectPtr<UTextBlock> SubtitleText;
    TWeakObjectPtr<UVerticalBox> FieldsBox;
    TWeakObjectPtr<UEditableTextBox> UsernameEntry;
    TWeakObjectPtr<UEditableTextBox> AddressEntry;
    TWeakObjectPtr<UEditableTextBox> MaxPlayersEntry;
    TWeakObjectPtr<UEditableTextBox> BotsEntry;
    TWeakObjectPtr<UEditableTextBox> BotDifficultyEntry;
    TWeakObjectPtr<UTextBlock> StatusText;
    TWeakObjectPtr<UButton> PrimaryButton;
    TWeakObjectPtr<UButton> SecondaryButton;
    TWeakObjectPtr<UButton> NetworkButton;
    TWeakObjectPtr<UButton> SettingsButton;
    TWeakObjectPtr<UButton> QuitButton;

    int32 Page = 0; // 0 main, 1 create server, 2 join server
    int32 PendingPage = INDEX_NONE; // Pass 24: structural Slate changes are applied outside input callbacks
    int32 LastAppliedPage = INDEX_NONE;
    bool bPendingHostedStart = false;
    bool bPendingNetworkConnect = false;
    bool bPendingSettingsOpen = false;
    bool bPendingQuit = false;
    bool bPendingPauseResume = false;
    uint64 PendingActionEarliestFrame = 0; // Pass 26: never execute a frontend action in the click's engine frame
    bool bMenuInputArmed = false; // Pass 25: SetInputMode is armed once, never reset every Tick
    bool bGameplayStarted = false;
    bool bPauseMenuActive = false;
    bool bPausePageApplied = false;
    bool bLocalTravelPending = false; // keep approved frontend intact until the server world replaces it
    bool bPresentationStateValid = false;
    bool bLastShowMenu = false;
    bool bLastShowBackdrop = false;
    bool bLastDimGameplay = false;
    bool bSettingsPanelStyled = false;
    bool bRootCacheBudgetLogged = false;
};