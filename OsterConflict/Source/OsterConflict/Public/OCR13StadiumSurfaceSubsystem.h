#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13StadiumSurfaceSubsystem.generated.h"

/**
 * Authoritative presentation owner for the hard-georeferenced Stadion Oster site.
 *
 * The authored implementation remains abstract so it cannot run its historical synchronous startup path directly.
 * GAME_RECOVERY activates it only through a concrete derived subsystem after the complete stadium asset set has
 * finished async preload. This preserves one visual owner while preventing the old game-thread package-load hitch.
 */
UCLASS(Abstract)
class OSTERCONFLICT_API UOCR13StadiumSurfaceSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

protected:
    /** Called only after GAME_RECOVERY has asynchronously resolved the full stadium presentation payload. */
    void ApplyStadiumSurface(UWorld& World);

private:
    bool bApplied = false;
};
