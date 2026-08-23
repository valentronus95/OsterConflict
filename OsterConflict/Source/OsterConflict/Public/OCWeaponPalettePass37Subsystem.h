#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "OCWeaponPalettePass37Subsystem.generated.h"

class UMaterialInterface;

/**
 * Pass 37/38 presentation recovery for restored weapon payloads.
 *
 * The Pass 37 forced-palette path overwrote every material slot on some restored meshes, which produced
 * the flat orange/black toy-like presentation visible in the latest runtime screenshots. Pass 38 never
 * overwrites a non-placeholder material. It only repairs clearly missing/default/basic placeholder slots,
 * and its startup audit is bounded so it cannot poll the world forever.
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
    int32 AuditPassCount = 0;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInterface> PaletteBaseMaterial;

    void AuditRackWeapons();
};