#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13CivicLandscapingSubsystem.generated.h"

/**
 * Visual-only planting around civic landmarks. Museum reference architecture, central park planting,
 * landmark furniture and roadside infrastructure keep separate ownership.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13CivicLandscapingSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    bool bApplied = false;
    void ApplyCivicLandscaping(UWorld& World);
};
