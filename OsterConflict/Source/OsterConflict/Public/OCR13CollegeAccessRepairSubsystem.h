#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13CollegeAccessRepairSubsystem.generated.h"

/**
 * Reference-only repair for the legacy college entrance fence.
 * Runtime execution is disabled while the college site itself is being re-anchored. Repairing a fence on a known-wrong
 * parcel is wasted work and contributes another delayed geometry mutation during the first seconds of gameplay.
 */
UCLASS(Abstract)
class OSTERCONFLICT_API UOCR13CollegeAccessRepairSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void ScheduleRepair(UWorld& World, int32 AttemptIndex);
    bool RepairCollegeEntrance(UWorld& World);
};
