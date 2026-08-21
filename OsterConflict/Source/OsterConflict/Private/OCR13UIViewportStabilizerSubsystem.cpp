#include "OCR13UIViewportStabilizerSubsystem.h"

#include "OCGameUIRootWidget.h"
#include "OCPlayerController.h"

#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Widget.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "UObject/UObjectIterator.h"

bool UOCR13UIViewportStabilizerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13UIViewportStabilizerSubsystem::Tick(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World || DeltaTime < 0.0f) return;

    AOCPlayerController* PC = Cast<AOCPlayerController>(World->GetFirstPlayerController());
    if (!PC || !PC->IsLocalController()) return;

    // World-render suppression is allowed only before a gameplay pawn exists. A stale deployment/settings flag
    // must never leave an already possessed player on the black HUD-only screen seen in the 2026-08-21 regression.
    const bool bHasGameplayPawn = IsValid(PC->GetPawn());
    const bool bPreGamePresentationVisible = !bHasGameplayPawn &&
        (PC->IsFrontendMenuVisible() || PC->IsSettingsVisible() || PC->IsDeploymentPanelVisible());
    SetWorldRenderingSuppressed(bPreGamePresentationVisible);

    // Only the startup main menu gets hard widget-layer isolation. Deployment/settings need their own root widgets.
    const bool bStartupMenuVisible = PC->IsFrontendMenuVisible() && !PC->IsSettingsVisible() && !bHasGameplayPawn;

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

    StabilizeDeployment(Root);
    ApplyStartupIsolation(Root, bStartupMenuVisible);
}

void UOCR13UIViewportStabilizerSubsystem::Deinitialize()
{
    // GameViewportClient survives world travel. Never allow a frontend world teardown to leave rendering disabled
    // for the newly opened gameplay world.
    SetWorldRenderingSuppressed(false);
    StartupSuppressedWidgets.Reset();
    bStartupIsolationActive = false;
    Super::Deinitialize();
}

void UOCR13UIViewportStabilizerSubsystem::SetWorldRenderingSuppressed(const bool bSuppress)
{
    if (bWorldRenderingSuppressed == bSuppress) return;

    if (GEngine && GEngine->GameViewport)
    {
        GEngine->GameViewport->bDisableWorldRendering = bSuppress;
        bWorldRenderingSuppressed = bSuppress;
        return;
    }

    if (!bSuppress)
    {
        bWorldRenderingSuppressed = false;
    }
}

void UOCR13UIViewportStabilizerSubsystem::StabilizeDeployment(UOCGameUIRootWidget* Root) const
{
    if (!Root) return;

    UBorder* DeploymentPanel = Cast<UBorder>(Root->GetWidgetFromName(TEXT("DeploymentPanel")));
    if (!DeploymentPanel) return;

    DeploymentPanel->SetClipping(EWidgetClipping::ClipToBounds);
    UHorizontalBox* Columns = Cast<UHorizontalBox>(DeploymentPanel->GetContent());
    if (!Columns) return;

    Columns->SetClipping(EWidgetClipping::ClipToBounds);
    const float ColumnWeights[] = { 0.58f, 0.18f, 0.24f };
    const int32 ColumnCount = FMath::Min(Columns->GetChildrenCount(), static_cast<int32>(UE_ARRAY_COUNT(ColumnWeights)));
    for (int32 Index = 0; Index < ColumnCount; ++Index)
    {
        UWidget* Child = Columns->GetChildAt(Index);
        if (!Child) continue;
        if (UHorizontalBoxSlot* Slot = Cast<UHorizontalBoxSlot>(Child->Slot))
        {
            FSlateChildSize FillSize;
            FillSize.SizeRule = ESlateSizeRule::Fill;
            FillSize.Value = ColumnWeights[Index];
            Slot->SetSize(FillSize);
            Slot->SetVerticalAlignment(VAlign_Top);
        }
        Child->SetClipping(EWidgetClipping::ClipToBounds);
    }
}

