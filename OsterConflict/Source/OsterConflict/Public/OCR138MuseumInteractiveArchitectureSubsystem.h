#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR138MuseumInteractiveArchitectureSubsystem.generated.h"

class AActor;
class UWorld;

/**
 * Pass 45 Museum interaction/collision owner.
 *
 * R13.7 is the single visible Museum exterior. R13.8 must never suppress, repaint or replace that
 * visible photo-driven shell. This subsystem owns only invisible collision sections plus replicated
 * interactive doors/windows needed for gameplay.
 */
UCLASS()
class OSTERCONFLICT_API UOCR138MuseumInteractiveArchitectureSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    /** Startup-coordinator entry. Must run after the R13.7 visible owner exists. */
    void RunAuthoritativeUpgradeNow(UWorld& World) { UpgradeMuseum(World); }

private:
    void UpgradeMuseum(UWorld& World);
    AActor* FindR137MuseumActor(UWorld& World) const;
    void ReleaseR137StructuralCollision(AActor& MuseumActor) const;
    void BuildInteractionCollisionArchitecture(UWorld& World) const;
    void SpawnInteractiveOpenings(UWorld& World) const;
};
