#include "OCTacticalMapSubsystem.h"

#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateImageBrush.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"

namespace
{
    const FLinearColor IconPlayer(0.46f, 0.82f, 0.30f, 1.0f);
    const FLinearColor IconFriendly(0.20f, 0.56f, 0.96f, 1.0f);
    const FLinearColor IconAmber(0.95f, 0.55f, 0.12f, 1.0f);
    const FLinearColor IconPrimary(0.86f, 0.90f, 0.92f, 1.0f);
    const FLinearColor IconMuted(0.50f, 0.57f, 0.61f, 1.0f);

    FString ResolveTacticalIconPath(const TCHAR* FileName)
    {
        return FPaths::ConvertRelativePathToFull(FPaths::Combine(
            FPaths::ProjectContentDir(), TEXT("UI/TacticalMap/Icons"), FileName));
    }

    UImage* AddVectorIcon(
        UWidgetTree* Tree,
        UCanvasPanel* Canvas,
        const FName Name,
        const TCHAR* FileName,
        const FVector2D Position,
        const FVector2D Size,
        const FLinearColor& Tint,
        const int32 ZOrder)
    {
        if (!Tree || !Canvas) return nullptr;

        const FString IconPath = ResolveTacticalIconPath(FileName);
        if (!IFileManager::Get().FileExists(*IconPath))
        {
            UE_LOG(LogTemp, Warning, TEXT("Tactical Map icon resource missing: %s"), *IconPath);
            return nullptr;
        }

        UImage* Icon = Tree->ConstructWidget<UImage>(UImage::StaticClass(), Name);
        FSlateVectorImageBrush VectorBrush(IconPath, Size, Tint, ESlateBrushTileType::NoTile);
        Icon->SetBrush(VectorBrush);
        Icon->SetVisibility(ESlateVisibility::HitTestInvisible);

        if (UCanvasPanelSlot* IconCanvasSlot = Canvas->AddChildToCanvas(Icon))
        {
            IconCanvasSlot->SetPosition(Position);
            IconCanvasSlot->SetSize(Size);
            IconCanvasSlot->SetZOrder(ZOrder);
        }
        return Icon;
    }

    void ConfigureLabel(
        UWidgetTree* Tree,
        const FName Name,
        const TCHAR* Value,
        const float NewX,
        const float NewWidth)
    {
        if (!Tree) return;
        UTextBlock* Text = Cast<UTextBlock>(Tree->FindWidget(Name));
        if (!Text) return;

        Text->SetText(FText::FromString(Value));
        if (UCanvasPanelSlot* TextCanvasSlot = Cast<UCanvasPanelSlot>(Text->Slot))
        {
            const FVector2D CurrentPosition = TextCanvasSlot->GetPosition();
            TextCanvasSlot->SetPosition(FVector2D(NewX, CurrentPosition.Y));
            const FVector2D CurrentSize = TextCanvasSlot->GetSize();
            TextCanvasSlot->SetSize(FVector2D(NewWidth, CurrentSize.Y));
        }
    }

    void SetSmallTextSize(UTextBlock* Text, const int32 Size)
    {
        if (!Text) return;
        FSlateFontInfo Font = Text->GetFont();
        Font.Size = Size;
        Text->SetFont(Font);
    }

    void AddKeyCap(
        UWidgetTree* Tree,
        UCanvasPanel* Canvas,
        const FName Name,
        const FString& Key,
        const FVector2D Position)
    {
        if (!Tree || !Canvas) return;

        UBorder* KeyCap = Tree->ConstructWidget<UBorder>(UBorder::StaticClass(), Name);
        KeyCap->SetBrushColor(FLinearColor(0.075f, 0.090f, 0.102f, 1.0f));
        KeyCap->SetPadding(FMargin(0.0f));
        KeyCap->SetHorizontalAlignment(HAlign_Center);
        KeyCap->SetVerticalAlignment(VAlign_Center);

        UTextBlock* KeyText = Tree->ConstructWidget<UTextBlock>();
        KeyText->SetText(FText::FromString(Key));
        KeyText->SetColorAndOpacity(FSlateColor(IconPrimary));
        KeyText->SetJustification(ETextJustify::Center);
        SetSmallTextSize(KeyText, 11);
        KeyCap->SetContent(KeyText);

        if (UCanvasPanelSlot* KeyCapCanvasSlot = Canvas->AddChildToCanvas(KeyCap))
        {
            KeyCapCanvasSlot->SetPosition(Position);
            KeyCapCanvasSlot->SetSize(FVector2D(28.0f, 26.0f));
            KeyCapCanvasSlot->SetZOrder(7);
        }
    }
}

