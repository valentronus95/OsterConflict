#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13SilpoParkingDetailSubsystem.generated.h"

/**
 * Late visual-only parking pass for the Oster Silpo photo reconstruction.
 * Uses the existing VehicleVarietyPack meshes as static parked cars, preserving gameplay/nav access.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13SilpoParkingDetailSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void ApplyParkingDetails(UWorld& World);
};
