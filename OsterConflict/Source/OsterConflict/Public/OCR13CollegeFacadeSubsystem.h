#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13CollegeFacadeSubsystem.generated.h"

/** Adds visual facade bands, plinth and glazed entrance detail to the authored Oster college massing. */
UCLASS()
class OSTERCONFLICT_API UOCR13CollegeFacadeSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    bool bApplied = false;
    void ApplyCollegeFacade(UWorld& World);
};
