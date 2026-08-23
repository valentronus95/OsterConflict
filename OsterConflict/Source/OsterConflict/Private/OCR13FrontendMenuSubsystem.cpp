#include "OCR13FrontendMenuSubsystem.h"

#include "Blueprint/WidgetTree.h"

#include "OCGameUIRootWidget.h"
#include "OCPlayerController.h"
#include "OCPlayerUserSettings.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/EditableTextBox.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Widget.h"
#include "CoreGlobals.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "GameFramework/PlayerInput.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Styling/SlateTypes.h"

namespace
{
    UTextBlock* R13FrontendMakeMenuText(UWidgetTree* Tree, const FText& Text, int32 FontSize, bool bBright = true)
    {
        if (!Tree) return nullptr;
        UTextBlock* Block = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
        if (!Block) return nullptr;
        Block->SetText(Text);
        Block->SetColorAndOpacity(FSlateColor(bBright
            ? FLinearColor(0.94f, 0.93f, 0.89f, 1.0f)
            : FLinearColor(0.69f, 0.69f, 0.66f, 1.0f)));
        FSlateFontInfo Font = Block->GetFont();
        Font.Size = FontSize;
        Block->SetFont(Font);
        return Block;
    }

    void R13FrontendApplyTypeface(UTextBlock* Text, const FName Typeface, int32 LetterSpacing)
    {
        if (!Text) return;
        FSlateFontInfo Font = Text->GetFont();
        if (!Typeface.IsNone()) Font.TypefaceFontName = Typeface;
        Font.LetterSpacing = LetterSpacing;
        Text->SetFont(Font);
    }

    UButton* R13FrontendMakeMenuButton(UWidgetTree* Tree, UVerticalBox* Parent, const FText& Label)
    {
        if (!Tree || !Parent) return nullptr;

        USizeBox* Size = Tree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
        UButton* Button = Tree->ConstructWidget<UButton>(UButton::StaticClass());
        UTextBlock* Text = R13FrontendMakeMenuText(Tree, Label, 16, true);
        if (!Size || !Button || !Text) return nullptr;

        Size->SetHeightOverride(50.0f);
        Size->SetWidthOverride(420.0f);
        Text->SetJustification(ETextJustify::Center);
        R13FrontendApplyTypeface(Text, FName(TEXT("Regular")), 45);
        Button->IsFocusable = true;
        Button->AddChild(Text);
        Size->SetContent(Button);

        FButtonStyle Style = Button->GetStyle();
        Style.Normal.TintColor = FSlateColor(FLinearColor(0.04f, 0.04f, 0.04f, 0.08f));
        Style.Hovered.TintColor = FSlateColor(FLinearColor(0.16f, 0.16f, 0.16f, 0.20f));
        Style.Pressed.TintColor = FSlateColor(FLinearColor(0.20f, 0.20f, 0.20f, 0.28f));
        Style.Disabled.TintColor = FSlateColor(FLinearColor(0.03f, 0.03f, 0.03f, 0.05f));
        Style.NormalPadding = FMargin(1.0f);
        Style.PressedPadding = FMargin(1.0f, 2.0f, 1.0f, 0.0f);
        Button->SetStyle(Style);

        if (UVerticalBoxSlot* Slot = Parent->AddChildToVerticalBox(Size))
        {
            Slot->SetPadding(FMargin(0.0f, 5.0f, 0.0f, 5.0f));
            Slot->SetHorizontalAlignment(HAlign_Left);
        }
        return Button;
    }

    UEditableTextBox* R13FrontendMakeField(UWidgetTree* Tree, UVerticalBox* Parent, const FText& Hint, const FString& Value)
    {
        if (!Tree || !Parent) return nullptr;
        UEditableTextBox* Field = Tree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass());
        if (!Field) return nullptr;
        Field->SetHintText(Hint);
        Field->SetText(FText::FromString(Value));

        // Pass 24: use an explicit game-owned dark style instead of the editor/default white field look.
        // Keeping all brushes resource-stable while tinting them avoids constructing transient Slate resources
        // during the main-menu -> server-setup transition.
        FEditableTextBoxStyle FieldStyle = Field->GetWidgetStyle();
        FieldStyle.BackgroundColor = FSlateColor(FLinearColor(0.045f, 0.052f, 0.061f, 1.0f));
        FieldStyle.ForegroundColor = FSlateColor(FLinearColor(0.93f, 0.93f, 0.91f, 1.0f));
        FieldStyle.FocusedForegroundColor = FSlateColor(FLinearColor::White);
        FieldStyle.ReadOnlyForegroundColor = FSlateColor(FLinearColor(0.62f, 0.62f, 0.60f, 1.0f));
        FieldStyle.BackgroundImageNormal.TintColor = FSlateColor(FLinearColor(0.055f, 0.062f, 0.072f, 1.0f));
        FieldStyle.BackgroundImageHovered.TintColor = FSlateColor(FLinearColor(0.075f, 0.085f, 0.098f, 1.0f));
        FieldStyle.BackgroundImageFocused.TintColor = FSlateColor(FLinearColor(0.085f, 0.098f, 0.115f, 1.0f));
        FieldStyle.BackgroundImageReadOnly.TintColor = FSlateColor(FLinearColor(0.040f, 0.046f, 0.054f, 1.0f));
        FieldStyle.Padding = FMargin(14.0f, 10.0f);
        Field->SetWidgetStyle(FieldStyle);

