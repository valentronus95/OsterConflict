#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCWorldGeometryStabilitySubsystem.generated.h"

/**
 * Runtime evidence for VIS-FLICKER-001.
 *
 * The current location coordinator cancels historical delayed builds and the landmark separation
 * guard owns an 8 second cleanup window. This validator starts after that window and proves that
 * the source world-sector geometry families stop changing instead of being rebuilt/trimmed late.
 * It does not claim GPU/shader shimmer is fixed; it separates late geometry mutation from rendering.
 */
UCLASS()
class OSTERCONFLICT_API UOCWorldGeometryStabilitySubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;

private:
    bool ReadTrackedCounts(TMap<FName, int32>& OutCounts, FString& OutFailure) const;
    bool CompareWithBaseline(const TMap<FName, int32>& CurrentCounts, FString& OutFailure) const;
    void FailValidation(const FString& Reason);

    float ElapsedSeconds = 0.0f;
    int32 StableComparisonCount = 0;
    bool bBaselineCaptured = false;
    bool bFinished = false;
    TMap<FName, int32> BaselineCounts;
};
