#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13MuseumProtectionSubsystem.generated.h"

/** Final cleanup gate that keeps generic environment dressing out of the museum facade/approach view. */
UCLASS()
class OSTERCONFLICT_API UOCR13MuseumProtectionSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void ApplyMuseumProtection(UWorld& World);
    bool bApplied = false;
};