        if (UVerticalBoxSlot* Slot = Parent->AddChildToVerticalBox(Field))
        {
            Slot->SetPadding(FMargin(0.0f, 5.0f));
        }
        return Field;
    }

    void R13FrontendSetButtonLabel(UButton* Button, const FText& Label)
    {
        if (!Button) return;
        if (UTextBlock* Text = Cast<UTextBlock>(Button->GetContent())) Text->SetText(Label);
    }

    void R13FrontendSetButtonState(UButton* Button, bool bVisible)
    {
        if (!Button) return;
        Button->SetIsEnabled(bVisible);
        if (UWidget* Parent = Button->GetParent())
        {
            Parent->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
        }
        else
        {
            Button->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
        }
    }

    void R13FrontendFillCanvas(UCanvasPanelSlot* Slot, int32 ZOrder)
    {
        if (!Slot) return;
        Slot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
        Slot->SetOffsets(FMargin(0.0f));
        Slot->SetAlignment(FVector2D::ZeroVector);
        Slot->SetZOrder(ZOrder);
    }

    void R13FrontendPlaceGradientStrip(UCanvasPanelSlot* Slot, float Left, float Width, int32 ZOrder)
    {
        if (!Slot) return;
        Slot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 1.0f));
        Slot->SetOffsets(FMargin(Left, 0.0f, Width, 0.0f));
        Slot->SetAlignment(FVector2D::ZeroVector);
        Slot->SetZOrder(ZOrder);
    }

    void R13FrontendSetPanelGeometry(UBorder* Panel, const FVector2D& Position, const FVector2D& Size)
    {
        if (!Panel) return;
        if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(Panel->Slot))
        {
            Slot->SetPosition(Position);
            Slot->SetSize(Size);
        }
    }

    FString R13SanitizeTravelName(FString Value)
    {
        Value.TrimStartAndEndInline();
        Value.ReplaceInline(TEXT("?"), TEXT(""));
        Value.ReplaceInline(TEXT("&"), TEXT(""));
        Value.ReplaceInline(TEXT("="), TEXT(""));
        Value.ReplaceInline(TEXT(" "), TEXT("_"));
        if (Value.IsEmpty()) Value = TEXT("Player");
        return Value.Left(24);
    }

    FString R13NormalizeDifficulty(FString Value)
    {
        Value.TrimStartAndEndInline();
        if (Value.Equals(TEXT("Easy"), ESearchCase::IgnoreCase)) return TEXT("Easy");
        if (Value.Equals(TEXT("Hard"), ESearchCase::IgnoreCase)) return TEXT("Hard");
        if (Value.Equals(TEXT("Veteran"), ESearchCase::IgnoreCase)) return TEXT("Veteran");
        return TEXT("Normal");
    }
}

bool UOCR13FrontendMenuSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13FrontendMenuSubsystem::Tick(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World) return;

    AOCPlayerController* PC = Cast<AOCPlayerController>(World->GetFirstPlayerController());
    if (!PC || !PC->IsLocalController()) return;

    UOCGameUIRootWidget* Root = nullptr;
    for (TObjectIterator<UOCGameUIRootWidget> It; It; ++It)
    {
        if (IsValid(*It) && It->GetWorld() == World && It->GetOwningPlayer() == PC)
        {
            Root = *It;
            break;
        }
    }
    if (!Root) return;

    EnsureFrontend(Root, PC);

    // Pass 26: a UButton::OnClicked delegate fires inside Slate's mouse-up routing. Merely setting
    // a flag and consuming it later in the same engine frame is not a strong enough lifetime fence.
    // Every frontend action now waits until at least the following engine frame before it is allowed
    // to touch widget visibility, input modes, travel, settings or quit/disconnect state.
    const bool bDeferredActionReady = PendingActionEarliestFrame != 0 && GFrameCounter >= PendingActionEarliestFrame;
    if (bDeferredActionReady)
    {
        PendingActionEarliestFrame = 0;

        if (bPendingPauseResume)
        {
            bPendingPauseResume = false;
            bPauseMenuActive = false;
            bPausePageApplied = false;
            if (PC->IsFrontendMenuVisible()) PC->UIToggleFrontend();
            ReleaseMenuInput();
            SetPresentationVisibility(false, false, false);
            UE_LOG(LogTemp, Display, TEXT("PASS26_FRONTEND_ACTION_EXECUTE action=pause_resume"));
            return;
        }

        if (PendingPage != INDEX_NONE)
        {
            // Pass 29: runtime repeatedly crashed inside Slate immediately after the main-menu START
            // path changed the live widget hierarchy. Page transitions are now forbidden in the R13
            // startup shell. Keep the frontend structurally static and route actions directly instead.
            const int32 BlockedPage = PendingPage;
            PendingPage = INDEX_NONE;
            UE_LOG(LogTemp, Error, TEXT("PASS29_UNSAFE_FRONTEND_PAGE_TRANSITION_BLOCKED page=%d"), BlockedPage);
        }
        if (bPendingHostedStart)
        {
            bPendingHostedStart = false;
            UE_LOG(LogTemp, Display, TEXT("PASS24_HOST_START_DEFERRED_EXECUTE"));
            UE_LOG(LogTemp, Display, TEXT("PASS26_FRONTEND_ACTION_EXECUTE action=start_host"));
            StartHostedGameplay();
            return;
        }
        else if (bPendingNetworkConnect)
        {
            bPendingNetworkConnect = false;
            UE_LOG(LogTemp, Display, TEXT("PASS24_NETWORK_CONNECT_DEFERRED_EXECUTE"));
            UE_LOG(LogTemp, Display, TEXT("PASS26_FRONTEND_ACTION_EXECUTE action=network_connect"));
            StartNetworkGameplay();
            return;
        }
        else if (bPendingSettingsOpen)
        {
            bPendingSettingsOpen = false;
            const bool bSettingsOverGameplay = bPauseMenuActive && (bGameplayStarted || PC->GetPawn() != nullptr);
            SetPresentationVisibility(false, !bSettingsOverGameplay, bSettingsOverGameplay);
            PC->UIOpenSettings();
            UE_LOG(LogTemp, Display, TEXT("PASS26_FRONTEND_ACTION_EXECUTE action=settings"));
            return;
        }
        else if (bPendingQuit)
        {
            bPendingQuit = false;
            UE_LOG(LogTemp, Display, TEXT("PASS26_FRONTEND_ACTION_EXECUTE action=quit"));
            if (bPauseMenuActive || bGameplayStarted || PC->GetPawn() != nullptr)
            {
                bPauseMenuActive = false;
                bPausePageApplied = false;
                bGameplayStarted = false;
                Page = 0;
                LastAppliedPage = INDEX_NONE;
                ReleaseMenuInput();
                SetPresentationVisibility(false, false, false);
                PC->DisconnectFromServer();
                return;
            }

            UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
            return;
        }
    }

    const bool bSettingsVisible = PC->IsSettingsVisible();
    const bool bFrontendVisible = PC->IsFrontendMenuVisible() && !bSettingsVisible;
    const bool bDeploymentVisible = !bSettingsVisible && PC->IsDeploymentPanelVisible();
    const bool bLiveGameplay = bGameplayStarted || PC->GetPawn() != nullptr;

    if (bLocalTravelPending && PC->GetPawn() == nullptr && !bSettingsVisible && !bDeploymentVisible)
    {
        bPauseMenuActive = false;
        bPausePageApplied = false;
        SetPresentationVisibility(true, true, false);
        ForceMenuInput();
        return;
    }

    if (bDeploymentVisible && !bFrontendVisible)
    {
        bPauseMenuActive = false;
        bPausePageApplied = false;
        SetPresentationVisibility(false, true, false);
        return;
    }

    if (bSettingsVisible)
    {
        const bool bSettingsOverGameplay = bPauseMenuActive && bLiveGameplay;
        if (UBorder* SettingsPanel = Cast<UBorder>(Root->GetWidgetFromName(TEXT("SettingsPanel"))))
        {
            SettingsPanel->SetBrushColor(FLinearColor(0.045f, 0.055f, 0.066f, 1.0f));
        }
        SetPresentationVisibility(false, !bSettingsOverGameplay, bSettingsOverGameplay);
        return;
    }

    if (!bFrontendVisible)
    {
        bPauseMenuActive = false;
        bPausePageApplied = false;
        SetPresentationVisibility(false, false, false);
        return;
    }

    if (bLiveGameplay)
    {
        SetPresentationVisibility(true, false, true);
        ApplyPausePage();
    }
    else
    {
        SetPresentationVisibility(true, true, false);
        ApplyPage();
    }

    ForceMenuInput();
}

