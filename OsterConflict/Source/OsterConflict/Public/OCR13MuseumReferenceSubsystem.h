#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13MuseumReferenceSubsystem.generated.h"

/**
 * R13.4 photo-reference pass for the Oster museum/Solonyna house.
 * Keeps the source-authored landmark topology, then adds the recognizable dark plinth, blue-grey upper volume,
 * entrance treatment, approach path and mature conifer composition visible in the supplied reference photos.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13MuseumReferenceSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void BuildMuseumReferenceLayer(UWorld& World);
};
