#include "OCR13UIThemeSubsystem.h"

#include "OCGameUIRootWidget.h"
#include "OCPlayerController.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ComboBoxString.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Styling/SlateTypes.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UObjectIterator.h"

namespace
{
    const FLinearColor ThemeText(0.94f, 0.93f, 0.89f, 1.0f);
    const FLinearColor ThemeMuted(0.68f, 0.69f, 0.67f, 1.0f);
    const FLinearColor ThemeControl(0.045f, 0.052f, 0.055f, 0.96f);
    const FLinearColor ThemeControlHover(0.16f, 0.145f, 0.105f, 0.98f);
    const FLinearColor ThemeAccent(0.48f, 0.40f, 0.25f, 1.0f);
    const FLinearColor ThemeAccentSoft(0.30f, 0.255f, 0.17f, 1.0f);

    void FillCanvas(UCanvasPanelSlot* Slot, int32 ZOrder)
    {
        if (!Slot) return;
        Slot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
        Slot->SetOffsets(FMargin(0.0f));
        Slot->SetAlignment(FVector2D::ZeroVector);
        Slot->SetZOrder(ZOrder);
    }

    void PlaceFeather(UCanvasPanelSlot* Slot, float Left, float Width, int32 ZOrder)
    {
        if (!Slot) return;
        Slot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 1.0f));
        Slot->SetOffsets(FMargin(Left, 0.0f, Width, 0.0f));
        Slot->SetAlignment(FVector2D::ZeroVector);
        Slot->SetZOrder(ZOrder);
    }

    void ThemeButton(UButton* Button)
    {
        if (!Button) return;

        FButtonStyle Style = Button->GetStyle();
        Style.Normal.TintColor = FSlateColor(ThemeControl);
        Style.Hovered.TintColor = FSlateColor(ThemeControlHover);
        Style.Pressed.TintColor = FSlateColor(ThemeAccentSoft);
        Style.Disabled.TintColor = FSlateColor(FLinearColor(0.035f, 0.040f, 0.042f, 0.55f));
        Style.NormalPadding = FMargin(1.0f);
        Style.PressedPadding = FMargin(1.0f, 2.0f, 1.0f, 0.0f);
        Button->SetStyle(Style);
        Button->SetBackgroundColor(FLinearColor::White);

        if (UTextBlock* Label = Cast<UTextBlock>(Button->GetContent()))
        {
            Label->SetColorAndOpacity(FSlateColor(ThemeText));
        }
    }

    void TintRowBrush(FSlateBrush& Brush, const FLinearColor& Color)
    {
        Brush.TintColor = FSlateColor(Color);
    }

    void ThemeCombo(UComboBoxString* Combo)
    {
        if (!Combo) return;

        FComboBoxStyle ComboStyle = Combo->GetWidgetStyle();
        FButtonStyle ComboButton = ComboStyle.ComboButtonStyle.ButtonStyle;
        ComboButton.Normal.TintColor = FSlateColor(ThemeControl);
        ComboButton.Hovered.TintColor = FSlateColor(ThemeControlHover);
        ComboButton.Pressed.TintColor = FSlateColor(ThemeAccentSoft);
        ComboButton.Disabled.TintColor = FSlateColor(FLinearColor(0.035f, 0.040f, 0.042f, 0.60f));
        ComboStyle.ComboButtonStyle.ButtonStyle = ComboButton;
        ComboStyle.ComboButtonStyle.DownArrowImage.TintColor = FSlateColor(ThemeMuted);
        ComboStyle.ComboButtonStyle.MenuBorderBrush.TintColor = FSlateColor(FLinearColor(0.020f, 0.023f, 0.024f, 1.0f));
        ComboStyle.ContentPadding = FMargin(12.0f, 7.0f);
        ComboStyle.MenuRowPadding = FMargin(10.0f, 6.0f);
        Combo->SetWidgetStyle(ComboStyle);
        Combo->ForegroundColor = FSlateColor(ThemeText);
        Combo->SetContentPadding(FMargin(12.0f, 7.0f));

        FTableRowStyle RowStyle = Combo->GetItemStyle();
        TintRowBrush(RowStyle.EvenRowBackgroundBrush, FLinearColor(0.025f, 0.029f, 0.030f, 1.0f));
        TintRowBrush(RowStyle.OddRowBackgroundBrush, FLinearColor(0.030f, 0.034f, 0.035f, 1.0f));
        TintRowBrush(RowStyle.EvenRowBackgroundHoveredBrush, ThemeControlHover);
        TintRowBrush(RowStyle.OddRowBackgroundHoveredBrush, ThemeControlHover);
        TintRowBrush(RowStyle.ActiveBrush, ThemeAccentSoft);
        TintRowBrush(RowStyle.ActiveHoveredBrush, ThemeAccent);
        TintRowBrush(RowStyle.InactiveBrush, ThemeAccentSoft);
        TintRowBrush(RowStyle.InactiveHoveredBrush, ThemeAccent);
        TintRowBrush(RowStyle.SelectorFocusedBrush, ThemeAccent);
        RowStyle.TextColor = FSlateColor(ThemeText);
        RowStyle.SelectedTextColor = FSlateColor(ThemeText);
        Combo->SetItemStyle(RowStyle);

        const FString Selected = Combo->GetSelectedOption();
        Combo->RefreshOptions();
        if (!Selected.IsEmpty()) Combo->SetSelectedOption(Selected);
    }
}