void UOCR13FrontendMenuSubsystem::EnsureFrontend(UOCGameUIRootWidget* Root, AOCPlayerController* PC)
{
    if (!Root || !PC) return;

    if (ActiveRoot.Get() == Root && MenuBox.IsValid() && MenuPanel.IsValid())
    {
        if (ActiveController.Get() != PC) bMenuInputArmed = false;
        ActiveController = PC;
        return;
    }

    ActiveRoot = Root;
    ActiveController = PC;
    Page = 0;
    PendingPage = INDEX_NONE;
    LastAppliedPage = INDEX_NONE;
    bPendingHostedStart = false;
    bPendingNetworkConnect = false;
    bPendingSettingsOpen = false;
    bPendingQuit = false;
    bPendingPauseResume = false;
    PendingActionEarliestFrame = 0;
    bMenuInputArmed = false;
    bGameplayStarted = false;
    bPauseMenuActive = false;
    bPausePageApplied = false;
    bLocalTravelPending = false;
    bPresentationStateValid = false;
    BuildFrontend(Root, PC);

    // Pass 26: legacy-layer suppression mutates widget state. Do it once for a newly built root,
    // not on every world Tick while Slate is painting/processing input.
    SuppressLegacyFrontendLayers(Root);
    UE_LOG(LogTemp, Display, TEXT("PASS26_LEGACY_FRONTEND_SUPPRESSED_ONCE"));

    // -Frontend belongs only to the startup shell. The same process keeps its command line after
    // `open ?listen`, so the newly created listen/client controller used to resurrect the main menu
    // over Deployment and produced a second START. Deployment owns all pre-spawn choices after travel.
    if (PC->GetNetMode() != NM_Standalone && PC->IsFrontendMenuVisible() &&
        PC->IsDeploymentPanelVisible() && PC->GetPawn() == nullptr)
    {
        PC->UIToggleFrontend();
        SetPresentationVisibility(false, true, false);
        UE_LOG(LogTemp, Display, TEXT("PASS14_FRONTEND_TRAVEL_HANDOFF_READY netmode=%d"), static_cast<int32>(PC->GetNetMode()));
    }
}

