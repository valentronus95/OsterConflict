#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13FrontendMenuSubsystem.generated.h"

class AOCPlayerController;
class UBorder;
class UButton;
class UEditableTextBox;
class UOCGameUIRootWidget;
class UTextBlock;
class UVerticalBox;

/**
 * Dedicated R13 player-facing frontend.
 *
 * The legacy frontend was originally a direct-connect/debug panel and later got rewritten by a second runtime
 * polish layer every frame. That made button bindings and page state fight each other. This subsystem replaces the
 * legacy panel content once with a stable eight-child menu while leaving the existing settings/deployment backend
 * intact. The eight-child shape is intentional: the older polish layer only rewrites nine-child legacy frontends.
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
    void BuildFrontend(UOCGameUIRootWidget* Root, UBorder* Panel, AOCPlayerController* PC);
    void ApplyPage();
    void ApplyPausePage();
    void StartLocalGameplay();
    void ForceMenuInput();

    TWeakObjectPtr<UOCGameUIRootWidget> ActiveRoot;
    TWeakObjectPtr<AOCPlayerController> ActiveController;
    TWeakObjectPtr<UBorder> ActivePanel;
    TWeakObjectPtr<UVerticalBox> MenuBox;
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
};
