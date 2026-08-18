#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13MetalFenceBridgeSubsystem.generated.h"

/**
 * Transitional R13 bridge for semantic MetalFences proxies.
 * Builds a readable open metal fence from lightweight instanced geometry until production fence art is available.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13MetalFenceBridgeSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void BuildMetalFenceBridge(UWorld& World);
};
