#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR139MuseumMainDoorReplacementSubsystem.generated.h"

/** Replaces the two generic R13.8 main-door leaves with the photo-proportioned museum double door. */
UCLASS()
class OSTERCONFLICT_API UOCR139MuseumMainDoorReplacementSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void ReplaceMainDoor(UWorld& World) const;
};
