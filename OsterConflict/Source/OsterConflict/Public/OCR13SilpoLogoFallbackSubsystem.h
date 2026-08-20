#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13SilpoLogoFallbackSubsystem.generated.h"

/**
 * Guards the main Oster Silpo sign against missing Cyrillic glyphs in the active TextRender font.
 * When the font cannot remap the required letters, a simple extruded geometric СІЛЬПО fallback is shown.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13SilpoLogoFallbackSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void ValidateLogo(UWorld& World);
};
