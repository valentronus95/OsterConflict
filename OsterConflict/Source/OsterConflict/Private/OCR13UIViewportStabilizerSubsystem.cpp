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

namespace
{
    constexpr float ViewportStabilizerIntervalSeconds = 0.10f;
}

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

    UpdateAccumulator += FMath::Max(0.0f, DeltaTime);
    if (UpdateAccumulator < ViewportStabilizerIntervalSeconds) return;
    UpdateAccumulator = FMath::Fmod(UpdateAccumulator, ViewportStabilizerIntervalSeconds);

    AOCPlayerController* PC = Cast<AOCPlayerController>(World->GetFirstPlayerController());
    if (!PC || !PC->IsLocalController())
    {
        if (UOCGameUIRootWidget* PreviousRoot = CachedRoot.Get())
        {
            ApplyStartupIsolation(PreviousRoot, false);
        }
        CachedRoot.Reset();
        CachedController.Reset();
        bDeploymentStabilized = false;
        bLastDeploymentVisible = false;
        SetWorldRenderingSuppressed(false);
        return;
    }

    SetWorldRenderingSuppressed(false);

    UOCGameUIRootWidget* Root = ResolveRoot(World, PC);
    if (!Root) return;

    const bool bHasGameplayPawn = IsValid(PC->GetPawn());
    const bool bStartupShell = !bHasGameplayPawn &&
        !PC->IsSettingsVisible() && !PC->IsDeploymentPanelVisible();
    const bool bDeploymentVisible = PC->IsDeploymentPanelVisible();

    if (!bDeploymentStabilized || (bDeploymentVisible && !bLastDeploymentVisible))
    {
        bDeploymentStabilized = StabilizeDeployment(Root);
    }
    bLastDeploymentVisible = bDeploymentVisible;

    if (bStartupShell != bStartupIsolationActive)
    {
        ApplyStartupIsolation(Root, bStartupShell);
    }

    if (!bUpdateBudgetLogged)
    {
        bUpdateBudgetLogged = true;
        UE_LOG(LogTemp, Display,
            TEXT("PASS40_UI_STABILIZER_BUDGET_READY update_hz=10 root_scan=cache_miss layout_writes=transition_only startup_visibility_writes=transition_only"));
    }
}

UOCGameUIRootWidget* UOCR13UIViewportStabilizerSubsystem::ResolveRoot(UWorld* World, AOCPlayerController* PC)
{
    if (!World || !PC) return nullptr;

    if (UOCGameUIRootWidget* ExistingRoot = CachedRoot.Get())
    {
        if (CachedController.Get() == PC && ExistingRoot->GetWorld() == World && ExistingRoot->GetOwningPlayer() == PC)
        {
            return ExistingRoot;
        }

        ApplyStartupIsolation(ExistingRoot, false);
    }
    else
    {
        StartupSuppressedWidgets.Reset();
        bStartupIsolationActive = false;
    }

    CachedRoot.Reset();
    CachedController.Reset();
    bDeploymentStabilized = false;
    bLastDeploymentVisible = false;

    for (TObjectIterator<UOCGameUIRootWidget> It; It; ++It)
    {
        if (IsValid(*It) && It->GetWorld() == World && It->GetOwningPlayer() == PC)
        {
            CachedRoot = *It;
            CachedController = PC;
            return *It;
        }
    }
    return nullptr;
}

void UOCR13UIViewportStabilizerSubsystem::Deinitialize()
{
    if (UOCGameUIRootWidget* Root = CachedRoot.Get())
    {
        ApplyStartupIsolation(Root, false);
    }
    SetWorldRenderingSuppressed(false);
    StartupSuppressedWidgets.Reset();
    CachedRoot.Reset();
    CachedController.Reset();
    bStartupIsolationActive = false;
    bDeploymentStabilized = false;
    bLastDeploymentVisible = false;
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

bool UOCR13UIViewportStabilizerSubsystem::StabilizeDeployment(UOCGameUIRootWidget* Root) const
{
    if (!Root) return false;

    UBorder* DeploymentPanel = Cast<UBorder>(Root->GetWidgetFromName(TEXT("DeploymentPanel")));
    if (!DeploymentPanel) return false;

    DeploymentPanel->SetClipping(EWidgetClipping::ClipToBounds);
    UHorizontalBox* Columns = Cast<UHorizontalBox>(DeploymentPanel->GetContent());
    if (!Columns) return false;

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
    return true;
}

void UOCR13UIViewportStabilizerSubsystem::ApplyStartupIsolation(UOCGameUIRootWidget* Root, const bool bEnable)
{
    if (!Root) return;

    UCanvasPanel* Canvas = Cast<UCanvasPanel>(Root->GetWidgetFromName(TEXT("OC_UI_Root")));
    if (!Canvas) return;

    if (bEnable)
    {
        if (bStartupIsolationActive) return;

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
