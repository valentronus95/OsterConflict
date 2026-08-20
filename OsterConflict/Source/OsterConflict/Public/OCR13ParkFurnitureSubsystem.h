#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13ParkFurnitureSubsystem.generated.h"

/**
 * Reference-only central-park furniture bridge.
 * Disabled while the park topology/anchor is under correction so benches are not spawned into a location already
 * known to be wrong and do not add another delayed runtime mutation.
 */
UCLASS(Abstract)
class OSTERCONFLICT_API UOCR13ParkFurnitureSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void BuildParkFurnitureBridge(UWorld& World);
};
