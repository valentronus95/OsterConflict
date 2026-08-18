#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13LandmarkWindowArtSubsystem.generated.h"

/** Rebuilds semantic landmark-window proxies as framed glass while preserving source-authored positions and sizes. */
UCLASS()
class OSTERCONFLICT_API UOCR13LandmarkWindowArtSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void BuildLandmarkWindowBridge(UWorld& World);
};
