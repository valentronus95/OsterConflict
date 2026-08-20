#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13BusStationPhotoModelSubsystem.generated.h"

/**
 * Photo/reference-driven Oster bus-station landmark at the verified public-map coordinate.
 * The footprint is deliberately conservative until a surveyed facade/parcel orientation is available.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13BusStationPhotoModelSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void BuildBusStation(UWorld& World);
};
