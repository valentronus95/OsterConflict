#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13StadiumSurfaceSubsystem.generated.h"

/** Visual turf, track, field markings and small spectator seating over the authored stadium collision geometry. */
UCLASS()
class OSTERCONFLICT_API UOCR13StadiumSurfaceSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    bool bApplied = false;
    void ApplyStadiumSurface(UWorld& World);
};
