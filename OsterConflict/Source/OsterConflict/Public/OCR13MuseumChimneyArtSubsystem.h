#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13MuseumChimneyArtSubsystem.generated.h"

/** Replaces only the two source-authored museum chimney proxies with bundled chimney art. */
UCLASS()
class OSTERCONFLICT_API UOCR13MuseumChimneyArtSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void BuildMuseumChimneyBridge(UWorld& World);
};
