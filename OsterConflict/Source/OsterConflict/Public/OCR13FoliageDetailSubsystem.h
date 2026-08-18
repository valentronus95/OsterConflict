#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13FoliageDetailSubsystem.generated.h"

/** Adds bounded, non-blocking shrub and rough-grass detail around source-authored green areas. */
UCLASS()
class OSTERCONFLICT_API UOCR13FoliageDetailSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void BuildFoliageDetail(UWorld& World);
};
