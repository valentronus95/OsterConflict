#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13StadiumSurfaceSubsystem.generated.h"

/**
 * Authoritative presentation owner for the hard-georeferenced Stadion Oster site.
 * Legacy sector geometry remains only as hidden collision/backstop where required; no second stadium presentation
 * owner may rebuild this site after BeginPlay.
 */
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