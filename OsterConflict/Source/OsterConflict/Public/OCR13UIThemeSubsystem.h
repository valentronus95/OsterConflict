#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13UIThemeSubsystem.generated.h"

class AOCPlayerController;
class UImage;
class UOCGameUIRootWidget;
class UPanelWidget;
class UWidget;

/**
 * Small runtime theme layer for R13 menus.
 *
 * Keeps the approved menu artwork opaque over the world, replaces the coarse
 * banded left overlay with a restrained feather, and applies one graphite / warm
 * sand visual language to the main menu and settings controls.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13UIThemeSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;

private:
    void EnsureThemeLayers(UOCGameUIRootWidget* Root);
    void ApplyTheme(UOCGameUIRootWidget* Root, AOCPlayerController* PC);
    void ThemeWidgetTree(UWidget* Widget);

    TWeakObjectPtr<UOCGameUIRootWidget> ActiveRoot;
    TWeakObjectPtr<UImage> ThemeWorldBlocker;
    TWeakObjectPtr<UImage> ThemeBackdrop;
    TArray<TWeakObjectPtr<UImage>> ThemeFeather;

    float TickAccumulator = 0.0f;
};
