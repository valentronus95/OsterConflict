#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCUIRuntimePolishSubsystem.generated.h"

class AOCPlayerController;
class APawn;
class UBorder;
class UButton;
class UEditableTextBox;
class UImage;
class UOCGameUIRootWidget;
class UVerticalBox;

/**
 * R13 runtime presentation layer for the source-built interface.
 *
 * Keeps the existing gameplay/network backend intact while presenting a normal player-facing flow:
 * - full-screen Oster museum backdrop for frontend/deployment instead of gameplay leaking behind the menu;
 * - top-level Start / Local / Network / Settings / Quit navigation;
 * - separate local/network pages instead of one overloaded direct-connect form;
 * - compact deployment and pause-menu presentation;
 * - driver free-look remains independent from the dedicated gunner controls;
 * - stale vehicle input mapping is removed when control returns to infantry.
 */
UCLASS()
class OSTERCONFLICT_API UOCUIRuntimePolishSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;

private:
    UFUNCTION() void LeaveCurrentSession();
    UFUNCTION() void FrontendQuickStart();
    UFUNCTION() void FrontendOpenLocal();
    UFUNCTION() void FrontendOpenNetwork();
    UFUNCTION() void FrontendStartLocal();
    UFUNCTION() void FrontendConnectNetwork();
    UFUNCTION() void FrontendBack();
    UFUNCTION() void FrontendOpenSettings();
    UFUNCTION() void FrontendQuit();

    void EnsureMenuBackdrop(UOCGameUIRootWidget* Root, bool bVisible);
    void EnsureFrontendExtras(UOCGameUIRootWidget* Root, UVerticalBox* Frontend);
    void ApplyFrontendPage(UOCGameUIRootWidget* Root, AOCPlayerController* PC, UVerticalBox* Frontend);
    void BeginLocalGameplay(AOCPlayerController* PC);

    TWeakObjectPtr<APawn> LastLocalPawn;
    TWeakObjectPtr<UButton> BoundPauseLeaveButton;

    TWeakObjectPtr<UOCGameUIRootWidget> ActiveFrontendRoot;
    TWeakObjectPtr<AOCPlayerController> ActiveFrontendController;
    TWeakObjectPtr<UEditableTextBox> ActiveUsernameEntry;
    TWeakObjectPtr<UEditableTextBox> ActiveAddressEntry;
    TWeakObjectPtr<UButton> FrontendSettingsExtraButton;
    TWeakObjectPtr<UButton> FrontendQuitExtraButton;
    TWeakObjectPtr<UImage> FullscreenMenuBackground;
    TWeakObjectPtr<UBorder> FullscreenMenuDimmer;

    int32 FrontendPage = 0; // 0 main, 1 local, 2 network
    int32 LastAppliedFrontendPage = INDEX_NONE;
    bool bFrontendSessionStarted = false;
};