bool UOCR13UIThemeSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13UIThemeSubsystem::Tick(float DeltaTime)
{
    TickAccumulator += DeltaTime;
    if (TickAccumulator < 0.10f) return;
    TickAccumulator = 0.0f;

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

    if (ActiveRoot.Get() != Root)
    {
        ActiveRoot = Root;
        ThemeWorldBlocker.Reset();
        ThemeBackdrop.Reset();
        ThemeFeather.Reset();
    }

    EnsureThemeLayers(Root);
    ApplyTheme(Root, PC);
}

void UOCR13UIThemeSubsystem::EnsureThemeLayers(UOCGameUIRootWidget* Root)
{
    if (!Root || ThemeBackdrop.IsValid()) return;

    UCanvasPanel* Canvas = Cast<UCanvasPanel>(Root->GetWidgetFromName(TEXT("OC_UI_Root")));
    if (!Canvas) return;

    UTexture2D* WhiteTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture"));
    UTexture2D* MenuTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Game/R13/UI/Oster_Menu_BG.Oster_Menu_BG"));

    UImage* Blocker = NewObject<UImage>(Root, TEXT("R13_ThemeOpaqueWorldBlocker"));
    if (Blocker)
    {
        if (WhiteTexture) Blocker->SetBrushFromTexture(WhiteTexture, false);
        Blocker->SetColorAndOpacity(FLinearColor(0.004f, 0.005f, 0.005f, 1.0f));
        Blocker->SetVisibility(ESlateVisibility::Collapsed);
        Blocker->SetIsEnabled(false);
        FillCanvas(Canvas->AddChildToCanvas(Blocker), 77);
        ThemeWorldBlocker = Blocker;
    }

    UImage* Backdrop = NewObject<UImage>(Root, TEXT("R13_ThemeMenuBackdrop"));
    if (Backdrop)
    {
        if (MenuTexture) Backdrop->SetBrushFromTexture(MenuTexture, false);
        Backdrop->SetColorAndOpacity(FLinearColor::White);
        Backdrop->SetVisibility(ESlateVisibility::Collapsed);
        Backdrop->SetIsEnabled(false);
        FillCanvas(Canvas->AddChildToCanvas(Backdrop), 78);
        ThemeBackdrop = Backdrop;
    }

    // Adjacent narrow strips create a much softer left-side feather than the old
    // nested 460/590/730/890/1080 px rectangles, which produced visible bands.
    constexpr int32 StripCount = 18;
    constexpr float StripWidth = 50.0f;
    for (int32 Index = 0; Index < StripCount; ++Index)
    {
        UImage* Strip = NewObject<UImage>(Root);
        if (!Strip) continue;
        if (WhiteTexture) Strip->SetBrushFromTexture(WhiteTexture, false);
        const float T = static_cast<float>(Index) / static_cast<float>(StripCount - 1);
        const float Alpha = FMath::Lerp(0.40f, 0.015f, FMath::Pow(T, 0.82f));
        Strip->SetColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, Alpha));
        Strip->SetVisibility(ESlateVisibility::Collapsed);
        Strip->SetIsEnabled(false);
        PlaceFeather(Canvas->AddChildToCanvas(Strip), StripWidth * Index, StripWidth + 1.0f, 79);
        ThemeFeather.Add(Strip);
    }
}

