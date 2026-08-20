#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13MuseumEnvironmentDetailSubsystem.generated.h"

/**
 * Photo/reference-driven immediate environment around the Oster local-history museum.
 * Keeps the landmark forecourt readable while replacing generic vegetation with the
 * documented lawn / mature-tree / park-furniture character of Solonyna park.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13MuseumEnvironmentDetailSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void BuildMuseumEnvironment(UWorld& World);
};
