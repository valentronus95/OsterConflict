#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13FoliageDiversitySubsystem.generated.h"

/**
 * R13.4 companion foliage pass.
 * Adds explicit conifers, shrubs and wetland reeds around already-authored vegetation zones without
 * replacing the primary collidable tree/navigation contract before the next large visual test.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13FoliageDiversitySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void ApplyFoliageDiversity(UWorld& World);

    bool bApplied = false;
};
