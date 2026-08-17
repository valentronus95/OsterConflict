#include "OCUIRuntimePolishSubsystem.h"

#include "OCCharacter.h"
#include "OCGameUIRootWidget.h"
#include "OCPlayerController.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/EditableTextBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Widget.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "InputMappingContext.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UObject/UObjectIterator.h"

namespace
{
    void SetChildVisibility(UVerticalBox* Box, int32 Index, ESlateVisibility Visibility)
    {
        if (!Box || Index < 0 || Index >= Box->GetChildrenCount()) return;
        if (UWidget* Child = Box->GetChildAt(Index)) Child->SetVisibility(Visibility);
    }

    void SetText(UVerticalBox* Box, int32 Index, const FText& Text)
    {
        if (!Box || Index < 0 || Index >= Box->GetChildrenCount()) return;
        if (UTextBlock* TextBlock = Cast<UTextBlock>(Box->GetChildAt(Index))) TextBlock->SetText(Text);
    }

    void SetButtonLabel(UButton* Button, const FText& Text)
    {
        if (!Button) return;
        if (UTextBlock* Label = Cast<UTextBlock>(Button->GetContent())) Label->SetText(Text);
    }

    void SetButtonText(UVerticalBox* Box, int32 Index, const FText& Text)
    {
        if (!Box || Index < 0 || Index >= Box->GetChildrenCount()) return;
        SetButtonLabel(Cast<UButton>(Box->GetChildAt(Index)), Text);
    }

    void SetButtonReady(UVerticalBox* Box, int32 Index, bool bVisible)
    {
        if (!Box || Index < 0 || Index >= Box->GetChildrenCount()) return;
        if (UButton* Button = Cast<UButton>(Box->GetChildAt(Index)))
        {
            Button->SetIsEnabled(bVisible);
            Button->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
        }
    }

    void PolishButtons(UWidget* Widget)
    {
        if (!Widget) return;
        if (UButton* Button = Cast<UButton>(Widget))
        {
            Button->SetBackgroundColor(FLinearColor(0.018f, 0.032f, 0.047f, 0.96f));
            if (UTextBlock* Label = Cast<UTextBlock>(Button->GetContent()))
            {
                Label->SetColorAndOpacity(FSlateColor(FLinearColor(0.96f, 0.97f, 0.98f, 1.0f)));
                FSlateFontInfo Font = Label->GetFont();
                Font.Size = 17;
                Label->SetFont(Font);
            }
        }
        if (UPanelWidget* Panel = Cast<UPanelWidget>(Widget))
        {
            for (int32 Index = 0; Index < Panel->GetChildrenCount(); ++Index)
            {
                PolishButtons(Panel->GetChildAt(Index));
            }
        }
    }

    UButton* MakeRuntimeButton(UOCGameUIRootWidget* Root, UVerticalBox* Parent, const FText& Label)
    {
        if (!Root || !Parent) return nullptr;
        UButton* Button = NewObject<UButton>(Root);
        UTextBlock* Text = NewObject<UTextBlock>(Root);
        if (!Button || !Text) return nullptr;
        Text->SetText(Label);
        Text->SetJustification(ETextJustify::Center);
        Button->AddChild(Text);
        if (UVerticalBoxSlot* Slot = Parent->AddChildToVerticalBox(Button))
        {
            Slot->SetPadding(FMargin(0.0f, 5.0f, 0.0f, 5.0f));
        }
        return Button;
    }

    void PrimeFrontendInput(UOCGameUIRootWidget* Root, AOCPlayerController* PC, UButton* PreferredButton)
    {
        if (!Root || !PC) return;
        Root->SetIsEnabled(true);
        PC->bShowMouseCursor = true;
        FInputModeGameAndUI Mode;
        Mode.SetHideCursorDuringCapture(false);
        Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        Mode.SetWidgetToFocus(Root->TakeWidget());
        PC->SetInputMode(Mode);
        if (PreferredButton && PreferredButton->GetIsEnabled()) PreferredButton->SetKeyboardFocus();
    }
}

bool UOCUIRuntimePolishSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    if (!World) return false;
    return World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE;
}

