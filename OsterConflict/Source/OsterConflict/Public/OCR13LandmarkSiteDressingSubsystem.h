#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13LandmarkSiteDressingSubsystem.generated.h"

/**
 * R13.4 landmark-site dressing pass.
 * Adds conservative, visual-only site furniture and planting around already-authored Museum, Stadium and College
 * landmarks. It deliberately does not replace landmark building geometry or participate in navigation/collision.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13LandmarkSiteDressingSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void ApplyLandmarkSiteDressing(UWorld& World);

    bool bApplied = false;
};
