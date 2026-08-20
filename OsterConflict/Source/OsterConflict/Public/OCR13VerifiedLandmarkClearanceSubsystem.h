#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13VerifiedLandmarkClearanceSubsystem.generated.h"

/**
 * Removes generic residential/legacy presentation from small verified landmark footprints.
 * Roads and sidewalks are deliberately untouched. Dedicated landmark models own their final collision/visuals.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13VerifiedLandmarkClearanceSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void ApplyClearance(UWorld& World);
};