void UOCUIRuntimePolishSubsystem::LeaveCurrentSession()
{
    if (AOCPlayerController* PC = ActiveFrontendController.Get())
    {
        PC->DisconnectFromServer();
        bFrontendSessionStarted = false;
        FrontendPage = 0;
        LastAppliedFrontendPage = INDEX_NONE;
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;
    if (AOCPlayerController* PC = Cast<AOCPlayerController>(World->GetFirstPlayerController()))
    {
        PC->DisconnectFromServer();
    }
}

void UOCUIRuntimePolishSubsystem::FrontendQuickStart()
{
    if (AOCPlayerController* PC = ActiveFrontendController.Get()) BeginLocalGameplay(PC);
}

void UOCUIRuntimePolishSubsystem::FrontendOpenLocal()
{
    FrontendPage = 1;
    LastAppliedFrontendPage = INDEX_NONE;
}

void UOCUIRuntimePolishSubsystem::FrontendOpenNetwork()
{
    FrontendPage = 2;
    LastAppliedFrontendPage = INDEX_NONE;
}

void UOCUIRuntimePolishSubsystem::FrontendStartLocal()
{
    if (AOCPlayerController* PC = ActiveFrontendController.Get()) BeginLocalGameplay(PC);
}

void UOCUIRuntimePolishSubsystem::FrontendConnectNetwork()
{
    AOCPlayerController* PC = ActiveFrontendController.Get();
    if (!PC) return;

    const FString Username = ActiveUsernameEntry.IsValid()
        ? ActiveUsernameEntry->GetText().ToString()
        : FString(TEXT("Player"));
    const FString Address = ActiveAddressEntry.IsValid()
        ? ActiveAddressEntry->GetText().ToString()
        : FString(TEXT("127.0.0.1:7777"));

    bFrontendSessionStarted = true;
    PC->UIConnect(Address, Username);
}

void UOCUIRuntimePolishSubsystem::FrontendBack()
{
    FrontendPage = 0;
    LastAppliedFrontendPage = INDEX_NONE;
}

void UOCUIRuntimePolishSubsystem::FrontendOpenSettings()
{
    if (AOCPlayerController* PC = ActiveFrontendController.Get())
    {
        PC->UIOpenSettings();
    }
}

void UOCUIRuntimePolishSubsystem::FrontendQuit()
{
    AOCPlayerController* PC = ActiveFrontendController.Get();
    if (!PC) return;
    UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
}

void UOCUIRuntimePolishSubsystem::BeginLocalGameplay(AOCPlayerController* PC)
{
    if (!PC) return;

    if (ActiveUsernameEntry.IsValid())
    {
        PC->SetNickname(ActiveUsernameEntry->GetText().ToString());
    }

    bFrontendSessionStarted = true;

    // The developer listen-server launcher already owns the local match. In that case Start simply reveals
    // deployment. A normal standalone executable opens the same map as a local listen server.
    if (PC->GetNetMode() != NM_Standalone)
    {
        if (PC->IsFrontendMenuVisible()) PC->UIToggleFrontend();
        return;
    }

    PC->ConsoleCommand(TEXT("open /Game/Maps/OsterConflict_Runtime?listen?Mode=Conquest?Bots=15?Population=16?BotFill=1?MaxPlayers=16"));
}

void UOCUIRuntimePolishSubsystem::EnsureMenuBackdrop(UOCGameUIRootWidget* Root, bool bVisible)
{
    if (!Root) return;
    UCanvasPanel* Canvas = Cast<UCanvasPanel>(Root->GetWidgetFromName(TEXT("OC_UI_Root")));
    if (!Canvas) return;

    if (!FullscreenMenuBackground.IsValid())
    {
        UImage* Background = NewObject<UImage>(Root);
        if (Background)
        {
            if (UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, TEXT("/Game/R13/UI/Oster_Menu_BG.Oster_Menu_BG")))
            {
                Background->SetBrushFromTexture(Texture, false);
            }
            Background->SetColorAndOpacity(FLinearColor::White);
            Background->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
            Background->SetIsEnabled(false);
            if (UCanvasPanelSlot* Slot = Canvas->AddChildToCanvas(Background))
            {
                Slot->SetPosition(FVector2D::ZeroVector);
                Slot->SetSize(FVector2D(1600.0f, 900.0f));
                Slot->SetZOrder(-100);
            }
            FullscreenMenuBackground = Background;
        }
    }

    if (!FullscreenMenuDimmer.IsValid())
    {
        UBorder* Dimmer = NewObject<UBorder>(Root);
        if (Dimmer)
        {
            Dimmer->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.38f));
            Dimmer->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
            Dimmer->SetIsEnabled(false);
            if (UCanvasPanelSlot* Slot = Canvas->AddChildToCanvas(Dimmer))
            {
                Slot->SetPosition(FVector2D::ZeroVector);
                Slot->SetSize(FVector2D(1600.0f, 900.0f));
                Slot->SetZOrder(-99);
            }
            FullscreenMenuDimmer = Dimmer;
        }
    }

    const ESlateVisibility Visibility = bVisible
        ? ESlateVisibility::SelfHitTestInvisible
        : ESlateVisibility::Collapsed;
    if (FullscreenMenuBackground.IsValid()) FullscreenMenuBackground->SetVisibility(Visibility);
    if (FullscreenMenuDimmer.IsValid()) FullscreenMenuDimmer->SetVisibility(Visibility);
}

