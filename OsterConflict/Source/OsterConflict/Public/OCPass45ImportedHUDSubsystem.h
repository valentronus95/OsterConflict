#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateBrush.h"
#include "Subsystems/WorldSubsystem.h"
#include "Widgets/SWidget.h"
#include "OCPass45ImportedHUDSubsystem.generated.h"

/**
 * Reuses the already-imported local CrosshairFreePack without replacing the existing HUD system.
 * The legacy line crosshair is suppressed in-memory only while the authored local overlay is active.
 */
UCLASS()
class OSTERCONFLICT_API UOCPass45ImportedHUDSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

private:
    TSharedPtr<SWidget> CrosshairOverlay;
    TSharedPtr<FSlateBrush> CrosshairBrush;
    bool bSuppressedLegacyCrosshair = false;
    bool bOriginalCrosshairSetting = true;
};
