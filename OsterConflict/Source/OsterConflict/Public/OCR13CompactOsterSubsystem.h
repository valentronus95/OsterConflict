#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13CompactOsterSubsystem.generated.h"

UCLASS()
class OSTERCONFLICT_API UOCR13CompactOsterSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    /** Human deployment is not allowed to commit until the runtime world/spawn relocation pass is complete. */
    bool IsCompactLayoutReady() const { return bApplied; }

private:
    int32 ApplyAttemptCount = 0;
    bool bWorldCropped = false;
    bool bApplied = false;

    void ScheduleApply(UWorld& World, float DelaySeconds);
    void TryApplyCompactLayout(UWorld& World);
};
