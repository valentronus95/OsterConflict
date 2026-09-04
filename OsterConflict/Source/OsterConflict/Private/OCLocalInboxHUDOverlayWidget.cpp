#include "OCLocalInboxHUDOverlayWidget.h"

#include "Engine/Texture2D.h"
#include "Rendering/DrawElements.h"
#include "Styling/SlateBrush.h"

int32 UOCLocalInboxHUDOverlayWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
    const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
    int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    const int32 BaseLayer = Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements,
        LayerId, InWidgetStyle, bParentEnabled);
    if (!HUDTexture) return BaseLayer;

    FSlateBrush Brush;
    Brush.SetResourceObject(HUDTexture);
    Brush.DrawAs = ESlateBrushDrawType::Image;
    Brush.ImageSize = FVector2D(
        static_cast<float>(FMath::Max(1, HUDTexture->GetSizeX())),
        static_cast<float>(FMath::Max(1, HUDTexture->GetSizeY())));

    FSlateDrawElement::MakeBox(
        OutDrawElements,
        BaseLayer + 1,
        AllottedGeometry.ToPaintGeometry(),
        &Brush,
        ESlateDrawEffect::None,
        InWidgetStyle.GetColorAndOpacityTint());

    return BaseLayer + 1;
}
