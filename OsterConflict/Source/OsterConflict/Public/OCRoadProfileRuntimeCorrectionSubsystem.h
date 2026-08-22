#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCRoadProfileRuntimeCorrectionSubsystem.generated.h"

/**
 * Compatibility correction for the source-only Oster road/sidewalk cube profile.
 *
 * The current S16A topology is useful and must remain unchanged in XY, but the old 14–16 cm road slabs
 * and 18–20 cm sidewalk/hardstand slabs read as raised/convex geometry in runtime. This subsystem
 * normalizes only the shallow Roads/Sidewalks instance Z profile after the world sector appears.
 */
UCLASS()
class OSTERCONFLICT_API UOCRoadProfileRuntimeCorrectionSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;

private:
    bool NormalizeRoadProfile(int32& OutRoadInstances, int32& OutSidewalkInstances);
    bool ValidateRoadProfile(int32& OutRoadInstances, int32& OutSidewalkInstances, FString& OutFailure) const;
    void FailValidation(const FString& Reason);

    float ElapsedSeconds = 0.0f;
    bool bFinished = false;
};
