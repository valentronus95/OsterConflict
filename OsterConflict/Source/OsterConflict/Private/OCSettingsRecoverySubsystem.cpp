#include "OCSettingsRecoverySubsystem.h"

#include "OCGameUIRootWidget.h"
#include "OCPlayerController.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Components/PanelWidget.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Engine/World.h"
#include "Styling/SlateTypes.h"
#include "UObject/UObjectIterator.h"

bool UOCSettingsRecoverySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && World->GetNetMode() != NM_DedicatedServer &&
        (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

TStatId UOCSettingsRecoverySubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCSettingsRecoverySubsystem, STATGROUP_Tickables);
}

void UOCSettingsRecoverySubsystem::Tick(float DeltaTime)
{
    (void)DeltaTime;
    UWorld* World = GetWorld();
    if (!World || !World->IsGameWorld()) return;

    AOCPlayerController* PC = Cast<AOCPlayerController>(World->GetFirstPlayerController());
    const bool bSettingsVisible = PC && PC->IsLocalController() && PC->IsSettingsVisible();

    if (bSettingsVisible && !bWasSettingsVisible)
    {
        RestoreSettingsScreen();
    }
    bWasSettingsVisible = bSettingsVisible;
}

void UOCSettingsRecoverySubsystem::RestoreSettingsScreen()
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

    UWidget* SettingsWidget = Root->GetWidgetFromName(TEXT("SettingsPanel"));
    if (!SettingsWidget) return;

    // R13 freezes hidden legacy panels by disabling them. Visibility alone does not undo that state,
    // which is why the settings screen could be seen but every tab/control was dead and washed out.
    SettingsWidget->SetIsEnabled(true);
    SettingsWidget->SetVisibility(ESlateVisibility::Visible);
    SettingsWidget->SetRenderOpacity(1.0f);

    if (UBorder* SettingsBorder = Cast<UBorder>(SettingsWidget))
    {
        SettingsBorder->SetBrushColor(FLinearColor(0.018f, 0.024f, 0.031f, 0.995f));
        SettingsBorder->SetPadding(FMargin(24.0f, 20.0f));
    }

    ApplyProductionStyle(SettingsWidget);
    Root->SetKeyboardFocus();

    UE_LOG(LogTemp, Display,
        TEXT("GAME_RECOVERY_SETTINGS_INTERACTION_READY enabled=1 opacity=1 tabs_input=1 production_theme=1 controls_styled=1"));
}

void UOCSettingsRecoverySubsystem::ApplyProductionStyle(UWidget* Widget)
{
    if (!Widget) return;

    Widget->SetRenderOpacity(1.0f);

    if (UButton* Button = Cast<UButton>(Widget))
    {
        FButtonStyle Style = Button->GetStyle();
        Style.Normal.TintColor = FSlateColor(FLinearColor(0.055f, 0.067f, 0.080f, 0.98f));
        Style.Hovered.TintColor = FSlateColor(FLinearColor(0.12f, 0.145f, 0.17f, 1.0f));
        Style.Pressed.TintColor = FSlateColor(FLinearColor(0.38f, 0.29f, 0.13f, 1.0f));
        Style.Disabled.TintColor = FSlateColor(FLinearColor(0.035f, 0.042f, 0.050f, 0.55f));
        Style.NormalPadding = FMargin(12.0f, 8.0f);
        Style.PressedPadding = FMargin(12.0f, 9.0f, 12.0f, 7.0f);
        Button->SetStyle(Style);
    }
    else if (UComboBoxString* Combo = Cast<UComboBoxString>(Widget))
    {
        FComboBoxStyle Style = Combo->GetWidgetStyle();
        Style.ContentPadding = FMargin(12.0f, 7.0f);
        Style.MenuRowPadding = FMargin(10.0f, 6.0f);
        Style.ComboButtonStyle.ButtonStyle.Normal.TintColor = FSlateColor(FLinearColor(0.045f, 0.055f, 0.066f, 1.0f));
        Style.ComboButtonStyle.ButtonStyle.Hovered.TintColor = FSlateColor(FLinearColor(0.10f, 0.12f, 0.145f, 1.0f));
        Style.ComboButtonStyle.ButtonStyle.Pressed.TintColor = FSlateColor(FLinearColor(0.30f, 0.235f, 0.115f, 1.0f));
        Style.ComboButtonStyle.DownArrowImage.TintColor = FSlateColor(FLinearColor(0.84f, 0.70f, 0.36f, 1.0f));
        Combo->SetWidgetStyle(Style);
        Combo->SetMaxListHeight(420.0f);
    }
    else if (UCheckBox* Check = Cast<UCheckBox>(Widget))
    {
        FCheckBoxStyle Style = Check->GetWidgetStyle();
        Style.UncheckedImage.TintColor = FSlateColor(FLinearColor(0.10f, 0.12f, 0.14f, 1.0f));
        Style.UncheckedHoveredImage.TintColor = FSlateColor(FLinearColor(0.18f, 0.21f, 0.24f, 1.0f));
        Style.UncheckedPressedImage.TintColor = FSlateColor(FLinearColor(0.24f, 0.20f, 0.12f, 1.0f));
        Style.CheckedImage.TintColor = FSlateColor(FLinearColor(0.84f, 0.70f, 0.36f, 1.0f));
        Style.CheckedHoveredImage.TintColor = FSlateColor(FLinearColor(0.94f, 0.81f, 0.48f, 1.0f));
        Style.CheckedPressedImage.TintColor = FSlateColor(FLinearColor(0.72f, 0.57f, 0.25f, 1.0f));
        Style.Padding = FMargin(2.0f);
        Check->SetWidgetStyle(Style);
    }
    else if (USlider* Slider = Cast<USlider>(Widget))
    {
        Slider->SetSliderBarColor(FLinearColor(0.16f, 0.19f, 0.22f, 1.0f));
        Slider->SetSliderHandleColor(FLinearColor(0.84f, 0.70f, 0.36f, 1.0f));
    }
    else if (UTextBlock* Text = Cast<UTextBlock>(Widget))
    {
        // Keep hierarchy bright enough on the near-opaque dark panel instead of inheriting the disabled washout.
        Text->SetRenderOpacity(1.0f);
    }

    if (UPanelWidget* Panel = Cast<UPanelWidget>(Widget))
    {
        for (int32 Index = 0; Index < Panel->GetChildrenCount(); ++Index)
        {
            ApplyProductionStyle(Panel->GetChildAt(Index));
        }
    }
}
