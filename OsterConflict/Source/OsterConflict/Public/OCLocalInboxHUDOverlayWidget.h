#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OCLocalInboxHUDOverlayWidget.generated.h"

class UTexture2D;

/** Lightweight runtime bridge for a user-supplied HUD PNG/TGA/JPG. */
UCLASS()
class OSTERCONFLICT_API UOCLocalInboxHUDOverlayWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetHUDTexture(UTexture2D* InTexture) { HUDTexture = InTexture; InvalidateLayoutAndVolatility(); }

protected:
    virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
        const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
        int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:
    UPROPERTY(Transient) TObjectPtr<UTexture2D> HUDTexture;
};
