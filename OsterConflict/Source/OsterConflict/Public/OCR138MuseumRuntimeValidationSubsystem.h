#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR138MuseumRuntimeValidationSubsystem.generated.h"

/** Authority-side runtime validation for the R13.8 museum architecture and interactions. */
UCLASS()
class OSTERCONFLICT_API UOCR138MuseumRuntimeValidationSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void ValidateMuseum(UWorld& World) const;
};
