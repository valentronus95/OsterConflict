#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13UtilityPoleSubsystem.generated.h"

/** Adds sparse visual utility poles along source-authored road edges without changing navigation collision. */
UCLASS()
class OSTERCONFLICT_API UOCR13UtilityPoleSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void BuildUtilityPoles(UWorld& World);
};
