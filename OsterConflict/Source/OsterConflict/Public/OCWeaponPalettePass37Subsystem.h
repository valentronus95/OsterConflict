#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "OCWeaponPalettePass37Subsystem.generated.h"

class UMaterialInterface;

/**
 * Pass 37 presentation recovery for restored weapon payloads that still render white/grey.
 *
 * Pass 36 only treated null/Engine DefaultMaterial as broken. Runtime proved that some restored
 * Stein meshes carry non-null but visually blank placeholder assignments. This pass is explicit:
 * it leaves the textured AK alone, gives the known incomplete restored payloads a weapon-specific
 * metal/wood/polymer palette, and only uses placeholder recovery on other rack weapons.
 */
UCLASS()
class OSTERCONFLICT_API UOCWeaponPalettePass37Subsystem final : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

private:
    FTimerHandle AuditTimer;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInterface> PaletteBaseMaterial;

    void AuditRackWeapons();
};
