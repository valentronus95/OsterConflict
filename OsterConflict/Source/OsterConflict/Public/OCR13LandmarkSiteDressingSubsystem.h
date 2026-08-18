#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13LandmarkSiteDressingSubsystem.generated.h"

/**
 * R13.5 landmark-site hardscape/furniture pass.
 * Adds conservative visual-only approach and service details around already-authored Museum, Stadium and College.
 * Planting and roadside poles are owned by the dedicated R13.5 civic/roadside subsystems, avoiding duplicate art.
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