void UOCUIRuntimePolishSubsystem::EnsureFrontendExtras(UOCGameUIRootWidget* Root, UVerticalBox* Frontend)
{
    if (!Root || !Frontend) return;

    // The original fourth button is reused as SETTINGS. Only one extra runtime button is necessary: QUIT.
    // This prevents the old Close Menu control from fighting the runtime polish pass every 0.2 seconds.
    if (FrontendSettingsExtraButton.IsValid())
    {
        FrontendSettingsExtraButton->SetVisibility(ESlateVisibility::Collapsed);
        FrontendSettingsExtraButton->SetIsEnabled(false);
    }

    if (!FrontendQuitExtraButton.IsValid())
    {
        UButton* QuitButton = MakeRuntimeButton(Root, Frontend, NSLOCTEXT("OCR13UI", "MainQuit", "ВИЙТИ З ГРИ"));
        if (QuitButton)
        {
            QuitButton->OnClicked.AddDynamic(this, &UOCUIRuntimePolishSubsystem::FrontendQuit);
            FrontendQuitExtraButton = QuitButton;
        }
    }
}

void UOCUIRuntimePolishSubsystem::ApplyFrontendPage(
    UOCGameUIRootWidget* Root, AOCPlayerController* PC, UVerticalBox* Frontend)
{
    if (!Root || !PC || !Frontend || Frontend->GetChildrenCount() < 9) return;

    EnsureFrontendExtras(Root, Frontend);

    UButton* Primary = Cast<UButton>(Frontend->GetChildAt(4));
    UButton* Secondary = Cast<UButton>(Frontend->GetChildAt(5));
    UButton* Tertiary = Cast<UButton>(Frontend->GetChildAt(6));
    UButton* Quaternary = Cast<UButton>(Frontend->GetChildAt(7));

    ActiveUsernameEntry = Cast<UEditableTextBox>(Frontend->GetChildAt(2));
    ActiveAddressEntry = Cast<UEditableTextBox>(Frontend->GetChildAt(3));

    if (FrontendPage == 0)
    {
        SetText(Frontend, 0, NSLOCTEXT("OCR13UI", "MainTitle", "OSTER CONFLICT"));
        SetText(Frontend, 1, NSLOCTEXT("OCR13UI", "MainSubtitle", "ОСТЕР  •  ГОЛОВНЕ МЕНЮ"));
        SetChildVisibility(Frontend, 1, ESlateVisibility::Visible);
        SetChildVisibility(Frontend, 2, ESlateVisibility::Collapsed);
        SetChildVisibility(Frontend, 3, ESlateVisibility::Collapsed);
        SetButtonReady(Frontend, 4, true);
        SetButtonReady(Frontend, 5, true);
        SetButtonReady(Frontend, 6, true);
        SetButtonReady(Frontend, 7, true);
        SetChildVisibility(Frontend, 8, ESlateVisibility::Collapsed);
        SetButtonText(Frontend, 4, NSLOCTEXT("OCR13UI", "MainStart", "СТАРТ"));
        SetButtonText(Frontend, 5, NSLOCTEXT("OCR13UI", "MainLocal", "ЛОКАЛЬНА ГРА"));
        SetButtonText(Frontend, 6, NSLOCTEXT("OCR13UI", "MainNetwork", "МЕРЕЖЕВА ГРА"));
        SetButtonText(Frontend, 7, NSLOCTEXT("OCR13UI", "MainSettings", "НАЛАШТУВАННЯ"));
        if (FrontendSettingsExtraButton.IsValid()) FrontendSettingsExtraButton->SetVisibility(ESlateVisibility::Collapsed);
        if (FrontendQuitExtraButton.IsValid())
        {
            FrontendQuitExtraButton->SetVisibility(ESlateVisibility::Visible);
            FrontendQuitExtraButton->SetIsEnabled(true);
        }
    }
    else if (FrontendPage == 1)
    {
        SetText(Frontend, 0, NSLOCTEXT("OCR13UI", "LocalTitle", "ЛОКАЛЬНА ГРА"));
        SetText(Frontend, 1, NSLOCTEXT("OCR13UI", "LocalSubtitle", "ІМ'Я ГРАВЦЯ"));
        SetChildVisibility(Frontend, 1, ESlateVisibility::Visible);
        SetChildVisibility(Frontend, 2, ESlateVisibility::Visible);
        SetChildVisibility(Frontend, 3, ESlateVisibility::Collapsed);
        SetButtonReady(Frontend, 4, true);
        SetButtonReady(Frontend, 5, false);
        SetButtonReady(Frontend, 6, false);
        SetButtonReady(Frontend, 7, true);
        SetChildVisibility(Frontend, 8, ESlateVisibility::Collapsed);
        SetButtonText(Frontend, 4, NSLOCTEXT("OCR13UI", "StartLocal", "ПОЧАТИ ЛОКАЛЬНУ ГРУ"));
        SetButtonText(Frontend, 7, NSLOCTEXT("OCR13UI", "Back", "НАЗАД"));
        if (FrontendSettingsExtraButton.IsValid()) FrontendSettingsExtraButton->SetVisibility(ESlateVisibility::Collapsed);
        if (FrontendQuitExtraButton.IsValid()) FrontendQuitExtraButton->SetVisibility(ESlateVisibility::Collapsed);
    }
    else
    {
        SetText(Frontend, 0, NSLOCTEXT("OCR13UI", "NetworkTitle", "МЕРЕЖЕВА ГРА"));
        SetText(Frontend, 1, NSLOCTEXT("OCR13UI", "NetworkSubtitle", "ПРЯМЕ ПІДКЛЮЧЕННЯ ДО СЕРВЕРА"));
        SetChildVisibility(Frontend, 1, ESlateVisibility::Visible);
        SetChildVisibility(Frontend, 2, ESlateVisibility::Visible);
        SetChildVisibility(Frontend, 3, ESlateVisibility::Visible);
        SetButtonReady(Frontend, 4, true);
        SetButtonReady(Frontend, 5, false);
        SetButtonReady(Frontend, 6, false);
        SetButtonReady(Frontend, 7, true);
        SetChildVisibility(Frontend, 8, ESlateVisibility::Visible);
        SetButtonText(Frontend, 4, NSLOCTEXT("OCR13UI", "ConnectNetwork", "ПІДКЛЮЧИТИСЯ"));
        SetButtonText(Frontend, 7, NSLOCTEXT("OCR13UI", "Back", "НАЗАД"));
        if (FrontendSettingsExtraButton.IsValid()) FrontendSettingsExtraButton->SetVisibility(ESlateVisibility::Collapsed);
        if (FrontendQuitExtraButton.IsValid()) FrontendQuitExtraButton->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (LastAppliedFrontendPage != FrontendPage)
    {
        if (Primary) Primary->OnClicked.Clear();
        if (Secondary) Secondary->OnClicked.Clear();
        if (Tertiary) Tertiary->OnClicked.Clear();
        if (Quaternary) Quaternary->OnClicked.Clear();

        if (FrontendPage == 0)
        {
            if (Primary) Primary->OnClicked.AddDynamic(this, &UOCUIRuntimePolishSubsystem::FrontendQuickStart);
            if (Secondary) Secondary->OnClicked.AddDynamic(this, &UOCUIRuntimePolishSubsystem::FrontendOpenLocal);
            if (Tertiary) Tertiary->OnClicked.AddDynamic(this, &UOCUIRuntimePolishSubsystem::FrontendOpenNetwork);
            if (Quaternary) Quaternary->OnClicked.AddDynamic(this, &UOCUIRuntimePolishSubsystem::FrontendOpenSettings);
        }
        else if (FrontendPage == 1)
        {
            if (Primary) Primary->OnClicked.AddDynamic(this, &UOCUIRuntimePolishSubsystem::FrontendStartLocal);
            if (Quaternary) Quaternary->OnClicked.AddDynamic(this, &UOCUIRuntimePolishSubsystem::FrontendBack);
        }
        else
        {
            if (Primary) Primary->OnClicked.AddDynamic(this, &UOCUIRuntimePolishSubsystem::FrontendConnectNetwork);
            if (Quaternary) Quaternary->OnClicked.AddDynamic(this, &UOCUIRuntimePolishSubsystem::FrontendBack);
        }

        PrimeFrontendInput(Root, PC, Primary);
        LastAppliedFrontendPage = FrontendPage;
    }

    PolishButtons(Frontend);
}

void UOCUIRuntimePolishSubsystem::Tick(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (TObjectIterator<UOCGameUIRootWidget> It; It; ++It)
    {
        UOCGameUIRootWidget* Root = *It;
        if (!IsValid(Root) || Root->GetWorld() != World) continue;

        AOCPlayerController* PC = Cast<AOCPlayerController>(Root->GetOwningPlayer());
        if (!PC || !PC->IsLocalController()) continue;

        if (ActiveFrontendRoot.Get() != Root)
        {
            ActiveFrontendRoot = Root;
            ActiveFrontendController = PC;
            ActiveUsernameEntry.Reset();
            ActiveAddressEntry.Reset();
            FrontendSettingsExtraButton.Reset();
            FrontendQuitExtraButton.Reset();
            FullscreenMenuBackground.Reset();
            FullscreenMenuDimmer.Reset();
            FrontendPage = 0;
            LastAppliedFrontendPage = INDEX_NONE;
        }
        else
        {
            ActiveFrontendController = PC;
        }

        // Armed vehicles previously added an RMB/mouse context which stole the driver's free-look and moved the
        // turret. Keep the normal vehicle context, but remove that obsolete solo-driver turret layer.
        APawn* CurrentPawn = PC->GetPawn();
        const bool bPawnChanged = CurrentPawn != LastLocalPawn.Get();
        if (bPawnChanged) LastLocalPawn = CurrentPawn;
        if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            for (TObjectIterator<UInputMappingContext> ContextIt; ContextIt; ++ContextIt)
            {
                UInputMappingContext* Context = *ContextIt;
                if (!IsValid(Context)) continue;
                const FName ContextName = Context->GetFName();
                if (ContextName == TEXT("IMC_DriverTurretRuntime"))
                {
                    InputSubsystem->RemoveMappingContext(Context);
                }
                else if (bPawnChanged && Cast<AOCCharacter>(CurrentPawn) && ContextName == TEXT("IMC_VehicleRuntime"))
                {
                    InputSubsystem->RemoveMappingContext(Context);
                }
            }
        }

        if (UWidget* ChatPanel = Root->GetWidgetFromName(TEXT("ChatPanel")))
        {
            const bool bChatOpen = PC->IsChatInputActive() &&
                !PC->IsFrontendMenuVisible() &&
                !PC->IsSettingsVisible() &&
                !PC->IsDeploymentPanelVisible();

            ChatPanel->SetRenderOpacity(bChatOpen ? 0.96f : 0.0f);
            ChatPanel->SetIsEnabled(bChatOpen);

            if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(ChatPanel->Slot))
            {
                Slot->SetPosition(FVector2D(30.0f, 650.0f));
                Slot->SetSize(FVector2D(520.0f, 210.0f));
            }
            if (UBorder* Border = Cast<UBorder>(ChatPanel))
            {
                Border->SetBrushColor(FLinearColor(0.015f, 0.020f, 0.026f, 0.80f));
                Border->SetPadding(FMargin(12.0f));
            }
        }

        const bool bSettings = PC->IsSettingsVisible();
        const bool bFrontend = PC->IsFrontendMenuVisible() && !bSettings;
        const bool bForcedFrontend = FParse::Param(FCommandLine::Get(), TEXT("Frontend"));
        const bool bMainFrontend = bFrontend && !bFrontendSessionStarted && PC->GetNetMode() != NM_Client &&
            (PC->GetNetMode() == NM_Standalone || bForcedFrontend);
        const bool bDeployment = !bFrontend && !bSettings && PC->IsDeploymentPanelVisible();
        const bool bMenuBackdrop = bMainFrontend || bDeployment || (bSettings && !bFrontendSessionStarted);
        EnsureMenuBackdrop(Root, bMenuBackdrop);

        if (bDeployment)
        {
            if (UBorder* DeploymentPanel = Cast<UBorder>(Root->GetWidgetFromName(TEXT("DeploymentPanel"))))
            {
                // The photo now fills the entire viewport behind the UI. The deployment panel itself is only a
                // readable translucent surface, so gameplay geometry can no longer leak around the photo frame.
                DeploymentPanel->SetBrushColor(FLinearColor(0.010f, 0.018f, 0.028f, 0.84f));
                DeploymentPanel->SetPadding(FMargin(30.0f));
                PolishButtons(DeploymentPanel->GetContent());

                if (UHorizontalBox* Columns = Cast<UHorizontalBox>(DeploymentPanel->GetContent()))
                {
                    if (Columns->GetChildrenCount() >= 3)
                    {
                        if (UVerticalBox* Left = Cast<UVerticalBox>(Columns->GetChildAt(0)))
                        {
                            if (UHorizontalBoxSlot* LeftSlot = Cast<UHorizontalBoxSlot>(Left->Slot))
                                LeftSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
                            SetText(Left, 0, NSLOCTEXT("OCR13UI", "DeployTitle", "OSTER CONFLICT  •  РОЗГОРТАННЯ"));
                            if (Left->GetChildrenCount() > 1)
                            {
                                if (UTextBlock* Identity = Cast<UTextBlock>(Left->GetChildAt(1)))
                                {
                                    Identity->SetAutoWrapText(true);
                                    FSlateFontInfo Font = Identity->GetFont();
                                    Font.Size = 13;
                                    Identity->SetFont(Font);
                                }
                            }
                            SetButtonText(Left, 2, NSLOCTEXT("OCR13UI", "Team1", "КОМАНДА 1"));
                            SetButtonText(Left, 3, NSLOCTEXT("OCR13UI", "Team2", "КОМАНДА 2"));
                            SetButtonText(Left, 4, NSLOCTEXT("OCR13UI", "Role", "ЗМІНИТИ КЛАС"));
                            SetButtonText(Left, 5, NSLOCTEXT("OCR13UI", "Squad", "ЗМІНИТИ ГРУПУ"));
                        }
                        if (UVerticalBox* Spawn = Cast<UVerticalBox>(Columns->GetChildAt(1)))
                        {
                            if (UHorizontalBoxSlot* SpawnSlot = Cast<UHorizontalBoxSlot>(Spawn->Slot))
                                SpawnSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
                            SetText(Spawn, 0, NSLOCTEXT("OCR13UI", "SpawnTitle", "МІСЦЕ ПОЯВИ"));
                            SetButtonText(Spawn, 2, NSLOCTEXT("OCR13UI", "Base", "БАЗА"));
                            SetButtonText(Spawn, 3, NSLOCTEXT("OCR13UI", "PointA", "ТОЧКА A"));
                            SetButtonText(Spawn, 4, NSLOCTEXT("OCR13UI", "PointB", "ТОЧКА B"));
                            SetButtonText(Spawn, 5, NSLOCTEXT("OCR13UI", "PointC", "ТОЧКА C"));
                            SetButtonText(Spawn, 6, NSLOCTEXT("OCR13UI", "Deploy", "ПОЧАТИ ГРУ"));
                        }
                        if (UVerticalBox* DebugColumn = Cast<UVerticalBox>(Columns->GetChildAt(2)))
                        {
                            DebugColumn->SetVisibility(ESlateVisibility::Collapsed);
                        }
                    }
                }

                if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(DeploymentPanel->Slot))
                {
                    Slot->SetPosition(FVector2D(110.0f, 140.0f));
                    Slot->SetSize(FVector2D(1380.0f, 600.0f));
                }
            }
        }

        if (bFrontend)
        {
            if (UBorder* FrontendPanel = Cast<UBorder>(Root->GetWidgetFromName(TEXT("FrontendPanel"))))
            {
                UVerticalBox* Frontend = Cast<UVerticalBox>(FrontendPanel->GetContent());
                if (!Frontend) continue;

                if (bMainFrontend)
                {
                    ApplyFrontendPage(Root, PC, Frontend);
                    FrontendPanel->SetIsEnabled(true);
                    FrontendPanel->SetBrushColor(FLinearColor(0.008f, 0.014f, 0.022f, 0.90f));
                    FrontendPanel->SetPadding(FMargin(28.0f));
                    if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(FrontendPanel->Slot))
                    {
                        Slot->SetPosition(FVector2D(120.0f, 150.0f));
                        Slot->SetSize(FVector2D(510.0f, 570.0f));
                    }
                }
                else
                {
                    // Escape during a live match is a pause menu, not the direct-connect frontend.
                    if (Frontend->GetChildrenCount() > 0)
                        if (UTextBlock* Title = Cast<UTextBlock>(Frontend->GetChildAt(0)))
                            Title->SetText(NSLOCTEXT("OCR13UI", "PauseMenuTitle", "МЕНЮ ГРИ  •  ESC = ПРОДОВЖИТИ"));

                    SetChildVisibility(Frontend, 1, ESlateVisibility::Collapsed);
                    SetChildVisibility(Frontend, 2, ESlateVisibility::Collapsed);
                    SetChildVisibility(Frontend, 3, ESlateVisibility::Collapsed);
                    SetChildVisibility(Frontend, 4, ESlateVisibility::Collapsed);
                    SetChildVisibility(Frontend, 5, ESlateVisibility::Collapsed);
                    SetChildVisibility(Frontend, 6, ESlateVisibility::Visible);
                    SetChildVisibility(Frontend, 7, ESlateVisibility::Visible);
                    SetChildVisibility(Frontend, 8, ESlateVisibility::Collapsed);
                    if (FrontendSettingsExtraButton.IsValid()) FrontendSettingsExtraButton->SetVisibility(ESlateVisibility::Collapsed);
                    if (FrontendQuitExtraButton.IsValid()) FrontendQuitExtraButton->SetVisibility(ESlateVisibility::Collapsed);
                    SetButtonText(Frontend, 6, NSLOCTEXT("OCR13UI", "PauseSettings", "НАЛАШТУВАННЯ"));
                    SetButtonText(Frontend, 7, NSLOCTEXT("OCR13UI", "PauseLeave", "ВИЙТИ В ГОЛОВНЕ МЕНЮ"));

                    if (UButton* LeaveButton = Cast<UButton>(Frontend->GetChildAt(7)))
                    {
                        if (BoundPauseLeaveButton.Get() != LeaveButton)
                        {
                            LeaveButton->OnClicked.Clear();
                            LeaveButton->OnClicked.AddDynamic(this, &UOCUIRuntimePolishSubsystem::LeaveCurrentSession);
                            BoundPauseLeaveButton = LeaveButton;
                        }
                    }

                    FrontendPanel->SetBrushColor(FLinearColor(0.010f, 0.016f, 0.024f, 0.985f));
                    FrontendPanel->SetPadding(FMargin(26.0f));
                    PolishButtons(Frontend);
                    if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(FrontendPanel->Slot))
                    {
                        Slot->SetPosition(FVector2D(555.0f, 285.0f));
                        Slot->SetSize(FVector2D(490.0f, 250.0f));
                    }
                }
            }
        }
    }
}

TStatId UOCUIRuntimePolishSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCUIRuntimePolishSubsystem, STATGROUP_Tickables);
}
