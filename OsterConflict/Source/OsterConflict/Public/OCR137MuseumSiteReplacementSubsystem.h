#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR137MuseumSiteReplacementSubsystem.generated.h"

/**
 * R13.7 museum-site cleanup pass.
 *
 * Runs immediately before the final R13.7 museum photo model. It removes the old source-authored
 * museum proxy (the previous shed-like landmark massing), old museum-only R13 presentation layers,
 * the old museum perimeter fences and primitive vegetation inside the museum site footprint.
 * It is intentionally scoped to MuseumAnchor and does not touch the adjacent stadium rebuild.
 */
UCLASS()
class OSTERCONFLICT_API UOCR137MuseumSiteReplacementSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void PrepareMuseumSite(UWorld& World);
};
