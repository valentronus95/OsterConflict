#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCAuthoredWorldSurfaceUpgradeSubsystem.generated.h"

/**
 * PASS45 item 31 runtime upgrade for the canonical Oster road family.
 *
 * OCWorldSectorOster still carries legacy Cube transforms as deterministic geo/topology authoring data.
 * Before visual acceptance and before the Pass12 12-second stability baseline, this subsystem replaces the
 * player-facing Roads/Sidewalks meshes with tracked Scene_RoadsideConstruction authored surfaces while
 * preserving each authored transform's geometric bounds. It never converts unrelated/unknown meshes.
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
