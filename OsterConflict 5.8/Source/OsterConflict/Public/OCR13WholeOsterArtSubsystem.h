#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13WholeOsterArtSubsystem.generated.h"

/**
 * R13 whole-Oster recovery/art bridge.
 *
 * R12 intentionally hid the source-generated residential/vegetation proxy families before drawing a single
 * Krushelnytska visual slice. That made the rest of the 2.4 km Oster sector read as an empty desert. R13 keeps the
 * verified city topology visible until each district has a proper art replacement and removes the visibly fantasy
 * fence/lamp pieces from the R12 slice.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13WholeOsterArtSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void ApplyWholeOsterBridge(UWorld& World);
};