void UOCR13FrontendMenuSubsystem::BuildFrontend(UOCGameUIRootWidget* Root, AOCPlayerController* PC)
{
    if (!Root || !PC) return;

    UWidgetTree* Tree = Root->WidgetTree;
    if (!Tree)
    {
        UE_LOG(LogTemp, Error, TEXT("PASS27_FRONTEND_WIDGETTREE_MISSING"));
        return;
    }

    UCanvasPanel* Canvas = Cast<UCanvasPanel>(Root->GetWidgetFromName(TEXT("OC_UI_Root")));
    if (!Canvas) return;

    if (UWidget* LegacyFrontend = Root->GetWidgetFromName(TEXT("FrontendPanel")))
    {
        LegacyFrontend->SetVisibility(ESlateVisibility::Collapsed);
        LegacyFrontend->SetIsEnabled(false);
        // Pass 27: keep the native frontend attached to its original WidgetTree. Detaching a widget
        // after UUserWidget::RebuildWidget has already produced Slate children creates an avoidable
        // structural lifetime edge; collapsed + disabled is sufficient to suppress it.
    }

    UBorder* Blocker = Tree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("R13_MenuWorldBlocker"));
    UImage* Background = Tree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("R13_MenuBackground"));
    UBorder* Shade = Tree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("R13_MenuShade"));
    UBorder* Panel = Tree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("R13_MenuPanel"));
    UVerticalBox* Box = Tree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("R13_PlayerFrontend"));
    if (!Blocker || !Background || !Shade || !Panel || !Box) return;

    Blocker->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 1.0f));
    Blocker->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    Blocker->SetIsEnabled(false);
    R13FrontendFillCanvas(Canvas->AddChildToCanvas(Blocker), 70);

    if (UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, TEXT("/Game/R13/UI/Oster_Menu_BG.Oster_Menu_BG")))
    {
        Background->SetBrushFromTexture(Texture, false);
    }
    Background->SetColorAndOpacity(FLinearColor::White);
    Background->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    Background->SetIsEnabled(false);
    R13FrontendFillCanvas(Canvas->AddChildToCanvas(Background), 71);

    Shade->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.40f));
    Shade->SetVisibility(ESlateVisibility::Collapsed);
    Shade->SetIsEnabled(false);
    R13FrontendFillCanvas(Canvas->AddChildToCanvas(Shade), 72);

    MenuGradientLayers.Reset();
    struct FGradientStrip { float Width; float Alpha; };
    const FGradientStrip GradientStrips[] = {
        { 420.0f, 0.23f }, { 520.0f, 0.15f }, { 620.0f, 0.09f },
        { 730.0f, 0.05f }, { 850.0f, 0.02f },
    };
    for (int32 Index = UE_ARRAY_COUNT(GradientStrips) - 1; Index >= 0; --Index)
    {
        UBorder* Gradient = Tree->ConstructWidget<UBorder>(UBorder::StaticClass());
        if (!Gradient) continue;
        Gradient->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, GradientStrips[Index].Alpha));
        Gradient->SetVisibility(ESlateVisibility::Collapsed);
        Gradient->SetIsEnabled(false);
        R13FrontendPlaceGradientStrip(Canvas->AddChildToCanvas(Gradient), 0.0f, GradientStrips[Index].Width, 73 + Index);
        MenuGradientLayers.Add(Gradient);
    }

    Panel->SetContent(Box);
    Panel->SetIsEnabled(true);
    Panel->SetVisibility(ESlateVisibility::Visible);
    Panel->SetBrushColor(FLinearColor::Transparent);
    Panel->SetPadding(FMargin(0.0f));
    if (UCanvasPanelSlot* PanelSlot = Canvas->AddChildToCanvas(Panel))
    {
        PanelSlot->SetAnchors(FAnchors(0.0f, 0.0f));
        PanelSlot->SetAlignment(FVector2D::ZeroVector);
        PanelSlot->SetPosition(FVector2D(112.0f, 92.0f));
        PanelSlot->SetSize(FVector2D(470.0f, 760.0f));
        PanelSlot->SetZOrder(810);
    }

    UTextBlock* BrandOster = R13FrontendMakeMenuText(Tree, NSLOCTEXT("OCR13Frontend", "BrandOster", "OSTER"), 50, true);
    UTextBlock* BrandConflict = R13FrontendMakeMenuText(Tree, NSLOCTEXT("OCR13Frontend", "BrandConflict", "CONFLICT"), 64, true);
    UTextBlock* Title = R13FrontendMakeMenuText(Tree, FText::GetEmpty(), 32, true);
    UTextBlock* Subtitle = R13FrontendMakeMenuText(Tree, NSLOCTEXT("OCR13Frontend", "Subtitle", "ОСТЕР  •  ГОЛОВНЕ МЕНЮ"), 14, false);
    if (!BrandOster || !BrandConflict || !Title || !Subtitle) return;

    R13FrontendApplyTypeface(BrandOster, FName(TEXT("Light")), 180);
    R13FrontendApplyTypeface(BrandConflict, FName(TEXT("Bold")), 18);
    R13FrontendApplyTypeface(Title, FName(TEXT("Bold")), 18);
    R13FrontendApplyTypeface(Subtitle, FName(TEXT("Regular")), 70);

    Box->AddChildToVerticalBox(BrandOster)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, -6.0f));
    Box->AddChildToVerticalBox(BrandConflict)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
    Box->AddChildToVerticalBox(Title)->SetPadding(FMargin(0.0f, 2.0f, 0.0f, 7.0f));
    Box->AddChildToVerticalBox(Subtitle)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 24.0f));

    UButton* Primary = R13FrontendMakeMenuButton(Tree, Box, NSLOCTEXT("OCR13Frontend", "Start", "СТАРТ"));
    UButton* Secondary = R13FrontendMakeMenuButton(Tree, Box, NSLOCTEXT("OCR13Frontend", "Back", "НАЗАД"));
    UButton* Network = R13FrontendMakeMenuButton(Tree, Box, NSLOCTEXT("OCR13Frontend", "Network", "МЕРЕЖЕВА ГРА"));

    UVerticalBox* Fields = Tree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("R13_FrontendFields"));
    UTextBlock* Status = R13FrontendMakeMenuText(Tree, FText::GetEmpty(), 12, false);
    if (!Fields || !Status) return;

    const UOCPlayerUserSettings* Prefs = UOCPlayerUserSettings::Get();
    UEditableTextBox* Username = R13FrontendMakeField(
        Tree, Fields, NSLOCTEXT("OCR13Frontend", "UsernameHint", "Ім'я гравця"),
        Prefs ? Prefs->GetSavedUsername() : FString(TEXT("Player")));
    UEditableTextBox* Address = R13FrontendMakeField(
        Tree, Fields, NSLOCTEXT("OCR13Frontend", "AddressHint", "IP:порт сервера"),
        Prefs ? Prefs->GetLastServerAddress() : FString(TEXT("127.0.0.1:7777")));
    UEditableTextBox* MaxPlayers = R13FrontendMakeField(
        Tree, Fields, NSLOCTEXT("OCR13Frontend", "MaxPlayersHint", "Максимум гравців (2–64)"), TEXT("16"));
    UEditableTextBox* Bots = R13FrontendMakeField(
        Tree, Fields, NSLOCTEXT("OCR13Frontend", "BotsHint", "Кількість ботів (0–63)"), TEXT("0"));
    UEditableTextBox* Difficulty = R13FrontendMakeField(
        Tree, Fields, NSLOCTEXT("OCR13Frontend", "DifficultyHint", "Складність: Easy / Normal / Hard / Veteran"), TEXT("Normal"));
    if (!Username || !Address || !MaxPlayers || !Bots || !Difficulty) return;

    Fields->AddChildToVerticalBox(Status)->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 5.0f));
    Box->AddChildToVerticalBox(Fields)->SetPadding(FMargin(0.0f, 5.0f));

    UButton* Settings = R13FrontendMakeMenuButton(Tree, Box, NSLOCTEXT("OCR13Frontend", "Settings", "НАЛАШТУВАННЯ"));
    UButton* Quit = R13FrontendMakeMenuButton(Tree, Box, NSLOCTEXT("OCR13Frontend", "Quit", "ВИЙТИ З ГРИ"));
    if (!Primary || !Secondary || !Network || !Settings || !Quit) return;

    Primary->OnClicked.AddDynamic(this, &UOCR13FrontendMenuSubsystem::OnPrimaryClicked);
    Secondary->OnClicked.AddDynamic(this, &UOCR13FrontendMenuSubsystem::OnSecondaryClicked);
    Network->OnClicked.AddDynamic(this, &UOCR13FrontendMenuSubsystem::OnNetworkClicked);
    Settings->OnClicked.AddDynamic(this, &UOCR13FrontendMenuSubsystem::OnSettingsClicked);
    Quit->OnClicked.AddDynamic(this, &UOCR13FrontendMenuSubsystem::OnQuitClicked);

    WorldBlocker = Blocker;
    MenuBackground = Background;
    MenuShade = Shade;
    MenuPanel = Panel;
    MenuBox = Box;
    BrandOsterText = BrandOster;
    BrandConflictText = BrandConflict;
    TitleText = Title;
    SubtitleText = Subtitle;
    FieldsBox = Fields;
    UsernameEntry = Username;
    AddressEntry = Address;
    MaxPlayersEntry = MaxPlayers;
    BotsEntry = Bots;
    BotDifficultyEntry = Difficulty;
    StatusText = Status;
    PrimaryButton = Primary;
    SecondaryButton = Secondary;
    NetworkButton = Network;
    SettingsButton = Settings;
    QuitButton = Quit;

    ApplyPage();
    UE_LOG(LogTemp, Display, TEXT("PASS27_FRONTEND_WIDGETTREE_OWNED"));
}