void UOCTacticalMapWidget::InstallTacticalIconography()
{
    if (!WidgetTree) return;
    UCanvasPanel* Root = Cast<UCanvasPanel>(WidgetTree->FindWidget(TEXT("TacticalMapRoot")));
    if (!Root) return;

    ConfigureLabel(WidgetTree, TEXT("LegendPlayer"), TEXT("ГРАВЕЦЬ"), 76.0f, 176.0f);
    ConfigureLabel(WidgetTree, TEXT("LegendSquad"), TEXT("ЧЛЕНИ ЗАГОНУ"), 76.0f, 184.0f);
    ConfigureLabel(WidgetTree, TEXT("LegendVehicle"), TEXT("ТРАНСПОРТ ЗАГОНУ"), 76.0f, 188.0f);
    ConfigureLabel(WidgetTree, TEXT("LegendObjective"), TEXT("КОМАНДА / ЦІЛЬ"), 76.0f, 184.0f);
    ConfigureLabel(WidgetTree, TEXT("LegendPOI"), TEXT("ТОЧКА ІНТЕРЕСУ"), 76.0f, 184.0f);

    AddVectorIcon(WidgetTree, Root, TEXT("LegendIconPlayer"), TEXT("navigation.svg"),
        FVector2D(46.0f, 173.0f), FVector2D(22.0f, 22.0f), IconPlayer, 6);
    AddVectorIcon(WidgetTree, Root, TEXT("LegendIconSquad"), TEXT("users.svg"),
        FVector2D(46.0f, 225.0f), FVector2D(22.0f, 22.0f), IconFriendly, 6);
    AddVectorIcon(WidgetTree, Root, TEXT("LegendIconVehicle"), TEXT("truck.svg"),
        FVector2D(46.0f, 277.0f), FVector2D(22.0f, 22.0f), IconFriendly, 6);
    AddVectorIcon(WidgetTree, Root, TEXT("LegendIconObjective"), TEXT("crosshair.svg"),
        FVector2D(46.0f, 329.0f), FVector2D(22.0f, 22.0f), IconAmber, 6);
    AddVectorIcon(WidgetTree, Root, TEXT("LegendIconPOI"), TEXT("map-pin.svg"),
        FVector2D(46.0f, 381.0f), FVector2D(22.0f, 22.0f), IconPrimary, 6);

    AddKeyCap(WidgetTree, Root, TEXT("TacticalMapKeyCapM"), TEXT("M"), FVector2D(34.0f, 816.0f));
    ConfigureLabel(WidgetTree, TEXT("TacticalMapHintClose"), TEXT("ЗАКРИТИ"), 72.0f, 130.0f);

    AddVectorIcon(WidgetTree, Root, TEXT("TacticalMapIconZoom"), TEXT("mouse.svg"),
        FVector2D(315.0f, 816.0f), FVector2D(22.0f, 22.0f), IconMuted, 7);
    ConfigureLabel(WidgetTree, TEXT("TacticalMapHintZoom"), TEXT("МАСШТАБ"), 347.0f, 150.0f);

    AddVectorIcon(WidgetTree, Root, TEXT("TacticalMapIconPan"), TEXT("move.svg"),
        FVector2D(650.0f, 816.0f), FVector2D(22.0f, 22.0f), IconMuted, 7);
    ConfigureLabel(WidgetTree, TEXT("TacticalMapHintPan"), TEXT("ЛКМ · ПЕРЕМІЩЕННЯ"), 682.0f, 240.0f);

    AddVectorIcon(WidgetTree, Root, TEXT("TacticalMapIconPing"), TEXT("map-pin.svg"),
        FVector2D(1180.0f, 816.0f), FVector2D(22.0f, 22.0f), IconAmber, 7);
    ConfigureLabel(WidgetTree, TEXT("TacticalMapHintPing"), TEXT("ПКМ · МІТКА"), 1212.0f, 210.0f);

    UE_LOG(LogTemp, Display,
        TEXT("Tactical Map iconography: Lucide vector legend and input prompts installed."));
}