void UOCR13UIThemeSubsystem::ApplyTheme(UOCGameUIRootWidget* Root, AOCPlayerController* PC)
{
    if (!Root || !PC) return;

    const bool bSettings = PC->IsSettingsVisible();
    const bool bFrontend = PC->IsFrontendMenuVisible() && !bSettings;

    // Mirror the dedicated frontend's decision for main-vs-pause, but settings always
    // use the same approved backdrop so they never sit over a live, unreadable scene.
    bool bShowBackdrop = bSettings;
    if (bFrontend)
    {
        if (UImage* ExistingBackdrop = FindObject<UImage>(Root, TEXT("R13_MenuBackground")))
        {
            bShowBackdrop = ExistingBackdrop->GetVisibility() != ESlateVisibility::Collapsed;
        }
        else
        {
            bShowBackdrop = PC->GetPawn() == nullptr;
        }
    }

    const ESlateVisibility BackdropVisibility = bShowBackdrop
        ? ESlateVisibility::SelfHitTestInvisible
        : ESlateVisibility::Collapsed;

    if (ThemeWorldBlocker.IsValid()) ThemeWorldBlocker->SetVisibility(BackdropVisibility);
    if (ThemeBackdrop.IsValid()) ThemeBackdrop->SetVisibility(BackdropVisibility);
    for (const TWeakObjectPtr<UImage>& Strip : ThemeFeather)
    {
        if (Strip.IsValid()) Strip->SetVisibility(BackdropVisibility);
    }

    if (UBorder* SettingsPanel = Cast<UBorder>(Root->GetWidgetFromName(TEXT("SettingsPanel"))))
    {
        SettingsPanel->SetBrushColor(FLinearColor(0.018f, 0.022f, 0.024f, 0.965f));
        SettingsPanel->SetPadding(FMargin(26.0f));
        if (SettingsPanel->GetVisibility() != ESlateVisibility::Collapsed)
        {
            ThemeWidgetTree(SettingsPanel->GetContent());
        }
    }

    if (UBorder* MenuPanel = FindObject<UBorder>(Root, TEXT("R13_MenuPanel")))
    {
        if (MenuPanel->GetVisibility() != ESlateVisibility::Collapsed)
        {
            ThemeWidgetTree(MenuPanel->GetContent());
        }
    }
}

void UOCR13UIThemeSubsystem::ThemeWidgetTree(UWidget* Widget)
{
    if (!Widget) return;

    if (UTextBlock* Text = Cast<UTextBlock>(Widget))
    {
        const bool bSmall = Text->GetFont().Size <= 13;
        Text->SetColorAndOpacity(FSlateColor(bSmall ? ThemeMuted : ThemeText));
    }
    else if (UButton* Button = Cast<UButton>(Widget))
    {
        ThemeButton(Button);
    }
    else if (UComboBoxString* Combo = Cast<UComboBoxString>(Widget))
    {
        ThemeCombo(Combo);
    }
    else if (USlider* Slider = Cast<USlider>(Widget))
    {
        Slider->SetSliderBarColor(FLinearColor(0.16f, 0.17f, 0.17f, 1.0f));
        Slider->SetSliderHandleColor(ThemeAccent);
    }

    if (UPanelWidget* Panel = Cast<UPanelWidget>(Widget))
    {
        for (int32 Index = 0; Index < Panel->GetChildrenCount(); ++Index)
        {
            ThemeWidgetTree(Panel->GetChildAt(Index));
        }
    }
}

TStatId UOCR13UIThemeSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCR13UIThemeSubsystem, STATGROUP_Tickables);
}
