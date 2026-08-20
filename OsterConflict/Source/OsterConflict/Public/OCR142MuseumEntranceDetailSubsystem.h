#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR142MuseumEntranceDetailSubsystem.generated.h"

/** Photo-detail layer for the glazed vestibule, porch railings and dormer/entrance trim. */
UCLASS()
class OSTERCONFLICT_API UOCR142MuseumEntranceDetailSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void BuildEntranceDetail(UWorld& World) const;
};
