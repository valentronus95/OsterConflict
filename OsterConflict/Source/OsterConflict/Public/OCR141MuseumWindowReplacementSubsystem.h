#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR141MuseumWindowReplacementSubsystem.generated.h"

/** Replaces only museum prototype breakable windows with the photo-styled museum variant. */
UCLASS()
class OSTERCONFLICT_API UOCR141MuseumWindowReplacementSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void ReplaceMuseumWindows(UWorld& World) const;
};
