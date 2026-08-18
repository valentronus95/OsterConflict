#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13GroundSurfaceSubsystem.generated.h"

/** Replaces the debug-flat ground tint with committed terrain material while preserving source collision/bounds. */
UCLASS()
class OSTERCONFLICT_API UOCR13GroundSurfaceSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void ApplyGroundSurface(UWorld& World);
};
