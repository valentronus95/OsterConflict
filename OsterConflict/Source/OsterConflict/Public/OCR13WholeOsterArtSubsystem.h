#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13WholeOsterArtSubsystem.generated.h"

/**
 * R13 whole-Oster environment bridge.
 *
 * Uses the already-committed AdvancedVillagePack meshes to replace the most visible source primitives across the
 * playable city: residential building boxes, primitive tree trunks/crowns and flat grass proxy tiles. Verified
 * road/landmark topology stays untouched while real environment meshes progressively replace source geometry.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13WholeOsterArtSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void ApplyWholeOsterBridge(UWorld& World);
};
