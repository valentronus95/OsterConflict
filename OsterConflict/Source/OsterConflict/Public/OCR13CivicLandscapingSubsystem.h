#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13CivicLandscapingSubsystem.generated.h"

/**
 * R13.5 civic-location landscaping pass.
 * Adds non-colliding shrubs, flower patches and long-grass accents around the established museum,
 * central park, college and stadium anchors. Building massing and gameplay collision stay untouched so
 * photo-reference refinement can happen later without another navigation rewrite.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13CivicLandscapingSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void ApplyCivicLandscaping(UWorld& World);

    bool bApplied = false;
};
