#include "OCR13UIThemeSubsystem.h"

#include "OCGameUIRootWidget.h"
#include "OCPlayerController.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableTextBox.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "Styling/SlateTypes.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UObjectIterator.h"

namespace
{
    const FLinearColor ThemeText(0.94f, 0.93f, 0.89f, 1.0f);
    const FLinearColor ThemeMuted(0.67f, 0.68f, 0.66f, 1.0f);
    const FLinearColor ThemePanel(0.014f, 0.018f, 0.020f, 0.985f);
    const FLinearColor ThemeControl(0.055f, 0.060f, 0.060f, 0.70f);
    const FLinearColor ThemeControlHover(0.24f, 0.205f, 0.135f, 0.72f);
    const FLinearColor ThemeAccent(0.49f, 0.41f, 0.26f, 1.0f);
    const FLinearColor ThemeAccentSoft(0.32f, 0.27f, 0.18f, 0.80f);

    void ThemeButton(UButton* Button)
    {
        if (!Button) return;

        FButtonStyle Style = Button->GetStyle();
        Style.Normal.TintColor = FSlateColor(ThemeControl);
        Style.Hovered.TintColor = FSlateColor(ThemeControlHover);
        Style.Pressed.TintColor = FSlateColor(ThemeAccentSoft);
        Style.Disabled.TintColor = FSlateColor(FLinearColor(0.035f, 0.038f, 0.038f, 0.42f));
        Style.NormalPadding = FMargin(1.0f);
        Style.PressedPadding = FMargin(1.0f, 2.0f, 1.0f, 0.0f);
        Button->SetStyle(Style);
        Button->SetBackgroundColor(FLinearColor::White);

        if (UTextBlock* Label = Cast<UTextBlock>(Button->GetContent()))
        {
            Label->SetColorAndOpacity(FSlateColor(ThemeText));
        }
    }

    void ThemeEditable(UEditableTextBox* Entry)
    {
        if (!Entry) return;

        FEditableTextBoxStyle Style = Entry->GetWidgetStyle();
        Style.BackgroundColor = FSlateColor(FLinearColor(0.025f, 0.029f, 0.030f, 0.96f));
        Style.BackgroundImageNormal.TintColor = FSlateColor(FLinearColor(0.16f, 0.17f, 0.17f, 0.88f));
        Style.BackgroundImageHovered.TintColor = FSlateColor(FLinearColor(0.25f, 0.22f, 0.16f, 0.95f));
        Style.BackgroundImageFocused.TintColor = FSlateColor(FLinearColor(0.34f, 0.29f, 0.19f, 1.0f));
        Style.BackgroundImageReadOnly.TintColor = FSlateColor(FLinearColor(0.08f, 0.085f, 0.085f, 0.80f));
        Style.ForegroundColor = FSlateColor(ThemeText);
        Style.FocusedForegroundColor = FSlateColor(ThemeText);
        Style.ReadOnlyForegroundColor = FSlateColor(ThemeMuted);
        Style.TextStyle.ColorAndOpacity = FSlateColor(ThemeText);
        Style.Padding = FMargin(12.0f, 8.0f);
        Entry->SetWidgetStyle(Style);
        Entry->SetForegroundColor(ThemeText);
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
        ComboButton.Normal.TintColor = FSlateColor(FLinearColor(0.055f, 0.060f, 0.060f, 0.92f));
        ComboButton.Hovered.TintColor = FSlateColor(ThemeControlHover);
        ComboButton.Pressed.TintColor = FSlateColor(ThemeAccentSoft);
        ComboButton.Disabled.TintColor = FSlateColor(FLinearColor(0.035f, 0.040f, 0.040f, 0.60f));
        ComboStyle.ComboButtonStyle.ButtonStyle = ComboButton;
        ComboStyle.ComboButtonStyle.DownArrowImage.TintColor = FSlateColor(ThemeMuted);
        ComboStyle.ComboButtonStyle.MenuBorderBrush.TintColor = FSlateColor(FLinearColor(0.018f, 0.021f, 0.022f, 1.0f));
        ComboStyle.ContentPadding = FMargin(12.0f, 7.0f);
        ComboStyle.MenuRowPadding = FMargin(10.0f, 6.0f);
        Combo->SetWidgetStyle(ComboStyle);
        Combo->ForegroundColor = FSlateColor(ThemeText);
        Combo->SetContentPadding(FMargin(12.0f, 7.0f));

        FTableRowStyle RowStyle = Combo->GetItemStyle();
        TintRowBrush(RowStyle.EvenRowBackgroundBrush, FLinearColor(0.020f, 0.023f, 0.024f, 1.0f));
        TintRowBrush(RowStyle.OddRowBackgroundBrush, FLinearColor(0.026f, 0.029f, 0.030f, 1.0f));
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
    }

