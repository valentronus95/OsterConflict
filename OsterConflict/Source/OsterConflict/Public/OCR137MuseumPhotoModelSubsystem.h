#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR137MuseumPhotoModelSubsystem.generated.h"

class UWorld;

/**
 * R13.7 museum-only replacement built from the current eight-angle Oster museum photo set.
 *
 * This pass deliberately runs after the older R13.6 museum/stadium photo layer, suppresses only the
 * legacy museum presentation, and rebuilds the museum as a single coherent runtime model while leaving
 * stadium/gameplay systems untouched. Dimensions are photo-proportioned rather than survey measurements.
 */
UCLASS()
class OSTERCONFLICT_API UOCR137MuseumPhotoModelSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    /** Current-main startup coordinator entry. Reuses the existing authoritative build without a late reveal. */
    void RunAuthoritativeBuildNow(UWorld& World) { ReplaceMuseum(World); }

private:
    void ReplaceMuseum(UWorld& World);
    void SuppressLegacyMuseum(UWorld& World);
    void BuildMuseum(UWorld& World);
};