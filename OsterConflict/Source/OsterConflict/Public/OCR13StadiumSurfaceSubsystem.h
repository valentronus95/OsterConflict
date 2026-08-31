#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13StadiumSurfaceSubsystem.generated.h"

/**
 * Authoritative presentation owner for the hard-georeferenced Stadion Oster site.
 *
 * PASS45_STADIUM_GAMEPLAY_START_QUARANTINE: the current OnWorldBeginPlay implementation synchronously resolves
 * the runtime-rejected KiteDemo HillTree_02 / ScotsPineTall_01 material-static-mesh dependency chain. Keep the
 * reference-backed implementation in source for a later repaired delayed activation path, but do not let Unreal
 * auto-instantiate it as a UWorldSubsystem during gameplay startup. Runtime status remains rejected until the
 * quarantined stadium presentation has a UE 5.8-safe content path and direct visual acceptance.
 */
UCLASS(Abstract)
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
