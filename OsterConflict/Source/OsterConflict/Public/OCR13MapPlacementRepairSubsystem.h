#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13MapPlacementRepairSubsystem.generated.h"

/**
 * Final R13 map-QA repair pass for placement defects exposed by the first playable city review.
 * It corrects deterministic source/art transforms only; it does not generate new landmarks or move gameplay geometry.
 */
UCLASS()
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
