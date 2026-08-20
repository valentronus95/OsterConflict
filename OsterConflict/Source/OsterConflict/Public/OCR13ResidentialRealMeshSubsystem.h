#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCR13ResidentialRealMeshSubsystem.generated.h"

/**
 * Restores the complete AdvancedVillagePack house meshes after the legacy residential
 * presentation pass has consumed them as placement metadata. The real house meshes keep
 * the already-tested collision while the later cube-built presentation is hidden.
 */
UCLASS()
class OSTERCONFLICT_API UOCR13ResidentialRealMeshSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void RestoreRealResidentialMeshes(UWorld& World);
};