void UOCR13UIViewportStabilizerSubsystem::ApplyStartupIsolation(UOCGameUIRootWidget* Root, const bool bEnable)
{
    if (!Root) return;

    UCanvasPanel* Canvas = Cast<UCanvasPanel>(Root->GetWidgetFromName(TEXT("OC_UI_Root")));
    if (!Canvas) return;

    if (bEnable)
    {
        for (int32 Index = 0; Index < Canvas->GetChildrenCount(); ++Index)
        {
            UWidget* Child = Canvas->GetChildAt(Index);
            if (!Child) continue;

            UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(Child->Slot);
            const int32 ZOrder = Slot ? Slot->GetZOrder() : INDEX_NONE;
            const FString Name = Child->GetName();
            const bool bNamedMenuLayer = Name.StartsWith(TEXT("R13_Menu"));
            const bool bGradientLayer = (ZOrder >= 73 && ZOrder <= 77) || (ZOrder >= 9003 && ZOrder <= 9007);
            const bool bMenuLayer = bNamedMenuLayer || bGradientLayer;

            if (!bMenuLayer)
            {
                const TWeakObjectPtr<UWidget> WeakChild(Child);
                if (!StartupSuppressedWidgets.Contains(WeakChild))
                {
                    StartupSuppressedWidgets.Add(WeakChild, Child->GetVisibility());
                }
                Child->SetVisibility(ESlateVisibility::Collapsed);
                continue;
            }

            if (!Slot) continue;
            if (Name == TEXT("R13_MenuWorldBlocker")) Slot->SetZOrder(9000);
            else if (Name == TEXT("R13_MenuBackground")) Slot->SetZOrder(9001);
            else if (Name == TEXT("R13_MenuShade")) Slot->SetZOrder(9002);
            else if (Name == TEXT("R13_MenuPanel")) Slot->SetZOrder(9010);
            else if (ZOrder >= 73 && ZOrder <= 77) Slot->SetZOrder(9003 + (ZOrder - 73));
        }

        // StartLocalGameplay initiates map travel from the frontend world. Its presentation layer can be collapsed
        // during the same input event before UE tears the old world down, which exposed one empty/grey frame.
        // As long as the authoritative frontend state still says this is the startup shell, keep the opaque fallback
        // and approved menu image alive. The new gameplay world destroys these widgets naturally during travel.
        if (UWidget* MenuBlocker = Root->GetWidgetFromName(TEXT("R13_MenuWorldBlocker")))
        {
            MenuBlocker->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        }
        if (UWidget* MenuBackground = Root->GetWidgetFromName(TEXT("R13_MenuBackground")))
        {
            MenuBackground->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        }

        if (UBorder* MenuPanel = Cast<UBorder>(Root->GetWidgetFromName(TEXT("R13_MenuPanel"))))
        {
            MenuPanel->SetClipping(EWidgetClipping::ClipToBounds);
            if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(MenuPanel->Slot))
            {
                // Keep the full composition inside the 1600x900 logical frame before the root ScaleBox adapts it.
                Slot->SetPosition(FVector2D(90.0f, 60.0f));
                Slot->SetSize(FVector2D(470.0f, 780.0f));
                Slot->SetZOrder(9010);
            }
        }

        bStartupIsolationActive = true;
        return;
    }

    if (!bStartupIsolationActive && StartupSuppressedWidgets.IsEmpty()) return;

    for (const TPair<TWeakObjectPtr<UWidget>, ESlateVisibility>& Pair : StartupSuppressedWidgets)
    {
        if (UWidget* Widget = Pair.Key.Get())
        {
            Widget->SetVisibility(Pair.Value);
        }
    }
    StartupSuppressedWidgets.Reset();
    bStartupIsolationActive = false;
}

TStatId UOCR13UIViewportStabilizerSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCR13UIViewportStabilizerSubsystem, STATGROUP_Tickables);
}