    void ThemeCheckBox(UCheckBox* Check)
    {
        if (!Check) return;

        FCheckBoxStyle Style = Check->GetWidgetStyle();
        Style.ForegroundColor = FSlateColor(ThemeText);
        Style.HoveredForeground = FSlateColor(ThemeText);
        Style.PressedForeground = FSlateColor(ThemeText);
        Style.CheckedForeground = FSlateColor(ThemeAccent);
        Style.CheckedHoveredForeground = FSlateColor(ThemeAccent);
        Style.CheckedPressedForeground = FSlateColor(ThemeAccent);
        Style.UncheckedImage.TintColor = FSlateColor(FLinearColor(0.20f, 0.21f, 0.21f, 1.0f));
        Style.UncheckedHoveredImage.TintColor = FSlateColor(ThemeControlHover);
        Style.CheckedImage.TintColor = FSlateColor(ThemeAccent);
        Style.CheckedHoveredImage.TintColor = FSlateColor(ThemeAccent);
        Check->SetWidgetStyle(Style);
    }

    void ThemeFrontendTree(UWidget* Widget)
    {
        if (!Widget) return;

        if (UEditableTextBox* Entry = Cast<UEditableTextBox>(Widget))
        {
            ThemeEditable(Entry);
        }
        else if (UTextBlock* Text = Cast<UTextBlock>(Widget))
        {
            const bool bSmall = Text->GetFont().Size <= 13;
            Text->SetColorAndOpacity(FSlateColor(bSmall ? ThemeMuted : ThemeText));
        }

        if (UPanelWidget* Panel = Cast<UPanelWidget>(Widget))
        {
            for (int32 Index = 0; Index < Panel->GetChildrenCount(); ++Index)
            {
                ThemeFrontendTree(Panel->GetChildAt(Index));
            }
        }
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
    if (TickAccumulator < 0.05f) return;
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

    // R13.3: OCR13FrontendMenuSubsystem is the sole owner of menu/deployment/settings backdrop presentation.
    // Older theme-owned copies of Oster_Menu_BG caused two independently ticked layers to fight and visibly flicker.
    EnsureThemeLayers(Root);
    ApplyTheme(Root, PC);
}

void UOCR13UIThemeSubsystem::EnsureThemeLayers(UOCGameUIRootWidget* Root)
{
    if (!Root) return;

    // Do not create another menu image here. Retire any stale layers that may exist after hot reload.
    if (UImage* OldBlocker = FindObject<UImage>(Root, TEXT("R13_ThemeOpaqueWorldBlocker")))
    {
        OldBlocker->SetVisibility(ESlateVisibility::Collapsed);
        OldBlocker->SetIsEnabled(false);
    }
    if (UImage* OldBackdrop = FindObject<UImage>(Root, TEXT("R13_ThemeMenuBackdrop")))
    {
        OldBackdrop->SetVisibility(ESlateVisibility::Collapsed);
        OldBackdrop->SetIsEnabled(false);
    }
    ThemeWorldBlocker.Reset();
    ThemeBackdrop.Reset();
    ThemeFeather.Reset();
}

void UOCR13UIThemeSubsystem::ApplyTheme(UOCGameUIRootWidget* Root, AOCPlayerController* PC)
{
    if (!Root || !PC) return;

    // Backdrop ownership deliberately lives in OCR13FrontendMenuSubsystem. Theme only styles controls.
    if (ThemeWorldBlocker.IsValid()) ThemeWorldBlocker->SetVisibility(ESlateVisibility::Collapsed);
    if (ThemeBackdrop.IsValid()) ThemeBackdrop->SetVisibility(ESlateVisibility::Collapsed);
    for (const TWeakObjectPtr<UImage>& Strip : ThemeFeather)
    {
        if (Strip.IsValid()) Strip->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (UBorder* SettingsPanel = Cast<UBorder>(Root->GetWidgetFromName(TEXT("SettingsPanel"))))
    {
        SettingsPanel->SetBrushColor(FLinearColor(0.014f, 0.018f, 0.020f, 0.985f));
        SettingsPanel->SetPadding(FMargin(26.0f));
        if (SettingsPanel->GetVisibility() != ESlateVisibility::Collapsed)
        {
            ThemeWidgetTree(SettingsPanel->GetContent());
        }
    }

    if (UBorder* DeploymentPanel = Cast<UBorder>(Root->GetWidgetFromName(TEXT("DeploymentPanel"))))
    {
        DeploymentPanel->SetBrushColor(ThemePanel);
        DeploymentPanel->SetPadding(FMargin(28.0f));
        if (DeploymentPanel->GetVisibility() != ESlateVisibility::Collapsed)
        {
            ThemeWidgetTree(DeploymentPanel->GetContent());
        }
    }

    if (UBorder* MenuPanel = FindObject<UBorder>(Root, TEXT("R13_MenuPanel")))
    {
        if (MenuPanel->GetVisibility() != ESlateVisibility::Collapsed)
        {
            ThemeFrontendTree(MenuPanel->GetContent());
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
    else if (UEditableTextBox* Entry = Cast<UEditableTextBox>(Widget))
    {
        ThemeEditable(Entry);
    }
    else if (USlider* Slider = Cast<USlider>(Widget))
    {
        Slider->SetSliderBarColor(FLinearColor(0.16f, 0.17f, 0.17f, 1.0f));
        Slider->SetSliderHandleColor(ThemeAccent);
    }
    else if (UCheckBox* Check = Cast<UCheckBox>(Widget))
    {
        ThemeCheckBox(Check);
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
