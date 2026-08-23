#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCContentReadinessPass19Subsystem.generated.h"

/**
 * Separates a playable real-mesh weapon rack from the stricter exact-production-art gate.
 * Generic R13 fallback meshes are allowed for focused gameplay/FPS recovery, but never count
 * as exact production assets.
 */
UCLASS()
class OSTERCONFLICT_API UOCContentReadinessPass19Subsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void ValidatePlayableWeaponSet(UWorld& World);
};
