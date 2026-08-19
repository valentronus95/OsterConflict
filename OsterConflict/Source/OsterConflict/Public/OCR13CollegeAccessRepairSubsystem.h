#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13CollegeAccessRepairSubsystem.generated.h"

/**
 * Repairs the legacy college front-fence instance that crosses the authored entrance stairs.
 * Keeps the original fence material/collision but replaces the one continuous span with two side segments.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13CollegeAccessRepairSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void RepairCollegeEntrance(UWorld& World);
};
