#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR138MuseumInteractiveArchitectureSubsystem.generated.h"

class AActor;
class UWorld;

/**
 * Museum-only R13.8 architecture pass.
 *
 * Runs after the R13.7 photo model, removes only the solid prototype shell/openings that prevent
 * an enterable building, and replaces them with a photo-proportioned segmented shell. Structural
 * sections are separate named components so a later Chaos/RPG pass can damage them independently.
 * Interactive doors and glass reuse the existing replicated gameplay actors.
 */
UCLASS()
class OSTERCONFLICT_API UOCR138MuseumInteractiveArchitectureSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    /** Current-main startup coordinator entry. Must run after the R13.7 photo owner exists. */
    void RunAuthoritativeUpgradeNow(UWorld& World) { UpgradeMuseum(World); }

private:
    void UpgradeMuseum(UWorld& World);
    AActor* FindR137MuseumActor(UWorld& World) const;
    void SuppressSolidPrototype(AActor& MuseumActor) const;
    void BuildSegmentedArchitecture(UWorld& World) const;
    void SpawnInteractiveOpenings(UWorld& World) const;
};