#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "UObject/UObjectIterator.h"
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
 * This subsystem owns the current frontend and pause presentation. The legacy direct-connect panel is detached
 * from the canvas so it cannot reappear underneath the R13 menu when the root widget refreshes after deployment
 * or when Escape is pressed during a live match.
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

    void EnsureFrontend(UOCGameUIRootWidget* Root, AOCPlayerController* PC);
    void BuildFrontend(UOCGameUIRootWidget* Root, AOCPlayerController* PC);
    void ApplyPage();
    void ApplyPausePage();
    void SetPresentationVisibility(bool bShowMenu, bool bShowBackdrop, bool bDimGameplay = false);
    void SuppressLegacyFrontendLayers(UOCGameUIRootWidget* Root);
    void StartLocalGameplay();
    void ForceMenuInput();
    void ReleaseMenuInput();

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
    TWeakObjectPtr<UTextBlock> StatusText;
    TWeakObjectPtr<UButton> PrimaryButton;
    TWeakObjectPtr<UButton> SecondaryButton;
    TWeakObjectPtr<UButton> NetworkButton;
    TWeakObjectPtr<UButton> SettingsButton;
    TWeakObjectPtr<UButton> QuitButton;

    int32 Page = 0; // 0 main, 1 local, 2 network
    bool bGameplayStarted = false;
    bool bPauseMenuActive = false;
    bool bLocalTravelPending = false; // keep the frontend frame intact until the gameplay world actually replaces it
};