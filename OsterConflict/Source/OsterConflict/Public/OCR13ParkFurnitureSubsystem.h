#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13ParkFurnitureSubsystem.generated.h"

/** Replaces only the semantic central-park bench proxies with bundled wood art. */
UCLASS()
class OSTERCONFLICT_API UOCR13ParkFurnitureSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void BuildParkFurnitureBridge(UWorld& World);
};
