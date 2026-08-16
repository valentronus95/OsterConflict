#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCKrushelnytskaVisualSliceSubsystem.generated.h"

/**
 * R12 visual-slice bootstrap for Solomii Krushelnytskoi Street in Oster.
 *
 * The R11 world sector is intentionally source-only and uses UE basic shapes. This subsystem is auto-created
 * for PIE/Game worlds and overlays the Krushelnytska corridor with real project meshes from the installed
 * AdvancedVillagePack. It also hides the old residential proxy families so the user sees an art/content pass,
 * not cubes and spheres pretending to be a town.
 *
 * This is a visual reconstruction based on street/aerial references, not survey-grade geometry.
 */
UCLASS()
class OSTERCONFLICT_API UOCKrushelnytskaVisualSliceSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void BuildVisualSlice(UWorld& World);
};
