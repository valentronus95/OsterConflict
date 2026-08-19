#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13SilpoFoliageUpgradeSubsystem.generated.h"

/**
 * Uses the already-bundled PN Foliage meshes for the photographed flower/weed strip at Oster Silpo.
 * If the content payload is unavailable at runtime, the procedural site-detail fallback remains visible.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13SilpoFoliageUpgradeSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void UpgradeFoliage(UWorld& World);
};