void UOCR13FrontendMenuSubsystem::ApplyPage()
{
    if (!MenuBox.IsValid() || LastAppliedPage == Page) return;
    bPauseMenuActive = false;
    bPausePageApplied = false;

    if (MenuPanel.IsValid())
    {
        const bool bMainPage = Page == 0;
        MenuPanel->SetBrushColor(bMainPage
            ? FLinearColor::Transparent
            : FLinearColor(0.025f, 0.030f, 0.036f, 0.96f));
        MenuPanel->SetPadding(bMainPage ? FMargin(0.0f) : FMargin(22.0f));
        R13FrontendSetPanelGeometry(MenuPanel.Get(),
            bMainPage ? FVector2D(112.0f, 92.0f) : FVector2D(112.0f, 106.0f),
            bMainPage ? FVector2D(470.0f, 760.0f) : FVector2D(500.0f, 700.0f));
    }

    if (Page == 0)
    {
        if (BrandOsterText.IsValid()) BrandOsterText->SetVisibility(ESlateVisibility::Visible);
        if (BrandConflictText.IsValid()) BrandConflictText->SetVisibility(ESlateVisibility::Visible);
        TitleText->SetVisibility(ESlateVisibility::Collapsed);
        SubtitleText->SetText(NSLOCTEXT("OCR13Frontend", "MainSubtitle", "ОСТЕР  •  ГОЛОВНЕ МЕНЮ"));
        SubtitleText->SetVisibility(ESlateVisibility::Visible);
        FieldsBox->SetVisibility(ESlateVisibility::Collapsed);
        R13FrontendSetButtonLabel(PrimaryButton.Get(), NSLOCTEXT("OCR13Frontend", "MainStart", "СТАРТ"));
        R13FrontendSetButtonLabel(NetworkButton.Get(), NSLOCTEXT("OCR13Frontend", "MainNetwork", "МЕРЕЖЕВА ГРА"));
        R13FrontendSetButtonLabel(SettingsButton.Get(), NSLOCTEXT("OCR13Frontend", "MainSettings", "НАЛАШТУВАННЯ"));
        R13FrontendSetButtonLabel(QuitButton.Get(), NSLOCTEXT("OCR13Frontend", "MainQuit", "ВИЙТИ З ГРИ"));
        R13FrontendSetButtonState(PrimaryButton.Get(), true);
        R13FrontendSetButtonState(SecondaryButton.Get(), false);
        R13FrontendSetButtonState(NetworkButton.Get(), true);
        R13FrontendSetButtonState(SettingsButton.Get(), true);
        R13FrontendSetButtonState(QuitButton.Get(), true);
        LastAppliedPage = Page;
        return;
    }

    if (BrandOsterText.IsValid()) BrandOsterText->SetVisibility(ESlateVisibility::Collapsed);
    if (BrandConflictText.IsValid()) BrandConflictText->SetVisibility(ESlateVisibility::Collapsed);
    TitleText->SetVisibility(ESlateVisibility::Visible);
    SubtitleText->SetVisibility(ESlateVisibility::Visible);
    FieldsBox->SetVisibility(ESlateVisibility::Visible);
    UsernameEntry->SetVisibility(ESlateVisibility::Visible);
    R13FrontendSetButtonState(PrimaryButton.Get(), true);
    R13FrontendSetButtonState(SecondaryButton.Get(), true);
    R13FrontendSetButtonState(NetworkButton.Get(), false);
    R13FrontendSetButtonState(SettingsButton.Get(), false);
    R13FrontendSetButtonState(QuitButton.Get(), false);

    if (Page == 1)
    {
        TitleText->SetText(NSLOCTEXT("OCR13Frontend", "HostTitle", "СТВОРЕННЯ СЕРВЕРА"));
        SubtitleText->SetText(NSLOCTEXT("OCR13Frontend", "HostSubtitle", "НАЛАШТУЙТЕ МАТЧ ПЕРЕД ЗАПУСКОМ"));
        AddressEntry->SetVisibility(ESlateVisibility::Collapsed);
        MaxPlayersEntry->SetVisibility(ESlateVisibility::Visible);
        BotsEntry->SetVisibility(ESlateVisibility::Visible);
        BotDifficultyEntry->SetVisibility(ESlateVisibility::Visible);
        StatusText->SetText(NSLOCTEXT("OCR13Frontend", "HostStatus", "Карта: Остер • Режим: Conquest • Після створення: TEAM → SQUAD → ROLE → SPAWN"));
        R13FrontendSetButtonLabel(PrimaryButton.Get(), NSLOCTEXT("OCR13Frontend", "CreateServer", "СТВОРИТИ СЕРВЕР"));
        R13FrontendSetButtonLabel(SecondaryButton.Get(), NSLOCTEXT("OCR13Frontend", "BackHost", "НАЗАД"));
    }
    else
    {
        TitleText->SetText(NSLOCTEXT("OCR13Frontend", "NetworkTitle", "МЕРЕЖЕВА ГРА"));
        SubtitleText->SetText(NSLOCTEXT("OCR13Frontend", "NetworkSubtitle", "ПРЯМЕ ПІДКЛЮЧЕННЯ ДО СЕРВЕРА"));
        AddressEntry->SetVisibility(ESlateVisibility::Visible);
        MaxPlayersEntry->SetVisibility(ESlateVisibility::Collapsed);
        BotsEntry->SetVisibility(ESlateVisibility::Collapsed);
        BotDifficultyEntry->SetVisibility(ESlateVisibility::Collapsed);
        StatusText->SetText(NSLOCTEXT("OCR13Frontend", "NetworkStatus", "Формат адреси: 127.0.0.1:7777"));
        R13FrontendSetButtonLabel(PrimaryButton.Get(), NSLOCTEXT("OCR13Frontend", "Connect", "ПІДКЛЮЧИТИСЯ"));
        R13FrontendSetButtonLabel(SecondaryButton.Get(), NSLOCTEXT("OCR13Frontend", "BackNetwork", "НАЗАД"));
    }

    LastAppliedPage = Page;
}

