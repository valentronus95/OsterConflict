#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13MapPlacementRepairSubsystem.generated.h"

/**
 * Legacy late QA repair pass retained as reference only.
 * Disabled from R13 runtime because it performs whole-world ISM/vegetation mutations at 0.95/2.85 seconds after
 * BeginPlay, producing visible popping and a measurable hitch exactly while the player is entering the match.
 * Placement fixes must be authored at source/spawn time instead of repaired over a live scene.
 */
UCLASS(Abstract)
class OSTERCONFLICT_API UOCR13MapPlacementRepairSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void RepairCentralParkBenchOrientation(UWorld& World);
    void RepairGeneratedVegetationGrounding(UWorld& World);
    void SuppressMisalignedPoleAttachments(UWorld& World);
};
