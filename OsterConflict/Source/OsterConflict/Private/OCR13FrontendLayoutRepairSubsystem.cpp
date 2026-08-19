#include "OCR13FrontendLayoutRepairSubsystem.h"

#include "OCGameUIRootWidget.h"
#include "OCPlayerController.h"

#include "Components/Border.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Engine/World.h"
#include "UObject/UObjectIterator.h"

namespace
{
    constexpr float RepairTimesSeconds[] = { 0.08f, 0.30f, 0.72f };

    void FillViewportSlot(UWidget* Widget, const int32 ZOrder)
    {
        if (!Widget) return;
        if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(Widget->Slot))
        {
            Slot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
            Slot->SetOffsets(FMargin(0.0f));
            Slot->SetAlignment(FVector2D::ZeroVector);
            Slot->SetZOrder(ZOrder);
        }
    }
}

bool UOCR13FrontendLayoutRepairSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13FrontendLayoutRepairSubsystem::Tick(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World || DeltaTime <= 0.0f) return;

    AOCPlayerController* PC = Cast<AOCPlayerController>(World->GetFirstPlayerController());

    // Geometry repair exists only for the pawn-less startup shell. Running these forced prepasses every time ESC
    // opens the in-game pause page was both unnecessary and actively wrong: it overwrote ApplyPausePage geometry
    // with startup-menu dimensions and could produce a visible hitch/jump on each pause transition.
    const bool bStartupFrontendVisible = PC && PC->IsLocalController() &&
        PC->IsFrontendMenuVisible() && !PC->IsSettingsVisible() && PC->GetPawn() == nullptr;
    if (!bStartupFrontendVisible)
    {
        FrontendVisibleAge = 0.0f;
        RepairPass = 0;
        LastRootSize = FVector2D::ZeroVector;
        return;
    }

    FrontendVisibleAge += DeltaTime;
    if (RepairPass < UE_ARRAY_COUNT(RepairTimesSeconds) &&
        FrontendVisibleAge >= RepairTimesSeconds[RepairPass])
    {
        if (RepairFrontendGeometry()) ++RepairPass;
        return;
    }

    // Window maximize/minimize/DPI changes can alter cached viewport geometry after the three startup passes.
    // Detect only a meaningful size change and repair once instead of rebuilding the menu every frame.
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

    const FVector2D RootSize = Root->GetCachedGeometry().GetLocalSize();
    if (RootSize.X < 320.0f || RootSize.Y < 180.0f) return;
    if (LastRootSize.SizeSquared() > KINDA_SMALL_NUMBER && FVector2D::Distance(RootSize, LastRootSize) > 4.0f)
    {
        RepairFrontendGeometry();
    }
    LastRootSize = RootSize;
}

bool UOCR13FrontendLayoutRepairSubsystem::RepairFrontendGeometry()
{
    UWorld* World = GetWorld();
    AOCPlayerController* PC = World ? Cast<AOCPlayerController>(World->GetFirstPlayerController()) : nullptr;
    if (!World || !PC || PC->GetPawn() != nullptr || PC->IsSettingsVisible()) return false;

    UOCGameUIRootWidget* Root = nullptr;
    for (TObjectIterator<UOCGameUIRootWidget> It; It; ++It)
    {
        if (IsValid(*It) && It->GetWorld() == World && It->GetOwningPlayer() == PC)
        {
            Root = *It;
            break;
        }
    }
    if (!Root) return false;

    UBorder* Blocker = FindObjectFast<UBorder>(Root, TEXT("R13_MenuWorldBlocker"));
    UImage* Background = FindObjectFast<UImage>(Root, TEXT("R13_MenuBackground"));
    UBorder* Shade = FindObjectFast<UBorder>(Root, TEXT("R13_MenuShade"));
    UBorder* Panel = FindObjectFast<UBorder>(Root, TEXT("R13_MenuPanel"));
    if (!Blocker || !Background || !Panel) return false;

    FillViewportSlot(Blocker, 70);
    FillViewportSlot(Background, 71);
    FillViewportSlot(Shade, 72);
    Background->SetColorAndOpacity(FLinearColor::White);

    if (UCanvasPanelSlot* PanelSlot = Cast<UCanvasPanelSlot>(Panel->Slot))
    {
        PanelSlot->SetAnchors(FAnchors(0.0f, 0.0f));
        PanelSlot->SetAlignment(FVector2D::ZeroVector);
        PanelSlot->SetPosition(FVector2D(112.0f, 92.0f));
        PanelSlot->SetSize(FVector2D(440.0f, 760.0f));
        PanelSlot->SetZOrder(810);
    }

    // This is the missing part that an OS minimize/restore happened to trigger for the user.
    Root->InvalidateLayoutAndVolatility();
    Root->ForceLayoutPrepass();
    LastRootSize = Root->GetCachedGeometry().GetLocalSize();

    UE_LOG(LogTemp, Verbose,
        TEXT("R13 frontend layout repair pass=%d root=%.0fx%.0f."), RepairPass + 1, LastRootSize.X, LastRootSize.Y);
    return true;
}

TStatId UOCR13FrontendLayoutRepairSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCR13FrontendLayoutRepairSubsystem, STATGROUP_Tickables);
}