void UOCR13FrontendMenuSubsystem::ApplyPausePage()
{
    if (!MenuBox.IsValid() || bPausePageApplied) return;
    LastAppliedPage = INDEX_NONE;
    bPauseMenuActive = true;
    bPausePageApplied = true;

    if (MenuPanel.IsValid())
    {
        MenuPanel->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.74f));
        MenuPanel->SetPadding(FMargin(28.0f));
        R13FrontendSetPanelGeometry(MenuPanel.Get(), FVector2D(105.0f, 155.0f), FVector2D(520.0f, 365.0f));
    }

    if (BrandOsterText.IsValid()) BrandOsterText->SetVisibility(ESlateVisibility::Collapsed);
    if (BrandConflictText.IsValid()) BrandConflictText->SetVisibility(ESlateVisibility::Collapsed);
    TitleText->SetText(NSLOCTEXT("OCR13Frontend", "PauseTitle", "МЕНЮ ГРИ"));
    TitleText->SetVisibility(ESlateVisibility::Visible);
    SubtitleText->SetText(NSLOCTEXT("OCR13Frontend", "PauseSubtitle", "ГРУ ПРИЗУПИНЕНО"));
    SubtitleText->SetVisibility(ESlateVisibility::Visible);
    FieldsBox->SetVisibility(ESlateVisibility::Collapsed);

    R13FrontendSetButtonLabel(PrimaryButton.Get(), NSLOCTEXT("OCR13Frontend", "PauseContinue", "ПРОДОВЖИТИ ГРУ"));
    R13FrontendSetButtonState(PrimaryButton.Get(), true);
    R13FrontendSetButtonState(SecondaryButton.Get(), false);
    R13FrontendSetButtonState(NetworkButton.Get(), false);
    R13FrontendSetButtonLabel(SettingsButton.Get(), NSLOCTEXT("OCR13Frontend", "PauseSettings", "НАЛАШТУВАННЯ"));
    R13FrontendSetButtonLabel(QuitButton.Get(), NSLOCTEXT("OCR13Frontend", "PauseLeave", "ВИЙТИ В ГОЛОВНЕ МЕНЮ"));
    R13FrontendSetButtonState(SettingsButton.Get(), true);
    R13FrontendSetButtonState(QuitButton.Get(), true);
}

void UOCR13FrontendMenuSubsystem::SetPresentationVisibility(bool bShowMenu, bool bShowBackdrop, bool bDimGameplay)
{
    // Pass 26: do not invalidate the same Slate visibility tree every Tick when nothing changed.
    if (bPresentationStateValid && bLastShowMenu == bShowMenu &&
        bLastShowBackdrop == bShowBackdrop && bLastDimGameplay == bDimGameplay)
    {
        return;
    }

    const ESlateVisibility MenuVisibility = bShowMenu ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
    const ESlateVisibility BackdropVisibility = bShowBackdrop
        ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;
    const ESlateVisibility ShadeVisibility = bDimGameplay
        ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;
    const ESlateVisibility GradientVisibility = (bShowMenu && bShowBackdrop)
        ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;

    if (WorldBlocker.IsValid())
    {
        WorldBlocker->SetVisibility(BackdropVisibility);
        WorldBlocker->SetIsEnabled(false);
    }
    if (MenuBackground.IsValid()) MenuBackground->SetVisibility(BackdropVisibility);
    if (MenuShade.IsValid())
    {
        MenuShade->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.40f));
        MenuShade->SetVisibility(ShadeVisibility);
    }
    for (const TWeakObjectPtr<UBorder>& Gradient : MenuGradientLayers)
    {
        if (Gradient.IsValid()) Gradient->SetVisibility(GradientVisibility);
    }
    if (MenuPanel.IsValid())
    {
        MenuPanel->SetVisibility(MenuVisibility);
        MenuPanel->SetIsEnabled(bShowMenu);
    }

    bPresentationStateValid = true;
    bLastShowMenu = bShowMenu;
    bLastShowBackdrop = bShowBackdrop;
    bLastDimGameplay = bDimGameplay;
}

