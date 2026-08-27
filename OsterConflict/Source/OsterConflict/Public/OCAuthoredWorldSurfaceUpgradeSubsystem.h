#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCAuthoredWorldSurfaceUpgradeSubsystem.generated.h"

/**
 * PASS45 item 31 runtime upgrade for canonical Oster Cube-authored topology that already has verified tracked assets.
 *
 * OCWorldSectorOster still carries legacy Cube transforms as deterministic geo/topology authoring data.
 * Before visual acceptance and before the Pass12 12-second stability baseline, this subsystem replaces the playable
 * Ground Cube with committed AdvancedVillagePack SM_Plane_1x1 + M_Inst_Landscape while preserving the authored XY
 * footprint and top-Z, replaces the player-facing Roads/Sidewalks meshes with tracked Scene_RoadsideConstruction
 * authored surfaces, separates exactly five central-park path transforms into the semantic ParkPaths family and
 * upgrades them to committed AdvancedVillagePack SM_Stonepath_Var01, and replaces the visible Fences family with the
 * committed AdvancedVillagePack authored fence mesh. Existing transforms, footprint orientation and bounds are
 * preserved. ParkDetails remains reserved for benches/memorial/detail geometry. It never converts unrelated/unknown
 * meshes.
 */
UCLASS()
class OSTERCONFLICT_API UOCAuthoredWorldSurfaceUpgradeSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override { return !bFinished; }

private:
    float ElapsedSeconds = 0.0f;
    bool bFinished = false;
};
