#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCWeaponPalettePass37Subsystem.generated.h"

/**
 * Historical Pass 37/38 weapon-palette subsystem.
 *
 * Pass 44 permanently retires runtime palette/material mutation here. User runtime evidence proved that
 * BasicShapeMaterial/placeholder recolouring can turn otherwise valid meshes into flat grey/orange weapons.
 * Authored material truth now belongs to the dedicated weapon material audit/preflight. This subsystem remains
 * as a compatibility shell so old serialized/runtime class references do not break, but it performs no polling,
 * no material creation and no SetMaterial calls.
 */
UCLASS()
class OSTERCONFLICT_API UOCWeaponPalettePass37Subsystem final : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
};