void UOCR13FrontendMenuSubsystem::SuppressLegacyFrontendLayers(UOCGameUIRootWidget* Root)
{
    if (!Root) return;

    if (UWidget* LegacyFrontend = Root->GetWidgetFromName(TEXT("FrontendPanel")))
    {
        LegacyFrontend->SetVisibility(ESlateVisibility::Collapsed);
        LegacyFrontend->SetIsEnabled(false);
        // Pass 27: never detach the root-owned legacy frontend after Slate has been built.
    }

    if (UCanvasPanel* Canvas = Cast<UCanvasPanel>(Root->GetWidgetFromName(TEXT("OC_UI_Root"))))
    {
        for (int32 Index = 0; Index < Canvas->GetChildrenCount(); ++Index)
        {
            UWidget* Child = Canvas->GetChildAt(Index);
            if (!Child || Child == WorldBlocker.Get() || Child == MenuBackground.Get() ||
                Child == MenuShade.Get() || Child == MenuPanel.Get())
            {
                continue;
            }

            if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(Child->Slot))
            {
                const int32 ZOrder = Slot->GetZOrder();
                if (ZOrder == -100 || ZOrder == -99)
                {
                    Child->SetVisibility(ESlateVisibility::Collapsed);
                    Child->SetIsEnabled(false);
                }
            }
        }
    }
}

bool UOCR13FrontendMenuSubsystem::HasPendingFrontendAction() const
{
    return PendingPage != INDEX_NONE || bPendingHostedStart || bPendingNetworkConnect ||
        bPendingSettingsOpen || bPendingQuit || bPendingPauseResume;
}

void UOCR13FrontendMenuSubsystem::ArmDeferredActionFence()
{
    PendingActionEarliestFrame = GFrameCounter + 1;
    UE_LOG(LogTemp, Display, TEXT("PASS26_FRONTEND_ACTION_FENCE armed=%llu execute_after=%llu"),
        static_cast<unsigned long long>(GFrameCounter),
        static_cast<unsigned long long>(PendingActionEarliestFrame));
}

void UOCR13FrontendMenuSubsystem::OnPrimaryClicked()
{
    UE_LOG(LogTemp, Display, TEXT("R13 frontend: primary pressed, page=%d pause=%d"), Page, bPauseMenuActive ? 1 : 0);

    if (bLocalTravelPending)
    {
        UE_LOG(LogTemp, Display, TEXT("R13 frontend: server travel already pending; duplicate primary press ignored"));
        return;
    }
    if (HasPendingFrontendAction()) return;

    if (bPauseMenuActive)
    {
        bPendingPauseResume = true;
        ArmDeferredActionFence();
        UE_LOG(LogTemp, Display, TEXT("PASS26_FRONTEND_ACTION_QUEUED action=pause_resume"));
        return;
    }

    if (Page == 0)
    {
        // Pass 29: START no longer turns the already-live Slate tree into a different page. The
        // repeated crash is synchronized with that structural transition, not with compilation.
        // Start the local hosted session directly from the stable main menu using the already-created
        // default/saved fields (16 max, 0 bots, Normal unless later changed by a dedicated safe UI).
        bPendingHostedStart = true;
        ArmDeferredActionFence();
        UE_LOG(LogTemp, Display, TEXT("PASS29_MAIN_START_DIRECT_HOST_QUEUED"));
        UE_LOG(LogTemp, Display, TEXT("PASS26_FRONTEND_ACTION_QUEUED action=start_host"));
        return;
    }

    if (Page == 1)
    {
        bPendingHostedStart = true;
        ArmDeferredActionFence();
        UE_LOG(LogTemp, Display, TEXT("PASS24_HOST_START_DEFERRED_QUEUED"));
        UE_LOG(LogTemp, Display, TEXT("PASS26_FRONTEND_ACTION_QUEUED action=start_host"));
        return;
    }

    bPendingNetworkConnect = true;
    ArmDeferredActionFence();
    UE_LOG(LogTemp, Display, TEXT("PASS24_NETWORK_CONNECT_DEFERRED_QUEUED"));
    UE_LOG(LogTemp, Display, TEXT("PASS26_FRONTEND_ACTION_QUEUED action=network_connect"));
}

void UOCR13FrontendMenuSubsystem::OnSecondaryClicked()
{
    // Pass 29: no runtime page mutation exists in the startup shell anymore. Secondary is retained
    // only for compatibility with old constructed widgets and must never alter the live Slate tree.
    UE_LOG(LogTemp, Display, TEXT("PASS29_SECONDARY_IGNORED_STATIC_FRONTEND"));
}

void UOCR13FrontendMenuSubsystem::OnNetworkClicked()
{
    if (bPauseMenuActive || bLocalTravelPending || HasPendingFrontendAction()) return;
    // Pass 29: keep the startup Slate hierarchy immutable. Network uses the saved/default address
    // already present in AddressEntry instead of opening the crash-prone structural page transition.
    bPendingNetworkConnect = true;
    ArmDeferredActionFence();
    UE_LOG(LogTemp, Display, TEXT("PASS29_NETWORK_DIRECT_CONNECT_QUEUED"));
    UE_LOG(LogTemp, Display, TEXT("PASS26_FRONTEND_ACTION_QUEUED action=network_connect"));
}

