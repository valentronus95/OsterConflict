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

    const bool bHasGameplayPawn = IsValid(PC->GetPawn());

    // Pass 6: do not flip GameViewportClient::bDisableWorldRendering during frontend -> gameplay travel.
    // The R13 frontend already owns an opaque blocker/background. Toggling the persistent viewport flag
    // during the asynchronous `open` gap was capable of producing the reported black/grey blink.
    SetWorldRenderingSuppressed(false);

    // Keep startup-layer isolation alive for the complete pawn-less shell, not merely while the old
    // frontend boolean happens to remain true. StartLocalGameplay can clear that flag before the old
    // world is torn down; restoring legacy widgets in that one-frame gap exposed the obsolete menu.
    const bool bStartupShell = !bHasGameplayPawn &&
        !PC->IsSettingsVisible() && !PC->IsDeploymentPanelVisible();

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
    ApplyStartupIsolation(Root, bStartupShell);
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

        // Keep the approved menu image and its opaque fallback alive throughout the travel gap.
        // The gameplay world owns their eventual destruction; the old frontend world must not reveal
        // any legacy layer merely because a transient controller flag changed first.
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
                // Geometry has one owner: OCR13FrontendMenuSubsystem. Isolation may raise Z-order,
                // but must never move or resize the approved menu composition.
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