void UOCR13FrontendMenuSubsystem::OnSettingsClicked()
{
    if (bLocalTravelPending || HasPendingFrontendAction()) return;
    bPendingSettingsOpen = true;
    ArmDeferredActionFence();
    UE_LOG(LogTemp, Display, TEXT("PASS26_FRONTEND_ACTION_QUEUED action=settings"));
}

void UOCR13FrontendMenuSubsystem::OnQuitClicked()
{
    if (bLocalTravelPending || HasPendingFrontendAction()) return;
    bPendingQuit = true;
    ArmDeferredActionFence();
    UE_LOG(LogTemp, Display, TEXT("PASS26_FRONTEND_ACTION_QUEUED action=quit"));
}

void UOCR13FrontendMenuSubsystem::StartNetworkGameplay()
{
    AOCPlayerController* PC = ActiveController.Get();
    if (!PC || bLocalTravelPending) return;

    const FString Username = UsernameEntry.IsValid() ? UsernameEntry->GetText().ToString() : FString(TEXT("Player"));
    const FString Address = AddressEntry.IsValid() ? AddressEntry->GetText().ToString() : FString(TEXT("127.0.0.1:7777"));
    if (UOCPlayerUserSettings* Prefs = UOCPlayerUserSettings::Get()) Prefs->SetFrontendIdentity(Username, Address);
    bGameplayStarted = true;
    ReleaseMenuInput();
    SetPresentationVisibility(false, false, false);
    PC->UIConnect(Address, Username);
}

void UOCR13FrontendMenuSubsystem::StartHostedGameplay()
{
    AOCPlayerController* PC = ActiveController.Get();
    if (!PC || bLocalTravelPending || PC->GetNetMode() != NM_Standalone) return;

    FString Username = UsernameEntry.IsValid() ? UsernameEntry->GetText().ToString() : FString(TEXT("Player"));
    Username = R13SanitizeTravelName(Username);
    const int32 MaxPlayers = FMath::Clamp(
        FCString::Atoi(*(MaxPlayersEntry.IsValid() ? MaxPlayersEntry->GetText().ToString() : FString(TEXT("16")))), 2, 64);
    const int32 Bots = FMath::Clamp(
        FCString::Atoi(*(BotsEntry.IsValid() ? BotsEntry->GetText().ToString() : FString(TEXT("0")))), 0, MaxPlayers);
    const FString Difficulty = R13NormalizeDifficulty(
        BotDifficultyEntry.IsValid() ? BotDifficultyEntry->GetText().ToString() : FString(TEXT("Normal")));

    if (UOCPlayerUserSettings* Prefs = UOCPlayerUserSettings::Get())
    {
        Prefs->SetFrontendIdentity(Username,
            Prefs->GetLastServerAddress().IsEmpty() ? FString(TEXT("127.0.0.1:7777")) : Prefs->GetLastServerAddress());
    }
    PC->SetNickname(Username);

    bPauseMenuActive = false;
    bPausePageApplied = false;
    bLocalTravelPending = true;
    bGameplayStarted = false;
    SetPresentationVisibility(true, true, false);
    ForceMenuInput();

    // Production host route. No LocationTest and no AutoDeploy: after server creation the human
    // remains controller-only in Deployment until TEAM -> SQUAD -> ROLE -> SPAWN -> У БІЙ is committed.
    const FString Travel = FString::Printf(
        TEXT("open /Game/Maps/OsterConflict_Runtime?listen?Mode=Conquest?Name=%s?Bots=%d?Population=%d?BotFill=0?MaxPlayers=%d?BotDifficulty=%s?PerfProfile=LowCPU?R13Gameplay=1"),
        *Username, Bots, Bots, MaxPlayers, *Difficulty);
    UE_LOG(LogTemp, Display,
        TEXT("PASS14_HOST_TRAVEL_BEGIN max_players=%d bots=%d difficulty=%s"), MaxPlayers, Bots, *Difficulty);
    UE_LOG(LogTemp, Display, TEXT("PASS29_STATIC_FRONTEND_HOST_TRAVEL_EXECUTE"));
    PC->ConsoleCommand(Travel);
}

void UOCR13FrontendMenuSubsystem::ForceMenuInput()
{
    AOCPlayerController* PC = ActiveController.Get();
    if (!PC || !MenuBox.IsValid()) return;

    // Pass 25: OnClicked fires on mouse release. Re-applying SetInputMode every world Tick
    // can reset Slate mouse capture between press and release, leaving every button visually
    // present but inert. Arm UI input once per menu/controller lifecycle instead.
    if (bMenuInputArmed) return;

    PC->ResetIgnoreMoveInput();
    PC->ResetIgnoreLookInput();
    PC->SetIgnoreMoveInput(true);
    PC->SetIgnoreLookInput(true);
    PC->bShowMouseCursor = true;
    PC->bEnableClickEvents = true;
    PC->bEnableMouseOverEvents = true;

    FInputModeUIOnly Mode;
    Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    PC->SetInputMode(Mode);
    bMenuInputArmed = true;
    UE_LOG(LogTemp, Display, TEXT("PASS25_MENU_INPUT_ARMED"));
}

void UOCR13FrontendMenuSubsystem::ReleaseMenuInput()
{
    bMenuInputArmed = false;
    AOCPlayerController* PC = ActiveController.Get();
    if (!PC) return;

    PC->ResetIgnoreMoveInput();
    PC->ResetIgnoreLookInput();
    PC->bShowMouseCursor = false;
    PC->bEnableClickEvents = false;
    PC->bEnableMouseOverEvents = false;
    if (PC->PlayerInput) PC->PlayerInput->FlushPressedKeys();
    PC->SetInputMode(FInputModeGameOnly());
}

TStatId UOCR13FrontendMenuSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCR13FrontendMenuSubsystem, STATGROUP_Tickables);
}